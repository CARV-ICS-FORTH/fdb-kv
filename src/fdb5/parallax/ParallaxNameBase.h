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

#include <memory>
#include <string>

#include "eckit/filesystem/URI.h"
#include "eckit/io/DataHandle.h"
#include "eckit/utils/Optional.h"
#include "fdb5/parallax/ParallaxArray.h"

namespace fdb5
{
class ParallaxNameBase {
    public:
	std::string asString() const;

    protected:
	ParallaxNameBase(const eckit::URI &);

	std::string key_;
	const eckit::URI uri_;
};

class ParallaxArray;

class ParallaxArrayName : public ParallaxNameBase {
    public:
	ParallaxArrayName(const eckit::URI &);

	eckit::DataHandle *dataHandle(const eckit::Offset &, const eckit::Length &) const;
};

}
