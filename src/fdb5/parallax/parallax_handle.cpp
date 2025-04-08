#include "parallax_handle.h"

std::unordered_map<std::string, par_handle> par_handles;

par_handle par_get_db(const std::string &db_name)
{
	// std::cout << "File: " << __FILE__ << ", Line: " << __LINE__ << ", Function: " << __func__ << std::endl;

	// Check if the database is already opened
	auto it = par_handles.find(db_name);
	if (it != par_handles.end()) {
		return it->second; // Return the existing handle
	}

	// Database is not opened yet, proceed to open it
	const char *volume_name = getenv(PARALLAX_VOLUME_ENV_VAR);

	par_db_options db_options = { .volume_name = (char *)volume_name,
				      .db_name = db_name.c_str(),
				      .create_flag = PAR_CREATE_DB,
				      .options = par_get_default_options() };
	db_options.options[LEVEL0_SIZE].value = PARALLAX_L0_SIZE;
	db_options.options[GROWTH_FACTOR].value = PARALLAX_GROWTH_FACTOR;
	db_options.options[PRIMARY_MODE].value = 1;
	db_options.options[ENABLE_BLOOM_FILTERS].value = 1;

	const char *error_message = NULL;
	par_handle handle = par_open(&db_options, &error_message);

	if (error_message) {
		LSM_DEBUG("Parallax says: %s", error_message);
	}

	if (handle == NULL && error_message) {
		LSM_FATAL("Error upon opening the DB, error %s", error_message);
	}

	// Insert the new handle into the hash table
	par_handles[db_name] = handle;

	return handle;
}
