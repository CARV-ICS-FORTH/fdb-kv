/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/config/Resource.h"

#include "fdb5/Handlers.h"
#include "fdb5/KeywordType.h"
#include "fdb5/KeywordHandler.h"

using namespace eckit;

namespace fdb5 {

//----------------------------------------------------------------------------------------------------------------------

Handlers::Handlers()
{
}

Handlers::~Handlers()
{
    for(HandlerMap::iterator i = handlers_.begin(); i != handlers_.end(); ++i) {
        delete (*i).second;
    }
}

const KeywordHandler& Handlers::lookupHandler(const std::string& keyword) const
{
    std::map<std::string, KeywordHandler*>::const_iterator j = handlers_.find(keyword);

    if (j != handlers_.end()) {
        return *(*j).second;
    }
    else {
        std::string type = Resource<std::string>(keyword+"FdbType","Default");
        KeywordHandler* newKH = KeywordType::build(type, keyword);
        handlers_[keyword] = newKH;
        return *newKH;
    }
}


//----------------------------------------------------------------------------------------------------------------------

} // namespace fdb5