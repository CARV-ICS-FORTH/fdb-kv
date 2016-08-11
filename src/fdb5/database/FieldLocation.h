/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date Sep 2012

#ifndef fdb5_FieldLocation_H
#define fdb5_FieldLocation_H

#include "eckit/eckit.h"

#include "eckit/filesystem/PathName.h"
#include "eckit/io/Length.h"
#include "eckit/io/Offset.h"
#include "eckit/memory/NonCopyable.h"
#include "eckit/types/Types.h"
#include "eckit/types/FixedString.h"

#include "fdb5/database/FileStore.h"
#include "fdb5/database/IndexAxis.h"
#include "fdb5/database/Key.h"

namespace fdb5 {


//----------------------------------------------------------------------------------------------------------------------

class FieldLocation {
public:

    FieldLocation();

    FieldLocation(const eckit::PathName &path, eckit::Offset offset, eckit::Length length ) ;

    const eckit::PathName &path() const { return path_; }
    const eckit::Offset &offset() const { return offset_; }
    const eckit::Length &length() const { return length_; }

    eckit::DataHandle *dataHandle() const ;

private:

    eckit::PathName path_;
    eckit::Offset   offset_;
    eckit::Length   length_;

    void print( std::ostream &out ) const;

    friend std::ostream &operator<<(std::ostream &s, const FieldLocation &x) {
        x.print(s);
        return s;
    }
};

//----------------------------------------------------------------------------------------------------------------------

} // namespace fdb5

#endif
