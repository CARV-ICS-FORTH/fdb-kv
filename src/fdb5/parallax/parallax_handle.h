#include "parallax.h"
#include <unistd.h>
#include <unordered_map>

#include "eckit/config/Resource.h"

#define PARALLAX_L0_SIZE (16 * 1024 * 1024UL);
#define PARALLAX_GROWTH_FACTOR 8
#define PARALLAX_VOLUME_ENV_VAR "PARH5_VOLUME"
#define PARALLAX_MAX_KEY_SIZE 256
#define PARALLAX_GLOBAL_DB "par_db"

using eckit::PathName;

#define LSM_DEBUG(...)                                                               \
	do {                                                                         \
		char buffer[1024];                                                   \
		snprintf(buffer, sizeof(buffer), __VA_ARGS__);                       \
		::std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << " " \
			    << " DEBUG: " << buffer << ::std::endl;                  \
	} while (0);

#define LSM_FATAL(...)                                                                                                \
	do {                                                                                                          \
		char buffer[1024];                                                                                    \
		snprintf(buffer, sizeof(buffer), __VA_ARGS__);                                                        \
		::std::cout << __FILE__ << ":" << __func__ << ":" << __LINE__ << " FATAL: " << buffer << ::std::endl; \
		_exit(EXIT_FAILURE);                                                                                  \
	} while (0);


par_handle par_get_db(const std::string &db_name);
