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
#include "fdb5/LibFdb5.h"
#include "parallax_handle.h"

namespace fdb5
{
ParallaxCatalogue::~ParallaxCatalogue() = default;

std::string ParallaxCatalogue::type() const
{
	throw std::logic_error("type Not implemented");
}

bool ParallaxCatalogue::exists() const
{
	return true; /*Needs extra work*/
}

WipeVisitor *ParallaxCatalogue::wipeVisitor(const Store &store, const metkit::mars::MarsRequest &request,
					    std::ostream &out, bool doit, bool porcelain, bool unsafeWipeAll) const
{
	throw std::logic_error("wipeVisitor Not implemented");
}

void ParallaxCatalogue::loadSchema()
{
	eckit::Timer timer("ParallaxCatalogue::loadSchema()", eckit::Log::debug<fdb5::LibFdb5>());

	par_handle db_handle = par_get_db(PARALLAX_GLOBAL_DB);
	const char *error_msg = nullptr;

	struct par_key key;
	std::string key_str = "schema";

	key.size = key_str.size() + 1;
	key.data = key_str.c_str();

	struct par_value value = { .val_buffer_size = 32168U, .val_size = 0, .val_buffer = (char *)malloc(32168U) };

	if (!value.val_buffer) {
		throw eckit::Exception("Memory allocation failed for schema retrieval.");
	}

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

std::vector<Index> ParallaxCatalogue::indexes(bool sorted) const
{
	throw std::logic_error("indexes Not implemented");
}

const Schema &ParallaxCatalogue::schema() const
{
	return schema_;
}

eckit::URI ParallaxCatalogue::uri() const
{
	throw std::logic_error("uri Not implemented");
}

}
