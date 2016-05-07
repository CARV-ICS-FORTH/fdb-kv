/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/option/CmdArgs.h"
#include "fdb5/toc/ReportVisitor.h"
#include "fdb5/toc/TocHandler.h"
#include "fdb5/tools/FDBInspect.h"

//----------------------------------------------------------------------------------------------------------------------

class PurgeVisitor : public fdb5::ReportVisitor {
public:
    PurgeVisitor(const eckit::PathName& directory): ReportVisitor(directory) {}
    void purge() const;
};



void PurgeVisitor::purge() const {


    for (std::map<const fdb5::Index *, fdb5::Statistics>::const_iterator i = indexStats_.begin();
            i != indexStats_.end(); ++i) {

        const fdb5::Statistics &stats = i->second;

        if (stats.fields_ == stats.duplicates_) {
            eckit::Log::info() << "Removing: " << *(i->first) << std::endl;
            fdb5::TocHandler handler(directory_);
            handler.writeClearRecord(*(*i).first);
        }
    }


    for (std::map<eckit::PathName, size_t>::const_iterator i = dataUsage_.begin(); i != dataUsage_.end(); ++i) {
        if (i->second == 0) {
            if (i->first.dirName().sameAs(directory_)) {
                i->first.unlink();
            }
        }
    }

    for (std::map<eckit::PathName, size_t>::const_iterator i = indexUsage_.begin(); i != indexUsage_.end(); ++i) {
        if (i->second == 0) {
            if (i->first.dirName().sameAs(directory_)) {
                i->first.unlink();
            }
        }
    }

}

class FDBPurge : public fdb5::FDBInspect {

  public: // methods

    FDBPurge(int argc, char **argv) :
        fdb5::FDBInspect(argc, argv),
        doit_(false),
        wipe_(false),
        count_(0) {

        options_.push_back(new eckit::option::SimpleOption<bool>("doit", "Delete the files (data and indexes)"));

    }

  private: // methods

    virtual void process(const eckit::PathName &, const eckit::option::CmdArgs &args);
    virtual void usage(const std::string &tool) const;
    virtual void init(const eckit::option::CmdArgs &args);
    virtual void finish(const eckit::option::CmdArgs &args);

    bool doit_;
    bool wipe_;
    fdb5::Statistics stats_;
    size_t count_;

};

void FDBPurge::usage(const std::string &tool) const {

    eckit::Log::info() << std::endl << "Usage: " << tool << " [--doit] [path1|request1] [path2|request2] ..." << std::endl;
    FDBInspect::usage(tool);
}

void FDBPurge::init(const eckit::option::CmdArgs &args) {
    args.get("doit", doit_);
}

void FDBPurge::process(const eckit::PathName &path, const eckit::option::CmdArgs &args) {

    eckit::Log::info() << "Scanning " << path << std::endl;

    fdb5::TocHandler handler(path);
    eckit::Log::info() << "Database key " << handler.databaseKey() << std::endl;

    PurgeVisitor visitor(path);

    std::vector<fdb5::Index *> indexes = handler.loadIndexes();


    for (std::vector<fdb5::Index *>::const_iterator i = indexes.begin(); i != indexes.end(); ++i) {
        (*i)->entries(visitor);
    }

    visitor.report(eckit::Log::info());

    if (doit_) {
        visitor.purge();
    }

    handler.freeIndexes(indexes);

    stats_ += visitor.totals();
    count_ ++;
}


void FDBPurge::finish(const eckit::option::CmdArgs &args) {

    if (count_ > 1) {
        eckit::Log::info() << std::endl
                           << "Grand total:" << std::endl
                           << "============" << std::endl
                           << std::endl
                           << stats_ << std::endl;
    }

    if (!doit_) {
        eckit::Log::info() << std::endl
                           << "Rerun command with --doit flag to delete unused files"
                           << std::endl
                           << std::endl;
    }

}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv) {
    FDBPurge app(argc, argv);
    return app.start();
}
