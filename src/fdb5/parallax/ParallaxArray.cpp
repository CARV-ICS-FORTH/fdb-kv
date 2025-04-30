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

uint64_t ParallaxArray::read(void *buffer, uint64_t length, const eckit::Offset &)
{
	par_handle db_handle = par_get_db(PARALLAX_GLOBAL_DB);

	struct par_key par_key;
	std::string key = key_;
	par_key.size = key.length() + 1;
	par_key.data = key.c_str();

	struct par_value par_value = { .val_buffer_size = VALUE_BUFFER_SIZE,
				       .val_size = 0,
				       .val_buffer = (char *)malloc(VALUE_BUFFER_SIZE) };

	const char *error_msg = NULL;
	par_get(db_handle, &par_key, &par_value, &error_msg);
	if (error_msg) {
		std::cerr << "Parallax get failed reason: " << error_msg << std::endl;
		free(par_value.val_buffer);
		par_value.val_buffer = NULL;
		par_value.val_size = 0;
	}
	// std::cerr << "Key: " << key << ", par_value size: " << par_value.val_size << std::endl;
	return par_value.val_size;
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
