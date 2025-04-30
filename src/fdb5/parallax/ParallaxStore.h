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

#include "fdb5/database/Store.h"
#include "fdb5/parallax/ParallaxArray.h"
#include "fdb5/parallax/ParallaxFieldLocation.h"
#include "fdb5/parallax/parallax_handle.h"
#include "fdb5/rules/Schema.h"

namespace fdb5
{
class ParallaxStore : public Store {
    public:
	ParallaxStore(const Schema &schema, const Key &key, const Config &config);

	~ParallaxStore() override
	{
	}

	eckit::URI uri() const override;
	bool uriBelongs(const eckit::URI &) const override;
	bool uriExists(const eckit::URI &) const override;
	std::vector<eckit::URI> collocatedDataURIs() const override;
	std::set<eckit::URI> asCollocatedDataURIs(const std::vector<eckit::URI> &) const override;

	bool open() override
	{
		return true;
	}
	void flush() override;
	void close() override{};

	void checkUID() const override
	{ /* nothing to do */
	}

    protected:
	std::string type() const override
	{
		return "parallax";
	}

	bool exists() const override;

	eckit::DataHandle *retrieve(Field &field) const override;
	std::unique_ptr<FieldLocation> archive(const Key &key, const void *data, eckit::Length length) override;

	void remove(const eckit::URI &uri, std::ostream &logAlways, std::ostream &logVerbose, bool doit) const override;

	void print(std::ostream &out) const override;
};

}
