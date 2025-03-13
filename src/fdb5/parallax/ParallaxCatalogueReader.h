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

#include "fdb5/parallax/ParallaxCatalogue.h"

namespace fdb5
{

class ParallaxCatalogueReader : public ParallaxCatalogue, public CatalogueReader {
    public:
	ParallaxCatalogueReader(const Key &key, const fdb5::Config &config);
	ParallaxCatalogueReader(const eckit::URI &uri, const fdb5::Config &config);

	DbStats stats() const override
	{
		NOTIMP;
	}

	bool selectIndex(const Key &key) override;
	void deselectIndex() override;

	bool open() override;
	void flush() override
	{
	}
	void clean() override
	{
	}
	void close() override
	{
	}

	bool axis(const std::string &keyword, eckit::StringSet &s) const override;

	bool retrieve(const Key &key, Field &field) const override;

	void print(std::ostream &out) const override
	{
		NOTIMP;
	}

    private:
	typedef std::map<Key, Index> IndexStore;

    private:
	IndexStore indexes_;
	Index current_;
};

}
