/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "fdb5/parallax/ParallaxArray.h"

namespace fdb5
{
ParallaxArray::ParallaxArray(const ParallaxNameBase &name)
	: key_(name.asString())
	, isOpen_(false)
{
}

uint64_t ParallaxArray::read(void *buffer, uint64_t length, const eckit::Offset &offset)
{
	struct par_key par_key;
	std::string key = key_;
	par_key.size = key.length() + 1;
	par_key.data = key.c_str();

	std::vector<char> temp_buffer(VALUE_BUFFER_SIZE);

	struct par_value par_value = { .val_buffer_size = VALUE_BUFFER_SIZE,
				       .val_size = 0,
				       .val_buffer = temp_buffer.data() };

	size_t hash = std::hash<std::string>{}(key.c_str());
	int db_index = hash % PARALLAX_DB_COUNT;

	std::string db_name = "par_db" + std::to_string(db_index);

	par_handle db_handle = par_get_db(db_name);
	const char *error_msg = NULL;

	// par_get(db_handle, &par_key, &par_value, &error_msg);
	read_blob(db_handle, &par_key, &par_value, &error_msg);
	if (error_msg) {
		std::cerr << "Parallax get failed reason: " << error_msg << std::endl;
		return 0;
	}

	uint64_t start_offset = static_cast<uint64_t>(offset);
	uint64_t object_size = par_value.val_size;

	if (start_offset >= object_size) {
		return 0;
	}

	uint64_t bytes_to_copy = length;
	if (start_offset + bytes_to_copy > object_size) {
		bytes_to_copy = object_size - start_offset;
	}

	memcpy(buffer, temp_buffer.data() + start_offset, bytes_to_copy);

	return bytes_to_copy;
}

void ParallaxArray::open()
{
	if (!isOpen_) {
		isOpen_ = true;
	}
}

void ParallaxArray::close()
{
	if (isOpen_) {
		isOpen_ = false;
	}
}

}
