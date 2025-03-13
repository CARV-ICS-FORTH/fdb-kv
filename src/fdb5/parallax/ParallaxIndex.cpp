/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "fdb5/parallax/ParallaxIndex.h"
#include "parallax_handle.h"
#include <limits.h>

#include "eckit/io/MemoryHandle.h"
#include "eckit/serialisation/HandleStream.h"

namespace fdb5
{
ParallaxIndex::ParallaxIndex(const fdb5::Key &key)
	: IndexBase(key, "parallaxKeyValue"),
	location_("parallaxKeyValue")
{
}

ParallaxIndex::ParallaxIndex(const Key &key, bool readAxes)
	: IndexBase(key, "parallaxKeyValue")
	, location_("parallaxKeyValue")
{
}

void ParallaxIndex::updateAxes()
{
	throw std::logic_error("updateAxes Not implemented");
}

bool ParallaxIndex::get(const Key &key, const Key &remapKey, Field &field) const
{
	throw std::logic_error("get Not implemented");
}

void ParallaxIndex::entries(EntryVisitor &visitor) const
{
	throw std::logic_error("entries Not implemented");
}

const std::vector<eckit::URI> ParallaxIndex::dataURIs() const
{
	throw std::logic_error("dataURIs Not implemented");
}

void ParallaxIndex::add(const Key &key, const Field &field)
{
	// throw std::logic_error("add Not implemented");
}

}