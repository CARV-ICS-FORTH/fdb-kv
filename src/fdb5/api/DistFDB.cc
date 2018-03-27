/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/config/LocalConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/parser/Tokenizer.h"

#include "fdb5/LibFdb.h"
#include "fdb5/dist/DistFDB.h"
#include "fdb5/io/HandleGatherer.h"


namespace fdb5 {

static FDBBuilder<DistFDB> distFdbBuilder("dist");

//----------------------------------------------------------------------------------------------------------------------


struct DistributionError : public eckit::Exception {
    DistributionError(const std::string& r, const eckit::CodeLocation& loc) :
        Exception(std::string(" DistributionError: " + r, loc)) {}
};

//----------------------------------------------------------------------------------------------------------------------

DistFDB::DistFDB(const eckit::Configuration& config) :
    FDBBase(config) {

    ASSERT(config.getString("type", "") == "dist");

    // Configure the available lanes.

    if (!config.has("lanes")) throw eckit::UserError("No lanes configured for pool", Here());

    std::vector<eckit::LocalConfiguration> laneConfigs;
    laneConfigs = config.getSubConfigurations("lanes");

    for(const eckit::LocalConfiguration& laneCfg : laneConfigs) {
        lanes_.push_back(FDB(laneCfg));
        hash_.addNode(lanes_.back().id());
    }
}

DistFDB::~DistFDB() {}


void DistFDB::archive(const Key& key, const void* data, size_t length) {

    std::vector<size_t> laneIndices;
    hash_.hashOrder(key.keyDict(), laneIndices);

    // Given an order supplied by the Rendezvous hash, try the FDB in order until
    // one works. n.b. Errors are unacceptable once the FDB is dirty.

    for (size_t idx : laneIndices) {

        FDB& lane(lanes_[idx]);

        if (!lane.writable()) {
            eckit::Log::info() << lane << " Not writable" << std::endl;
            continue;
        }

        try {

            lane.archive(key, data, length);
            return;

        } catch (eckit::Exception& e) {

            // TODO: This will be messy and verbose. Reduce output if it has already failed.

            std::stringstream ss;
            ss << "Archive failure on lane: " << lane << " (" << idx << ")";
            eckit::Log::info() << ss.str() << std::endl;
            eckit::Log::info() << "with exception: " << e << std::endl;

            // If we have written, but not flushed, data to a give lane, and an archive operation
            // fails, then this is a bit of an issue. Otherwise, just skip the lane.

            if (lane.dirty()) {
                ss << " -- Exception: " << e;
                throw DistributionError(ss.str(), Here());
            }

            // Mark the lane as no longer writable
            lane.setNonWritable();
        }
    }

    throw DistributionError("No writable lanes available for archive", Here());
}


eckit::DataHandle *DistFDB::retrieve(const MarsRequest &request) {

    // TODO: Deduplication. Currently no masking.
    // TODO: Error handling on read.

//    HandleGatherer result(true); // Sorted
    HandleGatherer result(false);

    for (FDB& lane : lanes_) {
        if (lane.visitable()) {
            result.add(lane.retrieve(request));
        }
    }

    return result.dataHandle();
}


std::string DistFDB::id() const {
    NOTIMP;
}


void DistFDB::flush() {
    for (FDB& lane : lanes_) {
        lane.flush();
    }
}


void DistFDB::print(std::ostream &s) const {
    s << "DistFDB(home=" << config_.expandPath("~fdb") << ")";
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace fdb5