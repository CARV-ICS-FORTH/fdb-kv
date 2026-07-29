/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "fdb5/parallax/ParallaxArrayPartHandle.h"

namespace fdb5
{
ParallaxArrayPartHandle::ParallaxArrayPartHandle(const fdb5::ParallaxArrayName &name, const eckit::Offset &off,
						 const eckit::Length &len)
	: name_(name)
	, open_(false)
	, offset_(off)
	, len_(len)
	, local_pos_(0)
{
}

ParallaxArrayPartHandle::~ParallaxArrayPartHandle()
{
	if (open_)
		eckit::Log::error() << "ParallaxArrayPartHandle not closed before destruction." << std::endl;
}

void ParallaxArrayPartHandle::print(std::ostream &s) const
{
	s << "ParallaxArrayPartHandle";
}

Length ParallaxArrayPartHandle::openForRead()
{
	if (open_)
		throw eckit::SeriousBug{ "Handle already opened." };
	arr_.emplace(name_);
	arr_->open();
	open_ = true;
	return size();
}

long ParallaxArrayPartHandle::read(void *buf, long len)
{
	ASSERT(open_);
	long bytes_left = static_cast<long>(static_cast<long long>(len_) - local_pos_);

	if (len > bytes_left) {
		len = bytes_left;
	}

	if (len <= 0) {
		return 0;
	}

	long read_bytes = arr_->read(buf, len, offset_);

	offset_ += read_bytes;
	local_pos_ += read_bytes;

	return read_bytes;
}

void ParallaxArrayPartHandle::close()
{
	if (!open_)
		return;
	arr_->close();
	open_ = false;
}

Length ParallaxArrayPartHandle::size()
{
	return len_;
}

}
