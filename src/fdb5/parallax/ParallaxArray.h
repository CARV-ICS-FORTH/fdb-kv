/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#pragma once

#include "fdb5/parallax/ParallaxNameBase.h"
#include "fdb5/parallax/parallax_handle.h"

#define VALUE_BUFFER_SIZE (4 * 1024 * 1024)

namespace fdb5
{
class ParallaxNameBase;

class ParallaxArray {
    public:
	ParallaxArray(const ParallaxNameBase &name);

	uint64_t read(void *buffer, uint64_t length, const eckit::Offset &);
	void open();
	void close();

    private:
	std::string key_;
	bool isOpen_;
};

}
