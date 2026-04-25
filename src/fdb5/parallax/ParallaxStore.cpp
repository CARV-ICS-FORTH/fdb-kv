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

namespace fdb5
{
ParallaxStore::ParallaxStore(const Schema &schema, const Key &key, const Config &config)
	: Store(schema)
{
}

eckit::DataHandle *ParallaxStore::retrieve(Field &field) const
{
	return field.dataHandle();
}

std::unique_ptr<FieldLocation> ParallaxStore::archive(const Key &key, const void *data, eckit::Length length)
{
	static std::atomic<uint64_t> archive_counter{ 0 };

	static std::string unique_prefix = []() {
		char hostname[HOST_NAME_MAX];
		gethostname(hostname, HOST_NAME_MAX);
		return std::string(hostname) + "_" + std::to_string(getpid()) + "_";
	}();

	std::string internalKey = key.valuesToString() + "_" + unique_prefix + std::to_string(archive_counter.fetch_add(1));

	size_t hash = std::hash<std::string>{}(internalKey);
	int db_index = hash % PARALLAX_DB_COUNT;

	std::string db_name = "par_db" + std::to_string(db_index);
	par_handle db_handle = par_get_db(db_name);

	const char *error_msg = nullptr;

	struct par_key_value kv;
	kv.k.data = internalKey.c_str();
	kv.k.size = internalKey.size() + 1;

	kv.v.val_buffer = const_cast<char *>(reinterpret_cast<const char *>(data));
	kv.v.val_size = length;

	// par_put(db_handle, &kv, &error_msg);
	write_blob(db_handle,&kv, &error_msg);

	if (error_msg) {
		std::cerr << "Parallax put failed: " << error_msg << std::endl;
		_exit(EXIT_FAILURE);
	}

	eckit::URI uri("parallax", kv.k.data);
	return std::make_unique<ParallaxFieldLocation>(uri, 0, length, Key(nullptr, true));
}

void ParallaxStore::print(std::ostream &out) const
{
	out << "ParallaxStore";
}

static StoreBuilder<ParallaxStore> builder("parallax");

}
