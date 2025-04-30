/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "fdb5/parallax/ParallaxFieldLocation.h"
#include "eckit/filesystem/URIManager.h"
#include "fdb5/LibFdb5.h"
#include "fdb5/parallax/ParallaxArray.h"

namespace fdb5
{
::eckit::ClassSpec ParallaxFieldLocation::classSpec_ = {
	&FieldLocation::classSpec(),
	"ParallaxFieldLocation",
};
::eckit::Reanimator<ParallaxFieldLocation> ParallaxFieldLocation::reanimator_;

ParallaxFieldLocation::ParallaxFieldLocation(const ParallaxFieldLocation &rhs)
	: FieldLocation(rhs.uri_, rhs.offset_, rhs.length_, rhs.remapKey_)
{
}

ParallaxFieldLocation::ParallaxFieldLocation(const eckit::URI &uri, eckit::Offset offset, eckit::Length length,
					     const Key &remapKey)
	: FieldLocation(uri, offset, length, remapKey)
{
}

ParallaxFieldLocation::ParallaxFieldLocation(eckit::Stream &s)
	: FieldLocation(s)
{
}

std::shared_ptr<FieldLocation> ParallaxFieldLocation::make_shared() const
{
	return std::make_shared<ParallaxFieldLocation>(std::move(*this));
}

eckit::DataHandle *ParallaxFieldLocation::dataHandle() const
{
	return fdb5::ParallaxArrayName(uri_).dataHandle(offset(), length());
}

void ParallaxFieldLocation::print(std::ostream &out) const
{
	out << "ParallaxFieldLocation[uri=" << uri_ << "]";
}

void ParallaxFieldLocation::visit(FieldLocationVisitor &visitor) const
{
	visitor(*this);
}

}
