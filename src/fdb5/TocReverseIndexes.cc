/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "fdb5/TocReverseIndexes.h"
#include "fdb5/Key.h"


namespace fdb5 {

TocReverseIndexes::TocReverseIndexes(const eckit::PathName &dir) :
    TocHandler(dir),
    inited_(false) {
}

void TocReverseIndexes::init() {

    if (inited_) {
        return;
    }

    eckit::Log::info() << "Initing TocReverseIndexes @ " << filePath_ << std::endl;

    openForRead();

    TocRecord r;

    while ( readNext(r) ) {
        //eckit::Log::info() << "TOC Record: " << r << std::endl;
        toc_.push_back(r);
    }

    close();

    inited_ = true;
}

std::vector<eckit::PathName> TocReverseIndexes::indexes(const Key &key) const {

    const_cast<TocReverseIndexes *>(this)->init();

    TocMap::const_iterator f = cacheIndexes_.find(key);

    if ( f != cacheIndexes_.end() )
        return f->second;

    // not in cache

    TocRecord::MetaData md( key.valuesToString() );

    eckit::Log::info() << "TocReverseIndexes md = " << md << std::endl;
    eckit::Log::info() << "TocReverseIndexes key = " << key << std::endl;

    std::vector< eckit::PathName > indexes;

    for ( TocVec::const_iterator itr = toc_.begin(); itr != toc_.end(); ++itr ) {
        const TocRecord &r = *itr;
        eckit::Log::info() << "TocRecord " << r << std::endl;
        switch (r.head_.tag_) {
        case TOC_INIT: // ignore the Toc initialisation
            break;

        case TOC_INDEX:
            eckit::Log::info() << "TOC_INDEX " << r.metadata() << std::endl;
            if ( r.metadata() == md )
                indexes.push_back( r.path() );
            break;

        case TOC_CLEAR:
            if ( r.metadata() == md )
                indexes.erase( std::remove( indexes.begin(), indexes.end(), r.path() ), indexes.end() );
            break;

        case TOC_WIPE:
            indexes.clear();
            break;

        default:
            throw eckit::SeriousBug("Unknown tag in TocRecord", Here());
            break;
        }
    }

    std::reverse(indexes.begin(), indexes.end()); // the entries of the last index takes precedence

    cacheIndexes_[ key ] = indexes; // cache before returning

    return indexes;
}

//-----------------------------------------------------------------------------

} // namespace fdb5
