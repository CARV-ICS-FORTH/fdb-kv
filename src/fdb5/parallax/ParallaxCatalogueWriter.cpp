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

inline uint32_t hash_dataset_name(const std::string &name)
{
	uint32_t hash = 2166136261u;
	for (char c : name) {
		hash ^= static_cast<uint8_t>(c);
		hash *= 16777619u;
	}
	return hash;
}

ParallaxCatalogueWriter::ParallaxCatalogueWriter(const Key &key, const fdb5::Config &config)
	: ParallaxCatalogue(key, config)
	, firstIndexWrite_(true)
{
	par_handle db_handle = par_get_db("par_db0");
	std::string dataset_name = this->key().valuesToString();

	prefix = hash_dataset_name(dataset_name);
	std::string prefix_str = std::to_string(prefix);

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
	std::string fullKey = "id" + std::to_string(prefix) + "_schema";
	schema_kv.k.size = fullKey.size() + 1;
	schema_kv.k.data = fullKey.c_str();
	schema_kv.v.val_size = schema_str.size() + 1;
	schema_kv.v.val_buffer = new char[schema_kv.v.val_size];
	std::memcpy(schema_kv.v.val_buffer, schema_str.c_str(), schema_str.size());
	schema_kv.v.val_buffer[schema_str.size()] = '\0';

	const char *schema_err = NULL;
	par_put(db_handle, &schema_kv, &schema_err);
	delete[] schema_kv.v.val_buffer;
	if (schema_err)
		throw eckit::Exception(std::string("Parallax put failed for schema: ") + schema_err);

	par_key_value lookup_kv;
	lookup_kv.k.size = dataset_name.size() + 1;
	lookup_kv.k.data = dataset_name.c_str();
	lookup_kv.v.val_size = prefix_str.size() + 1;
	lookup_kv.v.val_buffer = new char[lookup_kv.v.val_size];
	std::memcpy(lookup_kv.v.val_buffer, prefix_str.c_str(), lookup_kv.v.val_size);

	const char *lookup_err = NULL;
	par_put(db_handle, &lookup_kv, &lookup_err);
	delete[] lookup_kv.v.val_buffer;
	if (lookup_err)
		throw eckit::Exception(std::string("Failed to save dataset lookup record: ") + lookup_err);

	eckit::Log::debug<LibFdb5>()
		<< "Created new dataset " << dataset_name << " (prefix " << prefix << ")" << std::endl;

	ParallaxCatalogue::loadSchema();
}

ParallaxCatalogueWriter::ParallaxCatalogueWriter(const eckit::URI &uri, const fdb5::Config &config)
	: ParallaxCatalogue(uri, ControlIdentifiers{}, config)
	, firstIndexWrite_(true)
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
		std::string keyStr = key.valuesToString();

		struct par_key_value kv;
		std::string fullKey = "id" + std::to_string(prefix) + "_" + keyStr;
		kv.k.size = fullKey.size() + 1;
		kv.k.data = fullKey.c_str();

		uint32_t buffer_size = 32168U;
		std::vector<char> buffer(buffer_size);

		kv.v.val_buffer_size = buffer_size;
		kv.v.val_size = 0;
		kv.v.val_buffer = buffer.data();

		size_t hash = std::hash<std::string>{}(keyStr);
		int db_index = hash % PARALLAX_DB_COUNT;

		std::string db_name = "par_db" + std::to_string(db_index);
		par_handle db_handle = par_get_db(db_name);
		const char *error_msg = nullptr;

		par_get(db_handle, &kv.k, &kv.v, &error_msg);

		error_msg = nullptr;
		if (kv.v.val_size <= 0) {
			std::string fullKey = "id" + std::to_string(prefix) + "_" + keyStr;
			kv.k.data = fullKey.c_str();
			std::string placeholderValue = "parallax_index_placeholder";

			if (placeholderValue.length() > kv.v.val_buffer_size) {
				throw eckit::Exception("Index placeholder exceeded maximum length.");
			}

			kv.v.val_size = placeholderValue.length() + 1;
			strncpy(kv.v.val_buffer, placeholderValue.c_str(), kv.v.val_buffer_size - 1);
			kv.v.val_buffer[kv.v.val_buffer_size - 1] = '\0';

			par_async_put(db_handle, &kv, &error_msg);
			// par_put(db_handle, &kv, &error_msg);

			if (error_msg) {
				throw eckit::Exception(std::string("Failed to insert placeholder index: ") + error_msg);
			}
		}
	}

	indexes_[key] = Index(new ParallaxIndex(key));
	current_ = indexes_[key];

	return true;
}

void ParallaxCatalogueWriter::deselectIndex()
{
	current_ = Index();
	currentIndexKey_ = Key();
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

	std::string axisNames = "";
	std::string sep = "";

	for (Key::const_iterator i = key.begin(); i != key.end(); ++i) {
		const std::string &keyword = i->first;
		std::string value = key.canonicalValue(keyword);
		if (value.empty())
			continue;

		axisNames += sep + keyword;
		sep = ",";

		if (knownAxisValues_[keyword].find(value) == knownAxisValues_[keyword].end()) {
			knownAxisValues_[keyword].insert(value);
			dirtyAxes_.insert(keyword);
		}
	}

	static size_t totalFieldsArchived = 0;
	totalFieldsArchived++;

	current_.put(key, field);

	if (firstIndexWrite_) {
		par_key_value kv;
		const char *error_message = nullptr;

		std::string indexKeyWithAxes = "axes";

		kv.k.data = indexKeyWithAxes.c_str();

		kv.v.val_buffer = (char *)axisNames.data();
		kv.v.val_size = axisNames.length();

		hash = std::hash<std::string>{}(indexKeyWithAxes.c_str());
		db_index = hash % PARALLAX_DB_COUNT;

		db_name = "par_db" + std::to_string(db_index);
		db_handle = par_get_db(db_name);

		std::string fullKey = "id" + std::to_string(prefix) + "_" + kv.k.data;
		kv.k.data = fullKey.c_str();
		kv.k.size = fullKey.size();
		par_async_put(db_handle, &kv, &error_message);
		// par_put(db_handle, &kv, &error_message);
		if (error_message) {
			std::cerr << "Parallax put failed: " << error_message << std::endl;
			_exit(EXIT_FAILURE);
		}

		firstIndexWrite_ = false;
	}
}

void ParallaxCatalogueWriter::flush()
{
	for (auto &pair : indexes_) {
		if (!pair.second.null()) {
			const_cast<fdb5::IndexAxis &>(pair.second.axes()).sort();
		}
	}

	for (const std::string &axisKey : dirtyAxes_) {
		struct par_key_value kv;
		const char *error_message = nullptr;

		std::string fullKey = "id" + std::to_string(prefix) + "_" + axisKey;
		par_key existing_key{ .size = static_cast<uint32_t>(fullKey.size()), .data = fullKey.c_str() };

		std::vector<char> value_buf(32 * 1024);
		par_value existing_value{ .val_buffer_size = static_cast<uint32_t>(value_buf.size()),
					  .val_size = 0,
					  .val_buffer = value_buf.data() };

		size_t hash = std::hash<std::string>{}(axisKey);
		int db_index = hash % PARALLAX_DB_COUNT;

		std::string db_name = "par_db" + std::to_string(db_index);
		par_handle db_handle = par_get_db(db_name);

		par_get(db_handle, &existing_key, &existing_value, &error_message);

		std::set<std::string> mergedValues;

		if (!error_message && existing_value.val_size > 0) {
			size_t safe_size = std::min(static_cast<size_t>(existing_value.val_size), value_buf.size());
			std::string oldValue(value_buf.begin(), value_buf.begin() + safe_size);

			std::vector<std::string> tokens;
			eckit::Tokenizer t(",");
			t(oldValue, tokens);
			for (const auto &val : tokens) {
				mergedValues.insert(val);
			}
		}

		for (const auto &val : knownAxisValues_[axisKey]) {
			mergedValues.insert(val);
		}

		std::string updatedValueStr = "";
		std::string sep = "";
		for (const auto &val : mergedValues) {
			updatedValueStr += sep + val;
			sep = ",";
		}

		kv.k.data = axisKey.c_str();
		kv.v.val_buffer = (char *)updatedValueStr.c_str();
		kv.v.val_size = updatedValueStr.size();

		error_message = nullptr;
		fullKey = "id" + std::to_string(prefix) + "_" + kv.k.data;
		kv.k.data = fullKey.c_str();
		kv.k.size = fullKey.size();
		std::cout << "async key" << kv.k.data << std::endl;
		par_async_put(db_handle, &kv, &error_message);
		// par_put(db_handle, &kv, &error_message);

		if (error_message) {
			std::cerr << "Parallax put failed for axis " << axisKey << ": " << error_message << std::endl;
		}
	}

	dirtyAxes_.clear();
	if (!current_.null())
		current_ = Index();
}

void ParallaxCatalogueWriter::closeIndexes()
{
	indexes_.clear();
}

static fdb5::CatalogueBuilder<fdb5::ParallaxCatalogueWriter> builder("parallax.writer");

}
