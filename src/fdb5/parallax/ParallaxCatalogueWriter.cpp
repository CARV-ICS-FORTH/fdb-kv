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
#include "fdb5/parallax/ParallaxCatalogue.h"
#include "fdb5/parallax/ParallaxCatalogueWriter.h"
#include "fdb5/parallax/ParallaxIndex.h"
#include "parallax_handle.h"
#include <fstream>

namespace fdb5
{
ParallaxCatalogueWriter::ParallaxCatalogueWriter(const Key &key, const fdb5::Config &config)
	: ParallaxCatalogue(key, config)
	, firstIndexWrite_(false)
{
	par_handle db_handle = par_get_db(PARALLAX_GLOBAL_DB);
	const char *error_msg = NULL;

	par_key_value dbKey_kv;
	dbKey_kv.k.size = sizeof("key");
	dbKey_kv.k.data = "key";

	std::ostringstream dbKeyStream;
	dbKeyStream << dbKey_;
	std::string dbKeyStr = dbKeyStream.str();

	if (dbKeyStr.size() + 1 > 512) {
		throw eckit::Exception("Serialised db key exceeded maximum length.");
	}

	dbKey_kv.v.val_size = dbKeyStr.size() + 1;
	dbKey_kv.v.val_buffer = (char*)dbKeyStr.c_str();

	par_put(db_handle, &dbKey_kv, &error_msg);
	if (error_msg) {
		throw eckit::Exception("Failed to insert dbKey: " + std::string(error_msg));
	}

	std::cout << "Inserted Key: " << dbKey_kv.k.data << " | Value: " << dbKey_kv.v.val_buffer << std::endl;

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
	error_msg = NULL;
	par_key_value schema_kv;
	std::string key_str = "schema";

	schema_kv.k.size = key_str.size() + 1;
	schema_kv.k.data = key_str.c_str();

	schema_kv.v.val_size = schema_str.size() + 1;
	schema_kv.v.val_buffer = new char[schema_kv.v.val_size];
	std::memcpy(schema_kv.v.val_buffer, schema_str.c_str(), schema_str.size());
	schema_kv.v.val_buffer[schema_str.size()] = '\0';

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
		par_handle db_handle = par_get_db(PARALLAX_GLOBAL_DB);
		const char *error_msg = nullptr;

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

		par_get(db_handle, &kv.k, &kv.v, &error_msg);

		indexes_[key] = Index(new ParallaxIndex(key));

		if(kv.v.val_size <= 0){
			std::string nstr = indexes_[key].location().uri().asString();
			if (nstr.length() > 512) {
				free(kv.v.val_buffer);
				throw eckit::Exception("Serialized index location exceeded configured maximum index location length.");
			}

	    		kv.v.val_size = nstr.length() + 1;
		        strncpy(kv.v.val_buffer, nstr.c_str(), kv.v.val_buffer_size - 1);
		    	kv.v.val_buffer[kv.v.val_buffer_size - 1] = '\0';
	
		    	par_put(db_handle, &kv, &error_msg);
		}
		free(kv.v.val_buffer);
    	}
   	current_ = indexes_[key];
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
        if (value.empty()) continue;

        axisNames += sep + keyword;
        sep = ",";

        const auto &axis_set = current_.axes().values(keyword);
        if (!axis_set.contains(value)) {
            axesToExpand.push_back(keyword);
            valuesToAdd.push_back(value);
        }
    }

    current_.put(key, field);

    par_handle db_handle = par_get_db("par_db");
    if (!db_handle) {
        std::cerr << "Failed to open Parallax database." << std::endl;
        _exit(EXIT_FAILURE);
    }

    if (firstIndexWrite_) {
        par_key_value kv{};
        const char *error_message = nullptr;

        std::string indexKeyWithAxes = "axes";

        kv.k.data = indexKeyWithAxes.c_str();
        kv.k.size = indexKeyWithAxes.size();

        kv.v.val_buffer = axisNames.data();
        kv.v.val_size = axisNames.length();

        par_put(db_handle, &kv, &error_message);
        if (error_message) {
            std::cerr << "Parallax put failed: " << error_message << std::endl;
            _exit(EXIT_FAILURE);
        }

        firstIndexWrite_ = false;
    }

    if (axesToExpand.empty()) return;

    while (!axesToExpand.empty()) {
        par_key_value kv2{};
        const char *error_message2 = nullptr;

        std::string axisKey = axesToExpand.back();

        kv2.k.data = axisKey.c_str();
        kv2.k.size = axisKey.size();

        kv2.v.val_buffer = (char*)valuesToAdd.back().c_str();
        kv2.v.val_size = valuesToAdd.back().size();

        par_put(db_handle, &kv2, &error_message2);
        if (error_message2) {
            std::cerr << "Parallax put failed: " << error_message2 << std::endl;
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
