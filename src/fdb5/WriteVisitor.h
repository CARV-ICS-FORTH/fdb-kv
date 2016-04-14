/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   WriteVisitor.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#ifndef fdb5_WriteVisitor_H
#define fdb5_WriteVisitor_H

#include <iosfwd>
#include <vector>

#include "eckit/memory/NonCopyable.h"
#include "eckit/types/Types.h"

#include "fdb5/Key.h"

class MarsRequest;

namespace fdb5 {

class Rule;

//----------------------------------------------------------------------------------------------------------------------

class WriteVisitor : public eckit::NonCopyable {
public: // methods

    WriteVisitor(std::vector<Key>&);
    virtual ~WriteVisitor();

    virtual bool selectDatabase(const Key& key, const Key& full) = 0;
    virtual bool selectIndex(const Key& key, const Key& full) = 0;
    virtual bool selectDatum(const Key& key, const Key& full) = 0;

    void rule(const Rule* r) { rule_ = r; }
    const Rule* rule() const { return rule_; }

private: // members

    friend class Rule;

    std::vector<Key>& prev_;

    const Rule* rule_; // Last rule used

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace fdb5

#endif
