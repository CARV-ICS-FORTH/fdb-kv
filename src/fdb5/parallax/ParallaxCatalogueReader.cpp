/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "fdb5/LibFdb5.h"

#include "fdb5/parallax/ParallaxCatalogueReader.h"
#include "fdb5/parallax/ParallaxIndex.h"
#include "fdb5/parallax/parallax_handle.h"

namespace fdb5
{
ParallaxCatalogueReader::ParallaxCatalogueReader(const Key &key, const fdb5::Config &config)
	: ParallaxCatalogue(key, config)
{
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
		par_key keyData;
		keyData.size = keyStr.size() + 1;
		keyData.data = keyStr.c_str();

		par_value valueData;
		valueData.val_size = 64;
		valueData.val_buffer_size = valueData.val_size;
		valueData.val_buffer = (char *)malloc(valueData.val_size);
		if (!valueData.val_buffer) {
			throw eckit::Exception("Memory allocation failed for Parallax index retrieval");
		}

		size_t hash = std::hash<std::string>{}(keyStr.c_str());
		int db_index = hash % PARALLAX_DB_COUNT;

		std::string db_name = "par_db" + std::to_string(db_index + 1);
		par_handle db_handle = par_get_db(db_name);

		par_get(db_handle, &keyData, &valueData, &error_msg);

		if (error_msg != nullptr || valueData.val_size <= 0) {
			free(valueData.val_buffer);
			return false;
		}

		free(valueData.val_buffer);

		indexes_[key] = Index(new ParallaxIndex(key, true));
	}

	current_ = indexes_[key];

	return true;
}

void ParallaxCatalogueReader::deselectIndex()
{
	NOTIMP; //< should not be called
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
