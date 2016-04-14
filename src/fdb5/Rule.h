/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   Rule.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#ifndef fdb5_Rule_H
#define fdb5_Rule_H

#include <iosfwd>
#include <vector>

#include "eckit/memory/NonCopyable.h"
#include "eckit/types/Types.h"

class MarsRequest;

namespace fdb5 {

class Predicate;
class ReadVisitor;
class WriteVisitor;
class Key;

//----------------------------------------------------------------------------------------------------------------------

class Rule : public eckit::NonCopyable {

public: // methods

    /// Takes ownership of vectors
    Rule(std::vector<Predicate*>& predicates, std::vector<Rule*>& rules);

    ~Rule();

    bool match(const Key& key) const;

    eckit::StringList keys(size_t level) const;

    void dump(std::ostream& s, size_t depth = 0) const;

    void expand(const MarsRequest& request,
        ReadVisitor& Visitor,
        size_t depth,
        std::vector<fdb5::Key>& keys,
        Key& full) const;

    void expand(const Key& field,
                WriteVisitor& Visitor,
                size_t depth,
                std::vector<fdb5::Key>& keys,
                Key& full) const;



    size_t depth() const;

private: // methods

    void expand(const MarsRequest& request,
                 std::vector<Predicate*>::const_iterator cur,
                 size_t depth,
                 std::vector<Key>& keys,
                 Key& full,
                 ReadVisitor& Visitor) const;

    void expand(const Key& field,
             std::vector<Predicate*>::const_iterator cur,
             size_t depth,
             std::vector<Key>& keys,
             Key& full,
             WriteVisitor& Visitor) const;


    void keys(size_t level, size_t depth, eckit::StringList&result, eckit::StringSet& seen) const;

    friend std::ostream& operator<<(std::ostream& s,const Rule& x);

    void print( std::ostream& out ) const;

private: // members

    std::vector<Predicate*> predicates_;
    std::vector<Rule*>      rules_;

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace fdb5

#endif