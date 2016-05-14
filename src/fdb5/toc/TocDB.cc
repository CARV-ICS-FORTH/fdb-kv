/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/log/Timer.h"

#include "fdb5/config/MasterConfig.h"
#include "fdb5/rules/Rule.h"
#include "fdb5/toc/Root.h"
#include "fdb5/toc/TocDB.h"

using namespace eckit;

namespace fdb5 {

//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------

TocDB::TocDB(const Key& key) :
    DB(key),
    TocHandler(Root::directory(key)) {
}

TocDB::TocDB(const eckit::PathName& directory) :
    DB(Key()),
    TocHandler(directory) {
}

TocDB::~TocDB() {
}

void TocDB::axis(const std::string &keyword, eckit::StringSet &s) const {
    Log::error() << "axis() not implemented for " << *this << std::endl;
    NOTIMP;
}

bool TocDB::open() {
    Log::error() << "Open not implemented for " << *this << std::endl;
    NOTIMP;
}

void TocDB::archive(const Key &key, const void *data, Length length) {
    Log::error() << "Archive not implemented for " << *this << std::endl;
    NOTIMP;
}

void TocDB::flush() {
    Log::error() << "Flush not implemented for " << *this << std::endl;
    NOTIMP;
}

eckit::DataHandle *TocDB::retrieve(const Key &key) const {
    Log::error() << "Retrieve not implemented for " << *this << std::endl;
    NOTIMP;
}

void TocDB::close() {
    Log::error() << "Close not implemented for " << *this << std::endl;
    NOTIMP;
}

void TocDB::loadSchema() {
    Timer timer("TocDB::loadSchema()");
    schema_.load( schemaPath() );
}

void TocDB::checkSchema(const Key &key) const {
    Timer timer("TocDB::checkSchema()");
    ASSERT(key.rule());
    schema_.compareTo(key.rule()->schema());
}

const Schema& TocDB::schema() const {
    return schema_;
}
//----------------------------------------------------------------------------------------------------------------------


std::vector<eckit::PathName> TocDB::databases(const Key &key) {

    const Schema& schema = MasterConfig::instance().schema();
    std::set<Key> keys;
    schema.matchFirstLevel(key, keys);

    std::vector<eckit::PathName> dirs = Root::roots(); // TODO: filter roots() with key
    std::vector<eckit::PathName> result;
    std::set<eckit::PathName> seen;

    for (std::vector<eckit::PathName>::const_iterator j = dirs.begin(); j != dirs.end(); ++j) {

        std::vector<eckit::PathName> subdirs;
        eckit::PathName::match((*j) / "*:*", subdirs, false);

        for (std::set<Key>::const_iterator i = keys.begin(); i != keys.end(); ++i) {


            Regex re("^" + (*i).valuesToString() + "$");
            // std::cout << re << std::endl;


            for (std::vector<eckit::PathName>::const_iterator k = subdirs.begin(); k != subdirs.end(); ++k) {

                if(seen.find(*k) != seen.end()) {
                    continue;
                }

                if (re.match((*k).baseName())) {
                    try {
                        TocHandler toc(*k);
                        if (toc.databaseKey().match(key)) {
                            result.push_back(*k);
                        }
                    } catch (eckit::Exception& e) {
                        eckit::Log::error() <<  "Error loading FDB database from " << *k << std::endl;
                        eckit::Log::error() << e.what() << std::endl;
                    }
                    seen.insert(*k);;
                }

            }
        }
    }

    return result;
}


} // namespace fdb5
