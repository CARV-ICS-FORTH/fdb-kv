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
#include "fdb5/parallax/ParallaxIndex.h"
#include "fdb5/parallax/parallax_handle.h"

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

	void deselectIndex() override
	{
		NOTIMP; //< should not be called
	}

	void print(std::ostream &out) const override
	{
		NOTIMP;
	}

	void flush() override
	{
		throw std::logic_error("flush Not implemented");
	}

	void clean() override
	{
		throw std::logic_error("clean Not implemented");
	}

	void close() override
	{
		throw std::logic_error("close Not implemented");
	}

	bool axis(const std::string &keyword, eckit::StringSet &s) const override;
	bool retrieve(const Key &key, Field &field) const override;
	bool open() override;
	bool selectIndex(const Key &key) override;

    private:
	IndexStore indexes_;
	Index current_;
};

}
