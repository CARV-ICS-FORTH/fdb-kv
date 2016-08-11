/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   GribIndexer.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   Mar 2016

#ifndef fdb5_GribIndexer_H
#define fdb5_GribIndexer_H

#include "eckit/filesystem/PathName.h"

#include "fdb5/database/Archiver.h"
#include "fdb5/grib/GribDecoder.h"

namespace eckit   { class DataHandle; }

namespace fdb5 {

//----------------------------------------------------------------------------------------------------------------------

class GribIndexer : public Archiver, public GribDecoder {

public: // methods

    GribIndexer(bool checkDuplicates = false);

    void index(const eckit::PathName& path);

private: // members


};

//----------------------------------------------------------------------------------------------------------------------

} // namespace fdb5

#endif
