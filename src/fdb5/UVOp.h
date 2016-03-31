/*
 * (C) Copyright 1996-2016 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   UVOp.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   Mar 2016

#ifndef fdb_UVOp_H
#define fdb_UVOp_H

#include "fdb5/Op.h"
#include "fdb5/Winds.h"

namespace fdb {

//----------------------------------------------------------------------------------------------------------------------

class UVOp : public fdb::Op {

public: // methods

    UVOp(Op& parent, const Winds& winds);

    /// Destructor
    
    virtual ~UVOp();

    virtual void descend();

    virtual void execute(const FdbTask& task, marskit::MarsRequest& field);

    virtual void fail(const FdbTask& task, marskit::MarsRequest& field);

private:

    Op& parent_;
    Winds winds_;
};

//----------------------------------------------------------------------------------------------------------------------

} // namespace fdb

#endif