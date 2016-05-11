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
#include "fdb5/database/Index.h"
#include "fdb5/rules/Schema.h"
#include "fdb5/toc/TocHandler.h"
#include "fdb5/tools/FDBInspect.h"


//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------

class FDBDump : public fdb5::FDBInspect {

  public: // methods

    FDBDump(int argc, char **argv) : fdb5::FDBInspect(argc, argv) {

    }

  private: // methods

    virtual void usage(const std::string &tool) const;
    virtual void process(const eckit::PathName &path, const eckit::option::CmdArgs &args);
    virtual int minimumPositionalArguments() const { return 1; }

};

void FDBDump::usage(const std::string &tool) const {
    fdb5::FDBInspect::usage(tool);
}

void FDBDump::process(const eckit::PathName &path, const eckit::option::CmdArgs &args) {

    eckit::Log::info() << "Dumping " << path << std::endl << std::endl;

    fdb5::TocHandler handler(path);
    handler.dump(eckit::Log::info());

    // eckit::Log::info() << std::endl;
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv) {
    FDBDump app(argc, argv);
    return app.start();
}
