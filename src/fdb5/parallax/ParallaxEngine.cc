/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <dirent.h>

#include <algorithm>
#include <cstring>
#include <list>
#include <ostream>

#include "eckit/eckit.h"

#include "eckit/filesystem/LocalFileManager.h"
#include "eckit/filesystem/LocalPathName.h"
#include "eckit/filesystem/StdDir.h"
#include "eckit/log/Log.h"
#include "eckit/os/BackTrace.h"
#include "eckit/os/Stat.h"
#include "eckit/utils/Regex.h"

#include "fdb5/LibFdb5.h"
#include "fdb5/rules/Schema.h"
#include "fdb5/toc/RootManager.h"
#include "fdb5/parallax/ParallaxEngine.h"
#include "fdb5/toc/TocHandler.h"

using namespace eckit;


namespace fdb5 {

//----------------------------------------------------------------------------------------------------------------------

void ParallaxEngine::scan_dbs(const std::string& path, std::list<std::string>& dbs) const {
    std::cout << "File: " << __FILE__ << ", Line: " << __LINE__ << ", Function: " << __func__ << std::endl;
    if ((eckit::PathName(path) / "toc").exists()) {
        dbs.push_back(path);
        return;
    }

    eckit::StdDir d(path.c_str());
    if (d == nullptr) {
        // If fdb-wipe is running in parallel, it is perfectly legit for a (non-matching)
        // path to have disappeared
        if (errno == ENOENT) {
            return;
        }

        // It should not be an error if we don't have permission to read a path/DB in the
        // tree. This is a multi-user system.
        if (errno == EACCES) {
            return;
        }

        Log::error() << "opendir(" << path << ")" << Log::syserr << std::endl;
        throw FailedSystemCall("opendir");
    }

    // Once readdir_r finally gets deprecated and removed, we may need to 
    // protecting readdir() as not yet guarranteed thread-safe by POSIX
    // technically it should only be needed on a per-directory basis
    // this should be a resursive mutex
    // AutoLock<Mutex> lock(mutex_); 

    for(;;)
    {
        struct dirent* e = d.dirent();
        if (e == nullptr) {
            break;
        }

        if(e->d_name[0] == '.') {
            if(e->d_name[1] == 0 || (e->d_name[1] =='.' && e->d_name[2] == 0))
                continue;
        }

        std::string full = path;
        if (path[path.length()-1] != '/') full += "/";
        full += e->d_name;

        bool do_stat = true;

#if defined(eckit_HAVE_DIRENT_D_TYPE)
        do_stat = false;
        if (e->d_type == DT_DIR) {
            scan_dbs(full.c_str(), dbs);
        } else if (e->d_type == DT_UNKNOWN) {
            do_stat = true;
        }
#endif
        if(do_stat) {
            eckit::Stat::Struct info;
            if(eckit::Stat::stat(full.c_str(), &info) == 0)
            {
                if(S_ISDIR(info.st_mode)) {
                    scan_dbs(full.c_str(), dbs);
                }
            }
            else Log::error() << "Cannot stat " << full << Log::syserr << std::endl;
        }
    }
}

std::string ParallaxEngine::name() const {
    std::cout << "File: " << __FILE__ << ", Line: " << __LINE__ << ", Function: " << __func__ << std::endl;
    return ParallaxEngine::typeName();
}

std::string ParallaxEngine::dbType() const {
    std::cout << "File: " << __FILE__ << ", Line: " << __LINE__ << ", Function: " << __func__ << std::endl;
    return ParallaxEngine::typeName();
}

eckit::URI ParallaxEngine::location(const Key& key, const Config& config) const
{
    std::cout << "File: " << __FILE__ << ", Line: " << __LINE__ << ", Function: " << __func__ << std::endl;
    return URI("toc", CatalogueRootManager(config).directory(key).directory_);
}

bool ParallaxEngine::canHandle(const eckit::URI& uri) const
{
    std::cout << "File: " << __FILE__ << ", Line: " << __LINE__ << ", Function: " << __func__ << std::endl;
    if (uri.scheme() != "toc")
        return false;

    eckit::PathName path = uri.path();
    eckit::PathName toc =  path / "toc";
    return path.isDir() && toc.exists();
}

static void matchKeyToDB(const Key& key, std::set<Key>& keys, const char* missing, const Config& config)
{
    std::cout << "File: " << __FILE__ << ", Line: " << __LINE__ << ", Function: " << __func__ << std::endl;
    const Schema& schema = config.schema();
    schema.matchFirstLevel(key, keys, missing);
}

static void matchRequestToDB(const metkit::mars::MarsRequest& rq, std::set<Key>& keys, const char* missing, const Config& config)
{
    std::cout << "File: " << __FILE__ << ", Line: " << __LINE__ << ", Function: " << __func__ << std::endl;
    const Schema& schema = config.schema();
    schema.matchFirstLevel(rq, keys, missing);
}

static constexpr const char* regexForMissingValues = "[^:/]*";

std::set<eckit::PathName> ParallaxEngine::databases(const std::set<Key>& keys,
                                               const std::vector<eckit::PathName>& roots,
                                               const Config& config) const {

    std::set<eckit::PathName> result;
    std::cout << "File: " << __FILE__ << ", Line: " << __LINE__ << ", Function: " << __func__ << std::endl;
    for (std::vector<eckit::PathName>::const_iterator j = roots.begin(); j != roots.end(); ++j) {

        LOG_DEBUG_LIB(LibFdb5) << "Scanning for TOC FDBs in root " << *j << std::endl;

        std::list<std::string> dbs;
        scan_dbs(*j, dbs);

        for (std::set<Key>::const_iterator i = keys.begin(); i != keys.end(); ++i) {

            std::vector<std::string> dbpaths = CatalogueRootManager(config).possibleDbPathNames(*i, regexForMissingValues);

            for(std::vector<std::string>::const_iterator dbpath = dbpaths.begin(); dbpath != dbpaths.end(); ++dbpath) {

                Regex re("^" + Regex::escape(j->asString()) + "/" + *dbpath + "$");

                LOG_DEBUG_LIB(LibFdb5) << " -> key i " << *i
                                     << " dbpath " << *dbpath
                                     << " pathregex " << re << std::endl;

                for (std::list<std::string>::const_iterator k = dbs.begin(); k != dbs.end(); ++k) {

                    LOG_DEBUG_LIB(LibFdb5) << "    -> db " << *k << std::endl;

                    if(result.find(*k) != result.end()) {
                        continue;
                    }

                    if (re.match(*k)) {
                        result.insert(*k);
                    }
                }
            }
        }
    }

    LOG_DEBUG_LIB(LibFdb5) << "ParallaxEngine::databases() results " << result << std::endl;

    return result;
}

std::vector<eckit::URI> ParallaxEngine::databases(const Key& key,
                                                  const std::vector<eckit::PathName>& roots,
                                                  const Config& config) const {

    std::set<Key> keys;
    std::cout << "File: " << __FILE__ << ", Line: " << __LINE__ << ", Function: " << __func__ << std::endl;
    matchKeyToDB(key, keys, regexForMissingValues, config);

    LOG_DEBUG_LIB(LibFdb5) << "Matched DB schemas for key " << key << " -> keys " << keys << std::endl;

    std::set<eckit::PathName> databasesMatchRegex(databases(keys, roots, config));

    std::vector<eckit::URI> result;
    for (const auto& path : databasesMatchRegex) {
        try {
            TocHandler toc(path, config);
            if (toc.databaseKey().match(key)) {
                LOG_DEBUG_LIB(LibFdb5) << " found match with " << path << std::endl;
                result.push_back(eckit::URI("toc", path));
            }
        } catch (eckit::Exception& e) {
            eckit::Log::error() <<  "Error loading FDB database from " << path << std::endl;
            eckit::Log::error() << e.what() << std::endl;
        }
    }

    return result;
}

std::vector<eckit::URI> ParallaxEngine::databases(const metkit::mars::MarsRequest& request,
                                                  const std::vector<eckit::PathName>& roots,
                                                  const Config& config) const {

    std::set<Key> keys;
    std::cout << "File: " << __FILE__ << ", Line: " << __LINE__ << ", Function: " << __func__ << std::endl;
//    matchRequestToDB(request, keys, regexForMissingValues, config);
    matchRequestToDB(request, keys, "", config);

    LOG_DEBUG_LIB(LibFdb5) << "Matched DB schemas for request " << request << " -> keys " << keys << std::endl;

    std::set<eckit::PathName> databasesMatchRegex(databases(keys, roots, config));

    std::vector<eckit::URI> result;
    for (const auto& path : databasesMatchRegex) {
        try {
            TocHandler toc(path, config);
            if (toc.databaseKey().partialMatch(request)) {
                LOG_DEBUG_LIB(LibFdb5) << " found match with " << path << std::endl;
                result.push_back(eckit::URI("toc", path));
            }
        } catch (eckit::Exception& e) {
            eckit::Log::error() <<  "Error loading FDB database from " << path << std::endl;
            eckit::Log::error() << e.what() << std::endl;
        }
    }

    return result;
}

std::vector<eckit::URI> ParallaxEngine::allLocations(const Key& key, const Config& config) const
{
    std::cout << "File: " << __FILE__ << ", Line: " << __LINE__ << ", Function: " << __func__ << std::endl;
    return databases(key, CatalogueRootManager(config).allRoots(key), config);
}

std::vector<eckit::URI> ParallaxEngine::visitableLocations(const Key& key, const Config& config) const
{
    std::cout << "File: " << __FILE__ << ", Line: " << __LINE__ << ", Function: " << __func__ << std::endl;
    return databases(key, CatalogueRootManager(config).visitableRoots(key), config);
}

std::vector<URI> ParallaxEngine::visitableLocations(const metkit::mars::MarsRequest& request, const Config& config) const
{
    std::cout << "File: " << __FILE__ << ", Line: " << __LINE__ << ", Function: " << __func__ << std::endl;
    return databases(request, CatalogueRootManager(config).visitableRoots(request), config);
}

std::vector<eckit::URI> ParallaxEngine::writableLocations(const Key& key, const Config& config) const
{
    std::cout << "File: " << __FILE__ << ", Line: " << __LINE__ << ", Function: " << __func__ << std::endl;
    return databases(key, CatalogueRootManager(config).canArchiveRoots(key), config);
}

void ParallaxEngine::print(std::ostream& out) const
{
    out << "ParallaxEngine()";
}

static EngineBuilder<ParallaxEngine> toc_builder;

static eckit::LocalFileManager manager_toc("parallax");

//----------------------------------------------------------------------------------------------------------------------

} // namespace fdb5