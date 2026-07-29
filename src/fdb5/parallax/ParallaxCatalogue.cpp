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

	par_handle db_handle = par_get_db("par_db0");

	std::string fullKey = "id" + std::to_string(prefix) + "_schema";

	struct par_key key;

	key.data = fullKey.c_str();
	key.size = fullKey.size() + 1;

	std::vector<char> schema_buffer(32168U);
	struct par_value schema_value = { .val_buffer_size = 32168U,
					  .val_size = 0,
					  .val_buffer = schema_buffer.data() };

	const char *error_msg = nullptr;
	par_get(db_handle, &key, &schema_value, &error_msg);

	if (error_msg) {
		throw eckit::Exception(std::string("Failed to retrieve schema: ") + error_msg);
	}

	std::string schemaContent(schema_value.val_buffer, schema_value.val_size);
	std::istringstream stream(schemaContent);
	schema_.load(stream);
}

const Schema &ParallaxCatalogue::schema() const
{
	return schema_;
}

}
