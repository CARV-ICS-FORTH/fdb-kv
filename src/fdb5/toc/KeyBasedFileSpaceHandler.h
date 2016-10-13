/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   KeyBasedFileSpaceHandler.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   Oct 2016

#ifndef fdb5_KeyBasedFileSpaceHandler_H
#define fdb5_KeyBasedFileSpaceHandler_H

#include <map>
#include <string>

#include "eckit/filesystem/PathName.h"
#include "eckit/thread/Mutex.h"

#include "fdb5/toc/FileSpaceHandler.h"
#include "fdb5/database/Key.h"

namespace fdb5 {

//----------------------------------------------------------------------------------------------------------------------

class KeyBasedFileSpaceHandler : public FileSpaceHandler {

    typedef std::map<std::string, eckit::PathName>  PathTable;

public: // methods

    KeyBasedFileSpaceHandler();

    virtual ~KeyBasedFileSpaceHandler();

    virtual eckit::PathName selectFileSystem(const Key& key, const FileSpace& fs) const;

protected: // methods

    void load() const;

    void append(const std::string& expver, const eckit::PathName& path) const;

    eckit::PathName select(const std::string& expver) const;

    mutable eckit::Mutex mutex_;

    mutable PathTable table_;
};

//----------------------------------------------------------------------------------------------------------------------

} // namespace fdb5

#endif
