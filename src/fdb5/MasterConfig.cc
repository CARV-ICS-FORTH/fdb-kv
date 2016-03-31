/*
 * (C) Copyright 1996-2016 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/exception/Exceptions.h"

#include "fdb5/MasterConfig.h"


using namespace eckit;

namespace fdb {

//----------------------------------------------------------------------------------------------------------------------

MasterConfig::MasterConfig()
{
}

MasterConfig::~MasterConfig()
{
}

std::vector<TOC> MasterConfig::findTOCs(const FdbTask& task)
{
    NOTIMP;
    return std::vector<TOC>();
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace fdb