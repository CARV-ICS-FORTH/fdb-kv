/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/io/MemoryHandle.h"
#include "eckit/serialisation/HandleStream.h"
#include "eckit/serialisation/MemoryStream.h"
#include "fdb5/database/Index.h"
#include "fdb5/parallax/ParallaxIndexLocation.h"
#include "parallax_handle.h"
#include <limits.h>

namespace fdb5
{
class ParallaxIndex : public fdb5::IndexBase {
    public:
	ParallaxIndex(const fdb5::Key &key);

	ParallaxIndex(const Key &key, bool readAxes);

	void flock() const override
	{
		NOTIMP;
	}

	void funlock() const override
	{
		NOTIMP;
	}

    private:
	const IndexLocation &location() const override
	{
		return location_;
	}

	bool dirty() const override
	{
		NOTIMP;
	}

	void open() override
	{
		NOTIMP;
	}

	void close() override
	{
		NOTIMP;
	}

	void reopen() override
	{
		NOTIMP;
	}

	void visit(IndexLocationVisitor &visitor) const override
	{
		NOTIMP;
	}

	void flush() override
	{
		NOTIMP;
	}

	void encode(eckit::Stream &s, const int version) const override
	{
		NOTIMP;
	}

	void print(std::ostream &out) const override
	{
		NOTIMP;
	}

	void dump(std::ostream &out, const char *indent, bool simple = false, bool dumpFields = false) const override
	{
		NOTIMP;
	}

	IndexStats statistics() const override
	{
		NOTIMP;
	}

	void add(const Key &key, const Field &field) override;
	bool get(const Key &key, const Key &remapKey, Field &field) const override;
	void updateAxes();

	void entries(EntryVisitor &visitor) const override
	{
		throw eckit::Exception("entries Not implemented");
	}

    private:
	ParallaxIndexLocation location_;
};

}
