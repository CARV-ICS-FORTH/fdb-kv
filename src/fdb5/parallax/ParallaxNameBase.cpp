/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "ParallaxNameBase.h"

#include "fdb5/parallax/ParallaxArrayPartHandle.h"

namespace fdb5
{
//===----------------------------------------------------------------------===//
// ParallaxNameBase
//===----------------------------------------------------------------------===//

ParallaxNameBase::ParallaxNameBase(const eckit::URI &uri)
	: uri_(uri)
{
	ASSERT(uri.scheme() == "parallax");
	ASSERT(uri.query() == std::string());
	ASSERT(uri.fragment() == std::string());

	eckit::Tokenizer parse("/");
	std::vector<std::string> bits;
	parse(uri.name(), bits);

	ASSERT(bits.size() > 0);
	ASSERT(bits.size() < 4);

	key_ = bits[0];
}

std::string ParallaxNameBase::asString() const
{
	return key_;
}

//===----------------------------------------------------------------------===//
// ParallaxArrayName
//===----------------------------------------------------------------------===//

ParallaxArrayName::ParallaxArrayName(const eckit::URI &uri)
	: ParallaxNameBase(uri)
{
}

eckit::DataHandle *ParallaxArrayName::dataHandle(const eckit::Offset &offset, const eckit::Length &length) const
{
	return new fdb5::ParallaxArrayPartHandle(*this, offset, length);
}

}
