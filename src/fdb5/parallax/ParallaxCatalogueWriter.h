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
class ParallaxCatalogueWriter : public ParallaxCatalogue, public CatalogueWriter {
    public:
	ParallaxCatalogueWriter(const Key &key, const fdb5::Config &config);
	ParallaxCatalogueWriter(const eckit::URI &uri, const fdb5::Config &config);

	virtual ~ParallaxCatalogueWriter() override;

	void index(const Key &key, const eckit::URI &uri, eckit::Offset offset, eckit::Length length) override
	{
		NOTIMP;
	};

	void reconsolidate() override
	{
		NOTIMP;
	}

	void overlayDB(const Catalogue &otherCatalogue, const std::set<std::string> &variableKeys,
		       bool unmount) override
	{
		NOTIMP;
	};

	const Index &currentIndex() override;

    protected:
	virtual bool selectIndex(const Key &key) override;
	virtual void deselectIndex() override;

	bool open() override
	{
		NOTIMP;
	}
	void flush() override;
	void clean() override;
	void close() override;

	void archive(const Key &key, std::unique_ptr<FieldLocation> fieldLocation) override;

	virtual void print(std::ostream &out) const override
	{
		NOTIMP;
	}

    private:
	void closeIndexes();

    private:
	typedef std::map<Key, Index> IndexStore;

    private:
	IndexStore indexes_;

	Index current_;

	bool firstIndexWrite_;
};

}