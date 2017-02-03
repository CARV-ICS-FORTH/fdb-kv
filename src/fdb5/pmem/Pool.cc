/*
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-3.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


/// @author Simon Smart
/// @date   Feb 2016


#include "eckit/log/Log.h"
#include "eckit/log/TimeStamp.h"

#include "pmem/PersistentPtr.h"
#include "pmem/Exceptions.h"

#include "fdb5/pmem/PBaseNode.h"
#include "fdb5/pmem/PBranchingNode.h"
#include "fdb5/pmem/PDataRoot.h"
#include "fdb5/pmem/PIndexRoot.h"
#include "fdb5/pmem/PRoot.h"
#include "fdb5/pmem/Pool.h"

using namespace eckit;
using namespace pmem;


template<> uint64_t pmem::PersistentType<fdb5::pmem::PRoot>::type_id = POBJ_ROOT_TYPE_NUM;

template<> uint64_t pmem::PersistentType<fdb5::pmem::PBaseNode>::type_id = 1;
template<> uint64_t pmem::PersistentType<fdb5::pmem::PBranchingNode>::type_id = 2;
template<> uint64_t pmem::PersistentType<fdb5::pmem::PDataNode>::type_id = 3;
template<> uint64_t pmem::PersistentType<pmem::PersistentVectorData<fdb5::pmem::PBaseNode> >::type_id = 4;
template<> uint64_t pmem::PersistentType<pmem::PersistentPODVectorData<uint64_t> >::type_id = 5;
template<> uint64_t pmem::PersistentType<pmem::PersistentBuffer>::type_id = 6;
template<> uint64_t pmem::PersistentType<pmem::PersistentString>::type_id = 7;
template<> uint64_t pmem::PersistentType<fdb5::pmem::PIndexRoot>::type_id = 8;
template<> uint64_t pmem::PersistentType<fdb5::pmem::PDataRoot>::type_id = 9;


// --------------------------------------------------------------------------------------------------

namespace fdb5 {
namespace pmem {

// -------------------------------------------------------------------------------------------------

Pool::Pool(const PathName& path, const std::string& name) :
    PersistentPool(path, name) {

    ASSERT(baseRoot().valid());
    ASSERT(baseRoot()->valid());
    ASSERT(root().valid());

    Log::info() << "Opened persistent pool created at: " << TimeStamp(root().created()) << std::endl;
}




Pool::Pool(const PathName& path, const size_t size, const std::string& name,
                   const AtomicConstructor<PRoot>& constructor) :
    PersistentPool(path, size, name, constructor) {}


Pool::~Pool() {}


/// Open an existing persistent pool, if it exists. If it does _not_ exist, then create it.
/// If open/create fail for other reasons, then the appropriate error is thrown.
///
/// @arg path - Specifies the directory to open, rather than the pool size itself.
Pool* Pool::obtain(const PathName &poolDir, const size_t size, const Key& dbKey) {

    // The pool must exist as a file within a directory.
    // n.b. PathName::mkdir uses mkdir_if_not_exists internally, so works OK if another process gets ahead.
    if (!poolDir.exists())
        poolDir.mkdir();
    ASSERT(poolDir.isDir());

    Pool* pool = 0;


    if(exists(poolDir)) {
        Log::info() << "Opening FDB PMem master pool  " << poolDir << std::endl;
        pool = new Pool(poolMaster(poolDir), "pmem-pool");
    }
    else {
        Log::info() << "Creating FDB PMem master pool " << poolDir << std::endl;
        pool = new Pool(poolMaster(poolDir), size, "pmem-pool", PRoot::ConstructorKey(PRoot::IndexClass, dbKey));
    }

    ASSERT(pool != 0);
    return pool;
}


bool Pool::exists(const PathName &poolDir) {
    return poolMaster(poolDir).exists();
}


eckit::PathName Pool::poolMaster(const PathName &poolDir) {

    return poolDir / "master";
}


PersistentPtr<PRoot> Pool::baseRoot() const {

    PersistentPtr<PRoot> rt = getRoot<PRoot>();
    ASSERT(rt.valid());
    ASSERT(rt->valid());
    ASSERT(rt->root_class() == PRoot::IndexClass);

    return rt;
}


PIndexRoot& Pool::root() const {

    PIndexRoot& rt = baseRoot()->indexRoot();

    ASSERT(rt.valid());
    return rt;
}


// -------------------------------------------------------------------------------------------------

} // namespace pmem
} // namespace tree
