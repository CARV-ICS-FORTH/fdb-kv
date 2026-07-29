/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "fdb5/parallax/ParallaxCatalogueReader.h"

namespace fdb5
{
ParallaxCatalogueReader::ParallaxCatalogueReader(const Key &key, const fdb5::Config &config)
	: ParallaxCatalogue(key, config)
{
	par_handle db_handle = par_get_db("par_db0");

	std::string dataset_name = this->key().valuesToString();

	struct par_key lookup_key = { .size = (uint32_t)(dataset_name.size() + 1), .data = dataset_name.c_str() };
	struct par_value lookup_val = {};
	lookup_val.val_buffer_size = 64;
	lookup_val.val_buffer = new char[lookup_val.val_buffer_size];

	const char *error_msg = NULL;
	par_get(db_handle, &lookup_key, &lookup_val, &error_msg);

	if (error_msg == NULL && lookup_val.val_size > 0) {
		prefix = std::stoi(lookup_val.val_buffer);
	} else {
		delete[] lookup_val.val_buffer;
		throw eckit::UserError("Dataset not found in Parallax: " + dataset_name);
	}

	delete[] lookup_val.val_buffer;
}

ParallaxCatalogueReader::ParallaxCatalogueReader(const eckit::URI &uri, const fdb5::Config &config)
	: ParallaxCatalogue(uri, ControlIdentifiers{}, config)
{
}

bool ParallaxCatalogueReader::selectIndex(const Key &key)
{
	if (currentIndexKey_ == key) {
		return true;
	}

	currentIndexKey_ = key;

	if (indexes_.find(key) == indexes_.end()) {
		const char *error_msg = nullptr;

		std::string keyStr = key.valuesToString();
		struct par_key keyData;
		std::string fullKey = "id" + std::to_string(prefix) + "_" + keyStr;
		keyData.size = fullKey.size() + 1;
		keyData.data = fullKey.c_str();

		std::vector<char> buffer(32168U);
		struct par_value valueData = { .val_buffer_size = static_cast<uint32_t>(buffer.size()),
					       .val_size = 0,
					       .val_buffer = buffer.data() };

		size_t hash = std::hash<std::string>{}(keyStr);
		int db_index = hash % PARALLAX_DB_COUNT;

		std::string db_name = "par_db" + std::to_string(db_index);
		par_handle db_handle = par_get_db(db_name);

		par_get(db_handle, &keyData, &valueData, &error_msg);

		if (error_msg != nullptr || valueData.val_size <= 0) {
			return false;
		}

		indexes_[key] = Index(new ParallaxIndex(key, true));
	}

	current_ = indexes_[key];

	return true;
}

bool ParallaxCatalogueReader::open()
{
	if (!ParallaxCatalogue::exists()) {
		return false;
	}

	ParallaxCatalogue::loadSchema();
	return true;
}

bool ParallaxCatalogueReader::axis(const std::string &keyword, eckit::StringSet &s) const
{
	bool found = false;
	if (current_.axes().has(keyword)) {
		found = true;
		const eckit::DenseSet<std::string> &a = current_.axes().values(keyword);
		s.insert(a.begin(), a.end());
	}
	return found;
}

bool ParallaxCatalogueReader::retrieve(const Key &key, Field &field) const
{
	eckit::Log::debug<LibFdb5>() << "Trying to retrieve key " << key << std::endl;
	eckit::Log::debug<LibFdb5>() << "Scanning index " << current_.location() << std::endl;

	if (!current_.mayContain(key))
		return false;

	return current_.get(key, fdb5::Key(), field);
}

static fdb5::CatalogueBuilder<fdb5::ParallaxCatalogueReader> builder("parallax.reader");

}
