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

#include "eckit/io/DataHandle.h"
#include "fdb5/parallax/ParallaxNameBase.h"
#include <optional>

namespace fdb5
{
class ParallaxArray;

class ParallaxArrayName;

class ParallaxArrayPartHandle : public eckit::DataHandle {
    public:
	ParallaxArrayPartHandle(const fdb5::ParallaxArrayName &, const eckit::Offset &, const eckit::Length &);

	~ParallaxArrayPartHandle();

	virtual void print(std::ostream &) const override;

	virtual eckit::Length openForRead() override;

	virtual long read(void *, long) override;
	virtual void close() override;

	virtual eckit::Length size() override;

    private:
	mutable fdb5::ParallaxArrayName name_;
	std::optional<fdb5::ParallaxArray> arr_;
	bool open_;
	eckit::Offset offset_;
	eckit::Length len_;
};

}