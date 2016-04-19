/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/log/Log.h"
#include "eckit/config/Resource.h"

#include "fdb5/Archiver.h"
#include "fdb5/BaseArchiveVisitor.h"

using namespace eckit;

namespace fdb5 {

BaseArchiveVisitor::BaseArchiveVisitor(Archiver& owner, const Key& field) :
    WriteVisitor(owner.prev_),
    owner_(owner),
    field_(field)
{
    checkMissingKeysOnWrite_ = eckit::Resource<bool>("checkMissingKeysOnWrite", true);
}

bool BaseArchiveVisitor::selectDatabase(const Key& key, const Key& full) {
    Log::info() << "selectDatabase " << key << std::endl;
    owner_.current_ = &owner_.session(key);
    return true;
}

bool BaseArchiveVisitor::selectIndex(const Key& key, const Key& full) {
    Log::info() << "selectIndex " << key << std::endl;
    ASSERT(owner_.current_);
    return owner_.current_->selectIndex(key);
}

void BaseArchiveVisitor::checkMissingKeys(const Key &full)
{
    if(checkMissingKeysOnWrite_)
    {
        StringSet missing;
        const StringDict& f = field_.dict();
        const StringDict& k = full.dict();

        for(StringDict::const_iterator j = f.begin(); j != f.end(); ++j) {
            if(k.find((*j).first) == k.end()) {
                missing.insert((*j).first);
            }

            if(missing.size()) {
                std::ostringstream oss;
                oss << "Keys not used in archiving: " << missing;
                throw SeriousBug(oss.str());
            }
        }
    }
}

DB* BaseArchiveVisitor::current() const
{
    return owner_.current_;
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace fdb5