/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "ParallaxCatalogue.h"

namespace fdb5
{
ParallaxCatalogue::ParallaxCatalogue(const Key &key, const fdb5::Config &config)
	: Catalogue(key, ControlIdentifiers(), config)
{
	par_init_db_handles();
}

ParallaxCatalogue::ParallaxCatalogue(const eckit::URI &uri, const ControlIdentifiers &controlIdentifiers,
				     const fdb5::Config &config)
	: Catalogue(Key(), controlIdentifiers, config)
{
}

ParallaxCatalogue::~ParallaxCatalogue() = default;

std::string ParallaxCatalogue::type() const
{
	return "parallax";
}

bool ParallaxCatalogue::exists() const
{
	return true;
}

void ParallaxCatalogue::loadSchema()
{
	eckit::Timer timer("ParallaxCatalogue::loadSchema()", eckit::Log::debug<fdb5::LibFdb5>());

	struct par_key key;
	std::string key_str = "schema";

	key.size = key_str.size() + 1;
	key.data = key_str.c_str();

	struct par_value value = { .val_buffer_size = 32168U, .val_size = 0, .val_buffer = (char *)malloc(32168U) };

	if (!value.val_buffer) {
		throw eckit::Exception("Memory allocation failed for schema retrieval.");
	}

	size_t hash = std::hash<std::string>{}(key_str.c_str());
	int db_index = hash % PARALLAX_DB_COUNT;

	std::string db_name = "par_db" + std::to_string(db_index + 1);
	par_handle db_handle = par_get_db(db_name);
	const char *error_msg = nullptr;

	par_get(db_handle, &key, &value, &error_msg);
	if (error_msg) {
		free(value.val_buffer);
		throw eckit::Exception(std::string("Failed to retrieve schema: ") + error_msg);
	}

	std::string schemaContent(value.val_buffer, value.val_size);
	free(value.val_buffer);

	std::istringstream stream(schemaContent);
	schema_.load(stream);
}

const Schema &ParallaxCatalogue::schema() const
{
	return schema_;
}

}
