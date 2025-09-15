/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "fdb5/parallax/ParallaxCatalogueWriter.h"

namespace fdb5
{
ParallaxCatalogueWriter::ParallaxCatalogueWriter(const Key &key, const fdb5::Config &config)
	: ParallaxCatalogue(key, config)
	, firstIndexWrite_(false)
{
	std::string path = config.schemaPath();

	std::stringstream schema_buffer;
	std::ifstream file(path);
	if (file.is_open()) {
		schema_buffer << file.rdbuf();
		file.close();
	} else {
		throw eckit::Exception("Error opening schema file: " + path);
	}

	std::string schema_str = schema_buffer.str();
	par_key_value schema_kv;
	std::string key_str = "schema";

	schema_kv.k.size = key_str.size() + 1;
	schema_kv.k.data = key_str.c_str();

	schema_kv.v.val_size = schema_str.size() + 1;
	schema_kv.v.val_buffer = new char[schema_kv.v.val_size];
	std::memcpy(schema_kv.v.val_buffer, schema_str.c_str(), schema_str.size());
	schema_kv.v.val_buffer[schema_str.size()] = '\0';

	size_t hash = std::hash<std::string>{}(key_str.c_str());
	int db_index = hash % PARALLAX_DB_COUNT;

	std::string db_name = "par_db" + std::to_string(db_index + 1);
	par_handle db_handle = par_get_db(db_name);
	const char *error_msg = NULL;

	par_put(db_handle, &schema_kv, &error_msg);
	if (error_msg) {
		std::cout << "Sorry Parallax put failed reason: " << error_msg << std::endl;
		delete[] schema_kv.v.val_buffer;
		_exit(EXIT_FAILURE);
	}

	delete[] schema_kv.v.val_buffer;

	eckit::Log::debug<LibFdb5>() << "Copy schema from " << config_.schemaPath() << " at key 'schema'." << std::endl;

	ParallaxCatalogue::loadSchema();
}

ParallaxCatalogueWriter::ParallaxCatalogueWriter(const eckit::URI &uri, const fdb5::Config &config)
	: ParallaxCatalogue(uri, ControlIdentifiers{}, config)
	, firstIndexWrite_(false)
{
}

ParallaxCatalogueWriter::~ParallaxCatalogueWriter()
{
	clean();
	close();
}

bool ParallaxCatalogueWriter::selectIndex(const Key &key)
{
	currentIndexKey_ = key;

	if (indexes_.find(key) == indexes_.end()) {
		par_key_value kv;
		std::string keyStr = key.valuesToString();
		kv.k.size = keyStr.size() + 1;
		kv.k.data = keyStr.c_str();

		kv.v.val_buffer_size = 32168U;
		kv.v.val_size = 0;
		kv.v.val_buffer = (char *)malloc(kv.v.val_buffer_size);

		if (!kv.v.val_buffer) {
			throw eckit::Exception("Memory allocation failed for index retrieval.");
		}

		size_t hash = std::hash<std::string>{}(keyStr.c_str());
		int db_index = hash % PARALLAX_DB_COUNT;

		std::string db_name = "par_db" + std::to_string(db_index + 1);
		par_handle db_handle = par_get_db(db_name);
		const char *error_msg = nullptr;

		par_get(db_handle, &kv.k, &kv.v, &error_msg);

		error_msg = nullptr;
		if (kv.v.val_size <= 0) {
			std::string placeholderValue = "parallax_index_placeholder";

			if (placeholderValue.length() > kv.v.val_buffer_size) {
				free(kv.v.val_buffer);
				throw eckit::Exception("Index placeholder exceeded maximum length.");
			}

			kv.v.val_size = placeholderValue.length() + 1;
			strncpy(kv.v.val_buffer, placeholderValue.c_str(), kv.v.val_buffer_size - 1);
			kv.v.val_buffer[kv.v.val_buffer_size - 1] = '\0';

			size_t hash = std::hash<std::string>{}(keyStr.c_str());
			int db_index = hash % PARALLAX_DB_COUNT;

			std::string db_name = "par_db" + std::to_string(db_index + 1);
			db_handle = par_get_db(db_name);

			par_put(db_handle, &kv, &error_msg);

			if (error_msg) {
				free(kv.v.val_buffer);
				throw eckit::Exception(std::string("Failed to insert placeholder index: ") + error_msg);
			}
		}
	}

	indexes_[key] = Index(new ParallaxIndex(key));
	current_ = indexes_[key];
	firstIndexWrite_ = true;
	return true;
}

void ParallaxCatalogueWriter::deselectIndex()
{
	current_ = Index();
	currentIndexKey_ = Key();
	firstIndexWrite_ = false;
}

void ParallaxCatalogueWriter::clean()
{
	flush();
	deselectIndex();
}

void ParallaxCatalogueWriter::close()
{
	closeIndexes();
}

const Index &ParallaxCatalogueWriter::currentIndex()
{
	if (current_.null()) {
		ASSERT(!currentIndexKey_.empty());
		selectIndex(currentIndexKey_);
	}
	return current_;
}

void ParallaxCatalogueWriter::archive(const Key &key, std::unique_ptr<FieldLocation> fieldLocation)
{
	par_handle db_handle;
	size_t hash;
	int db_index;
	std::string db_name;

	if (current_.null()) {
		ASSERT(!currentIndexKey_.empty());
		selectIndex(currentIndexKey_);
	}

	Field field(std::move(fieldLocation), currentIndex().timestamp());

	const_cast<fdb5::IndexAxis &>(current_.axes()).sort();

	std::vector<std::string> axesToExpand;
	std::vector<std::string> valuesToAdd;
	std::string axisNames = "";
	std::string sep = "";

	for (Key::const_iterator i = key.begin(); i != key.end(); ++i) {
		const std::string &keyword = i->first;
		std::string value = key.canonicalValue(keyword);
		if (value.empty())
			continue;

		axisNames += sep + keyword;
		sep = ",";

		const auto &axis_set = current_.axes().values(keyword);
		if (!axis_set.contains(value)) {
			axesToExpand.push_back(keyword);
			valuesToAdd.push_back(value);
		}
	}

	current_.put(key, field);

	if (firstIndexWrite_) {
		par_key_value kv;
		const char *error_message = nullptr;

		std::string indexKeyWithAxes = "axes";

		kv.k.data = indexKeyWithAxes.c_str();
		kv.k.size = indexKeyWithAxes.size();

		kv.v.val_buffer = axisNames.data();
		kv.v.val_size = axisNames.length();

		hash = std::hash<std::string>{}(indexKeyWithAxes.c_str());
		db_index = hash % PARALLAX_DB_COUNT;

		db_name = "par_db" + std::to_string(db_index + 1);
		db_handle = par_get_db(db_name);

		par_put(db_handle, &kv, &error_message);
		if (error_message) {
			std::cerr << "Parallax put failed: " << error_message << std::endl;
			_exit(EXIT_FAILURE);
		}

		firstIndexWrite_ = false;
	}

	if (axesToExpand.empty())
		return;

	while (!axesToExpand.empty()) {
		const std::string &axisKey = axesToExpand.back();
		const std::string &newValue = valuesToAdd.back();

		par_key_value kv2;
		const char *error_message2 = nullptr;

		par_key existing_key{ .size = static_cast<uint32_t>(axisKey.size()), .data = axisKey.c_str() };

		std::vector<char> value_buf(1024);
		par_value existing_value{ .val_buffer_size = static_cast<uint32_t>(value_buf.size()),
					  .val_size = 0,
					  .val_buffer = value_buf.data() };

		hash = std::hash<std::string>{}(axisKey.c_str());
		db_index = hash % PARALLAX_DB_COUNT;

		db_name = "par_db" + std::to_string(db_index + 1);
		db_handle = par_get_db(db_name);

		par_get(db_handle, &existing_key, &existing_value, &error_message2);

		std::string updatedValueStr;
		if (!error_message2 && existing_value.val_size > 0) {
			std::string oldValue(value_buf.begin(), value_buf.begin() + existing_value.val_size);

			std::vector<std::string> tokens;
			eckit::Tokenizer t(",");
			t(oldValue, tokens);

			if (std::find(tokens.begin(), tokens.end(), newValue) == tokens.end()) {
				oldValue += "," + newValue;
			}
			updatedValueStr = oldValue;
		} else {
			updatedValueStr = newValue;
		}

		kv2.k.data = axisKey.c_str();
		kv2.k.size = axisKey.size();

		kv2.v.val_buffer = (char *)updatedValueStr.c_str();
		kv2.v.val_size = updatedValueStr.size();

		error_message2 = nullptr;

		size_t hash = std::hash<std::string>{}(axisKey.c_str());
		int db_index = hash % PARALLAX_DB_COUNT;

		std::string db_name = "par_db" + std::to_string(db_index + 1);
		db_handle = par_get_db(db_name);
		par_put(db_handle, &kv2, &error_message2);
		if (error_message2) {
			_exit(EXIT_FAILURE);
		}

		axesToExpand.pop_back();
		valuesToAdd.pop_back();
	}
}

void ParallaxCatalogueWriter::flush()
{
	if (!current_.null())
		current_ = Index();
}

void ParallaxCatalogueWriter::closeIndexes()
{
	indexes_.clear();
}

static fdb5::CatalogueBuilder<fdb5::ParallaxCatalogueWriter> builder("parallax.writer");

}
