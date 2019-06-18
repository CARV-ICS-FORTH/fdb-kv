/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/serialisation/Reanimator.h"

#include "fdb5/api/helpers/WipeIterator.h"
#include "fdb5/database/IndexLocation.h"

namespace fdb5 {

//----------------------------------------------------------------------------------------------------------------------

WipeElement::WipeElement(eckit::Stream& s) {
    s >> owner;
    s >> metadataPaths;
    s >> dataPaths;
    s >> safePaths;

    size_t nIndexes;
    indexes.reserve(nIndexes);
    for (size_t i = 0; i < nIndexes; i++) {
        indexes.emplace_back(std::shared_ptr<IndexLocation>(eckit::Reanimator<IndexLocation>::reanimate(s)));
    }
}

// This routine returns a very crude size (which will be rounded to page size)
// to allow a guessing of the eckit::buffer size needed to encode this object
// into a stream

size_t WipeElement::guessEncodedSize() const {

    // Sizes to use for a guess (include arbitrary size for stream-internal sizes)

    const size_t objTag = 32;

    size_t totalSize = objTag;

    // Owner
    totalSize += objTag + owner.size();

    // metadataPaths
    totalSize += objTag;
    for (const auto& p : metadataPaths) { totalSize += objTag + p.size(); }

    // dataPaths
    totalSize += objTag;
    for (const auto& p : dataPaths) { totalSize += objTag + p.size(); }

    return totalSize;
}

void WipeElement::encode(eckit::Stream &s) const {
    s << owner;
    s << metadataPaths;
    s << dataPaths;
    s << safePaths;

    size_t nIndexes = indexes.size();
    s << nIndexes;
    for (const std::shared_ptr<const IndexLocation>& loc : indexes) {
        s << *loc;
    }
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace fdb5

