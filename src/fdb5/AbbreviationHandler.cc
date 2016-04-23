/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "fdb5/AbbreviationHandler.h"

#include "marslib/MarsRequest.h"

#include "fdb5/KeywordType.h"

namespace fdb5 {

//----------------------------------------------------------------------------------------------------------------------

AbbreviationHandler::AbbreviationHandler(const std::string& name, const std::string& type) :
    KeywordHandler(name, type)
{
    count_ = char(type[type.length()-1]) - '0';
}

AbbreviationHandler::~AbbreviationHandler()
{
}

void AbbreviationHandler::toKey(std::ostream& out,
                       const std::string& keyword,
                       const std::string& value) const {

    out << value.substr(0, count_);
}

void AbbreviationHandler::getValues(const MarsRequest& request,
                               const std::string& keyword,
                               eckit::StringList& values,
                               const MarsTask& task,
                               const DB* db) const
{
    std::vector<std::string> vals;

    request.getValues(keyword, vals);

    values.reserve(vals.size());

    for(std::vector<std::string>::const_iterator i = vals.begin(); i != vals.end(); ++i) {
        values.push_back((*i).substr(0, count_));
    }
}

void AbbreviationHandler::print(std::ostream &out) const
{
    out << "AbbreviationHandler(" << name_ << ")";
}

static KeywordHandlerBuilder<AbbreviationHandler> handler("First3");

//----------------------------------------------------------------------------------------------------------------------

} // namespace fdb5
