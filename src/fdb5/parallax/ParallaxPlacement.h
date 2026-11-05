/*
 * Shared server placement for the Parallax backend.
 *
 * Without this, the server that stores a dataset is a fixed function of names (djb2 of the DB name), so many
 * writers can pile onto the same server. With -DFDB_PARALLAX_PLACEMENT (CMake option FDB_PARALLAX_PLACEMENT) and the
 * environment variable FDB_PARALLAX_PLACEMENT_FILE pointing to a file on a filesystem visible to all clients:
 *   - a writer, before it opens any Parallax DB, picks the least loaded server (fewest datasets assigned), records
 *     "<dataset prefix> <server index>" in the file under a lock and connects all its DBs to that server;
 *   - a reader looks the dataset up in the same file (waiting a little if the writer has not registered yet) and
 *     connects to the recorded server.
 * Without the compile flag or without the environment variable nothing changes.
 */
#ifndef FDB5_PARALLAX_PLACEMENT_H
#define FDB5_PARALLAX_PLACEMENT_H

#include <cstdint>
#include <string>

namespace fdb5
{
/** Same FNV-1a hash the catalogues use to turn a dataset key into the id prefix. */
uint32_t parallax_dataset_prefix(const std::string &dataset_name);

/** Writer side: choose (or reuse) the server for this dataset and make this process use it. */
void parallax_placement_assign(const std::string &dataset_name);

/** Reader side: find the server recorded for this dataset and make this process use it. */
void parallax_placement_lookup(const std::string &dataset_name);
}

#endif
