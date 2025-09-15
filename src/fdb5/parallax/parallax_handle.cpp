#include "parallax_handle.h"

std::unordered_map<std::string, par_handle> par_handles;

int current_index = 0;

par_handle par_get_db(const std::string &db_name)
{
	// Check if the database is already opened
	auto it = par_handles.find(db_name);
	if (it != par_handles.end()) {
		return it->second; // Return the existing handle
	}

	// Database is not opened yet, proceed to open it

	std::string volume_path = "/tmp/root/parallax/default" + std::to_string(current_index);
	const char *volume_name = volume_path.c_str();

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

void par_init_db_handles()
{
	int num_of_servers = par_get_num_of_servers();
	for (int i = 0; i < PARALLAX_DB_COUNT; ++i) {
		current_index = i;
		for (int j = 0; j < num_of_servers; j++) {
			std::string db_name = PARALLAX_GLOBAL_DB + std::to_string(i * num_of_servers + j);
			par_get_db(db_name);
		}
	}
}
