/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "fdb5/database/IndexLocation.h"

class ParallaxIndexLocation : public fdb5::IndexLocation {
    public:
	ParallaxIndexLocation(const std::string &location)
		: location_(location)
	{
	}

	std::string asString() const
	{
		return location_;
	}

	eckit::URI uri() const override
	{
		return eckit::URI(location_);
	}

	fdb5::IndexLocation *clone() const override
	{
		return new ParallaxIndexLocation(location_);
	}

	void encode(eckit::Stream &s) const override
	{
		s << location_;
	}

	void print(std::ostream &out) const override
	{
		out << "ParallaxIndexLocation(" << location_ << ")";
	}

    private:
	std::string location_; /*Have to change logic here*/
};
