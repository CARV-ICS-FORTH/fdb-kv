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

#include "fdb5/database/DB.h"
#include <parallax.h>

namespace fdb5
{
class ParallaxCatalogue : public Catalogue {
    public:
	ParallaxCatalogue(const Key &key, const fdb5::Config &config)
		: Catalogue(key, ControlIdentifiers(), config)
	{
	}

	ParallaxCatalogue(const eckit::URI &uri, const ControlIdentifiers &controlIdentifiers,
			  const fdb5::Config &config)
		: Catalogue(Key(), controlIdentifiers, config)
	{
		throw std::logic_error("constructor not implemented");
	}

	~ParallaxCatalogue() override;

	std::string type() const override;
	bool exists() const override;
	void loadSchema() override;
	std::vector<Index> indexes(bool sorted = false) const override;
	const Schema &schema() const override;
	eckit::URI uri() const override;

	void checkUID() const override
	{
		throw std::logic_error("checkUID Not implemented");
	};

	void dump(std::ostream &out, bool simple, const eckit::Configuration &conf) const override
	{
		throw std::logic_error("dump Not implemented");
	};

	std::vector<eckit::PathName> metadataPaths() const override
	{
		throw std::logic_error("metadataPaths Not implemented");
	};

	StatsReportVisitor *statsReportVisitor() const override
	{
		throw std::logic_error("statsReportVisitor Not implemented");
	};

	PurgeVisitor *purgeVisitor(const Store &store) const override
	{
		throw std::logic_error("purge Visitor Not implemented");
	};

	WipeVisitor *wipeVisitor(const Store &store, const metkit::mars::MarsRequest &request, std::ostream &out,
				 bool doit, bool porcelain, bool unsafeWipeAll) const override;
	MoveVisitor *moveVisitor(const Store &store, const metkit::mars::MarsRequest &request, const eckit::URI &dest,
				 eckit::Queue<MoveElement> &queue) const override
	{
		throw std::logic_error("move Visitor Not implemented");
	};

	void maskIndexEntry(const Index &index) const override
	{
		throw std::logic_error("mask Index Entry Not implemented");
	};

	void allMasked(std::set<std::pair<eckit::URI, eckit::Offset> > &metadata,
		       std::set<eckit::URI> &data) const override
	{
		throw std::logic_error("all Masked Not implemented");
	};

	void control(const ControlAction &action, const ControlIdentifiers &identifiers) const override
	{
		throw std::logic_error("control Not implemented");
	};

    protected:
	Key currentIndexKey_;

    private:
	Schema schema_;
};

}