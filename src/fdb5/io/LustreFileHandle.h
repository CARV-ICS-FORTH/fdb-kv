/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   LustreFileHandle.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   October 2016

#ifndef fdb5_LustreFileHandle_h
#define fdb5_LustreFileHandle_h

#include "mars_server_config.h"

#include "eckit/io/Length.h"
#include "eckit/config/Resource.h"
#include "eckit/exception/Exceptions.h"

namespace fdb5 {

int fdb5LustreapiFileCreate(const char* path, size_t stripesize, size_t stripecount);

//----------------------------------------------------------------------------------------------------------------------

struct LustreStripe {

    LustreStripe(unsigned int count, size_t size) :
        count_(count),
        size_(size)
    {
    }

    unsigned int count_;
    size_t size_;
};


template< class HANDLE >
class LustreFileHandle : public HANDLE {

public: // methods

    LustreFileHandle(const std::string& path, LustreStripe stripe) :
        HANDLE(path),
        stripe_(stripe)
    {
    }

    LustreFileHandle(const std::string& path, size_t buffsize, LustreStripe stripe) :
        HANDLE(path, buffsize),
        stripe_(stripe)
    {
    }

    LustreFileHandle(const std::string& path, size_t buffcount, size_t buffsize, LustreStripe stripe) :
        HANDLE(path, buffcount, buffsize),
        stripe_(stripe)
    {
    }

    virtual ~LustreFileHandle() {}

    virtual void openForAppend(const eckit::Length& len) {

            /* From the docs: llapi_file_create closes the file descriptor. You must re-open the file afterwards */

            std::string path = HANDLE::path_;

            int err = fdb5LustreapiFileCreate(path.c_str(), stripe_.size_, stripe_.count_);

            if(err == EINVAL) {

                std::ostringstream oss;
                oss << "Invalid stripe parameters for Lustre file system"
                    << " - stripe count " << stripe_.count_
                    << " - stripe size "  << stripe_.size_;

                throw eckit::BadParameter(oss.str(), Here());
            }

            if(err && err != EEXIST) {
                throw eckit::FailedSystemCall("llapi_file_create", Here());
            }

        this->HANDLE::openForAppend(len);
    }

private: // members

    LustreStripe stripe_;

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace fdb5

#endif
