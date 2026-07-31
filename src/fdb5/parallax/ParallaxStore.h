/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "fdb5/parallax/ParallaxCatalogue.h"
#include "fdb5/parallax/ParallaxFieldLocation.h"
#include <atomic>
#include <limits.h>

namespace fdb5
{
class ParallaxStore : public Store {
    public:
	ParallaxStore(const Schema &schema, const Key &key, const Config &config);

	eckit::URI uri() const override
	{
		throw std::logic_error("uri not implemented");
	}

	bool uriBelongs(const eckit::URI &) const override
	{
		throw std::logic_error("uriBelongs not implemented");
	}

	bool uriExists(const eckit::URI &) const override
	{
		throw std::logic_error("uriExists not implemented");
	}

	std::vector<eckit::URI> collocatedDataURIs() const override
	{
		throw std::logic_error("collocatedDataURIs not implemented");
	}

	std::set<eckit::URI> asCollocatedDataURIs(const std::vector<eckit::URI> &) const override
	{
		throw std::logic_error("asCollocatedDataURIs not implemented");
	}

	bool open() override
	{
		return true;
	}

	void close() override
	{
	}

	void checkUID() const override
	{
	}

    protected:
	void remove(const eckit::URI &uri, std::ostream &logAlways, std::ostream &logVerbose, bool doit) const override
	{
		throw std::logic_error("remove not implemented");
	}

	bool exists() const override
	{
		throw std::logic_error("exists not implemented");
	}

	std::string type() const override
	{
		return "parallax";
	}

	eckit::DataHandle *retrieve(Field &field) const override;
	std::unique_ptr<FieldLocation> archive(const Key &key, const void *data, eckit::Length length) override;
	void print(std::ostream &out) const override;
	void flush();
};

}
