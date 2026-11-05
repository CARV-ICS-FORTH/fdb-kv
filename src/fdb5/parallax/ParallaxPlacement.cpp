#include "fdb5/parallax/ParallaxPlacement.h"

#include "parallax.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

namespace fdb5
{
uint32_t parallax_dataset_prefix(const std::string &name)
{
	uint32_t hash = 2166136261u;
	for (char c : name) {
		hash ^= static_cast<uint8_t>(c);
		hash *= 16777619u;
	}
	return hash;
}

#ifdef FDB_PARALLAX_PLACEMENT
namespace
{
const char *placement_file()
{
	const char *p = std::getenv("FDB_PARALLAX_PLACEMENT_FILE");
	return (p && *p) ? p : nullptr;
}

// Portable lock (works on NFS and Lustre without flock support): a directory created atomically with mkdir.
class DirLock {
    public:
	explicit DirLock(const std::string &base)
		: dir_(base + ".lock")
	{
		for (int i = 0; i < 20000; ++i) {
			if (::mkdir(dir_.c_str(), 0755) == 0) {
				held_ = true;
				return;
			}
			if (errno != EEXIST)
				break;
			struct stat st;
			if (::stat(dir_.c_str(), &st) == 0 && ::time(nullptr) - st.st_mtime > 60)
				::rmdir(dir_.c_str()); // stale lock left by a crashed process
			::usleep(2000 + (::rand() % 4000));
		}
		std::cerr << "fdb5 parallax placement: could not lock " << dir_ << std::endl;
	}
	~DirLock()
	{
		if (held_)
			::rmdir(dir_.c_str());
	}
	bool held() const { return held_; }

    private:
	std::string dir_;
	bool held_ = false;
};

std::map<uint32_t, int> read_registry(const char *path)
{
	std::map<uint32_t, int> m;
	std::ifstream in(path);
	std::string line;
	while (std::getline(in, line)) {
		if (line.empty() || line[0] == '#')
			continue;
		std::istringstream ls(line);
		unsigned long long pre;
		int idx;
		if (ls >> pre >> idx)
			m[static_cast<uint32_t>(pre)] = idx;
	}
	return m;
}
} // namespace

void parallax_placement_assign(const std::string &dataset_name)
{
	const char *path = placement_file();
	if (!path)
		return;
	const int servers = par_get_num_of_servers();
	if (servers <= 0)
		return;
	const uint32_t pre = parallax_dataset_prefix(dataset_name);

	DirLock lock(path);
	if (!lock.held())
		return; // fall back to the default hash placement
	std::map<uint32_t, int> reg = read_registry(path);

	int chosen;
	auto it = reg.find(pre);
	if (it != reg.end()) {
		chosen = it->second % servers;
	} else {
		std::vector<int> load(servers, 0);
		for (auto &kv : reg)
			load[kv.second % servers]++;
		chosen = 0;
		for (int s = 1; s < servers; ++s)
			if (load[s] < load[chosen])
				chosen = s;
		int fd = ::open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
		if (fd < 0) {
			std::cerr << "fdb5 parallax placement: cannot open " << path << ": " << strerror(errno) << std::endl;
			return;
		}
		std::string rec = std::to_string(pre) + " " + std::to_string(chosen) + "\n";
		if (::write(fd, rec.data(), rec.size()) != static_cast<ssize_t>(rec.size()))
			std::cerr << "fdb5 parallax placement: short write to " << path << std::endl;
		::fsync(fd);
		::close(fd);
	}
	par_set_server_index(chosen);
	std::cerr << "fdb5 parallax placement: dataset " << dataset_name << " (prefix " << pre << ") -> server " << chosen
		  << " of " << servers << std::endl;
}

void parallax_placement_lookup(const std::string &dataset_name)
{
	const char *path = placement_file();
	if (!path)
		return;
	const int servers = par_get_num_of_servers();
	if (servers <= 0)
		return;
	const uint32_t pre = parallax_dataset_prefix(dataset_name);
	const int timeout_s = 30;
	for (int waited = 0; waited <= timeout_s * 10; ++waited) {
		std::map<uint32_t, int> reg = read_registry(path); // append-only: no lock needed to read
		auto it = reg.find(pre);
		if (it != reg.end()) {
			par_set_server_index(it->second % servers);
			std::cerr << "fdb5 parallax placement: dataset " << dataset_name << " (prefix " << pre
				  << ") found on server " << it->second % servers << " of " << servers << std::endl;
			return;
		}
		::usleep(100000);
	}
	std::cerr << "fdb5 parallax placement: WARNING dataset " << dataset_name << " (prefix " << pre << ") not found in "
		  << path << " after " << timeout_s << " s; using default hash placement" << std::endl;
}
#else
void parallax_placement_assign(const std::string &) {}
void parallax_placement_lookup(const std::string &) {}
#endif
} // namespace fdb5
