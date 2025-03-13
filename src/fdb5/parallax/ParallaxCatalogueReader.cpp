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
		par_handle db_handle = par_get_db(PARALLAX_GLOBAL_DB);
		const char *error_msg = nullptr;

		std::string keyStr = key.valuesToString();
		std::cout << "Looking for key in Parallax: " << keyStr << std::endl;
		par_key keyData;
		keyData.size = keyStr.size() + 1;
		keyData.data = keyStr.c_str();

		par_value valueData;
		valueData.val_size = 512;
		valueData.val_buffer_size = valueData.val_size;
		valueData.val_buffer = (char *)malloc(valueData.val_size);
		if (!valueData.val_buffer) {
			throw eckit::Exception("Memory allocation failed for Parallax index retrieval");
		}

		par_get(db_handle, &keyData, &valueData, &error_msg);

		if (error_msg != nullptr) {
			std::cout << "Key NOT found in Parallax: " << keyStr << std::endl;
			free(valueData.val_buffer);
			return false;
		}

		std::cout << "Key FOUND in Parallax: " << keyStr << std::endl;
		std::string indexLocation(valueData.val_buffer, valueData.val_size);
		free(valueData.val_buffer);

		indexes_[key] = Index(new ParallaxIndex(key, true));
	}

	current_ = indexes_[key];
	
	return true;
}

void ParallaxCatalogueReader::deselectIndex()
{
	NOTIMP;
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
	// throw std::logic_error("retrieve Not implemented");

	eckit::Log::debug<LibFdb5>() << "Trying to retrieve key " << key << std::endl;
	eckit::Log::debug<LibFdb5>() << "Scanning index " << current_.location() << std::endl;

	if (!current_.mayContain(key))
		return false;

	return current_.get(key, fdb5::Key(), field);
}

static fdb5::CatalogueBuilder<fdb5::ParallaxCatalogueReader> builder("parallax.reader");

}