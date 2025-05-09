/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "ParallaxStore.h"

#include "eckit/config/Resource.h"
#include "fdb5/parallax/ParallaxCatalogue.h"
#include <atomic>

namespace fdb5
{
ParallaxStore::ParallaxStore(const Schema &schema, const Key &key, const Config &config)
	: Store(schema)
{
}

eckit::URI ParallaxStore::uri() const
{
	throw std::logic_error("uri not implemented");
}

bool ParallaxStore::uriBelongs(const eckit::URI &uri) const
{
	throw std::logic_error("uriBelongs not implemented");
}

bool ParallaxStore::uriExists(const eckit::URI &uri) const
{
	throw std::logic_error("uriExists not implemented");
}

std::vector<eckit::URI> ParallaxStore::collocatedDataURIs() const
{
	throw std::logic_error("collocatedDataURIs not implemented");
}

std::set<eckit::URI> ParallaxStore::asCollocatedDataURIs(const std::vector<eckit::URI> &uris) const
{
	throw std::logic_error("asCollocatedDataURIs not implemented");
}

bool ParallaxStore::exists() const
{
	throw std::logic_error("exists not implemented");
}

eckit::DataHandle *ParallaxStore::retrieve(Field &field) const
{
	return field.dataHandle();
}

std::unique_ptr<FieldLocation> ParallaxStore::archive(const Key &key, const void *data, eckit::Length length)
{
	static std::atomic<uint64_t> archive_counter{ 0 };

	std::string internalKey = key.valuesToString();
	internalKey += std::to_string(archive_counter.fetch_add(1));

	size_t hash = std::hash<std::string>{}(internalKey);
	int db_index = hash % PARALLAX_DB_COUNT;

	std::string db_name = "par_db" + std::to_string(db_index + 1);
	par_handle db_handle = par_get_db(db_name);

	const char *error_msg = nullptr;

	struct par_key_value kv {};
	kv.k.data = internalKey.data();
	kv.k.size = internalKey.size() + 1;

	kv.v.val_buffer = const_cast<char *>(reinterpret_cast<const char *>(data));
	kv.v.val_size = length;

	par_put(db_handle, &kv, &error_msg);

	if (error_msg) {
		std::cerr << "Parallax put failed: " << error_msg << std::endl;
		_exit(EXIT_FAILURE);
	}

	eckit::URI uri("parallax", kv.k.data);
	return std::make_unique<ParallaxFieldLocation>(uri, 0, length, Key(nullptr, true));
}

void ParallaxStore::flush()
{
}

void ParallaxStore::remove(const eckit::URI &uri, std::ostream &logAlways, std::ostream &logVerbose, bool doit) const
{
	throw std::logic_error("remove not implemented");
}

void ParallaxStore::print(std::ostream &out) const
{
	throw std::logic_error("print not implemented");
}

static StoreBuilder<ParallaxStore> builder("parallax");

}
