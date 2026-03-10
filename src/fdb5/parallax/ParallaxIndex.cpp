/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "fdb5/parallax/ParallaxIndex.h"

namespace fdb5
{
ParallaxIndex::ParallaxIndex(const fdb5::Key &key)
	: IndexBase(key, "parallaxKeyValue")
	, location_("parallaxKeyValue")
{
	key_ = key;
}

ParallaxIndex::ParallaxIndex(const Key &key, bool readAxes)
	: IndexBase(key, "parallaxKeyValue")
	, location_("parallaxKeyValue")
{
	if (readAxes) {
		updateAxes();
	}
}

void ParallaxIndex::updateAxes()
{
	par_handle db_handle;
	size_t hash;
	int db_index;

	std::string db_name;

	std::string keyStr = "axes";
	struct par_key axes_key {
		.size = static_cast<uint32_t>(keyStr.size()), .data = keyStr.c_str()
	};

	std::vector<char> axes_data(512);
	struct par_value axes_value = { .val_buffer_size = static_cast<uint32_t>(axes_data.size()),
					.val_size = 0,
					.val_buffer = axes_data.data() };

	const char *error_msg = nullptr;
	hash = std::hash<std::string>{}(keyStr);
	db_index = hash % PARALLAX_DB_COUNT;

	db_name = "par_db" + std::to_string(db_index);
	db_handle = par_get_db(db_name);
	par_get(db_handle, &axes_key, &axes_value, &error_msg);

	std::vector<std::string> axis_names;
	if (axes_value.val_size > 0 && !error_msg) {
		std::string axes_str(axes_data.begin(), axes_data.begin() + axes_value.val_size);
		eckit::Tokenizer parse(",");
		parse(axes_str, axis_names);
	}

	std::string indexKeyStr = key_.valuesToString();

	std::vector<char> axis_values_buf(32 * 1024);

	for (const auto &name : axis_names) {
		std::string axisKeyStr = name;
		struct par_key axis_key = { .size = static_cast<uint32_t>(axisKeyStr.size()),
					    .data = axisKeyStr.c_str() };

		struct par_value axis_value = { .val_buffer_size = static_cast<uint32_t>(axis_values_buf.size()),
						.val_size = 0,
						.val_buffer = axis_values_buf.data() };

		hash = std::hash<std::string>{}(axisKeyStr);
		db_index = hash % PARALLAX_DB_COUNT;

		db_name = "par_db" + std::to_string(db_index);
		db_handle = par_get_db(db_name);

		par_get(db_handle, &axis_key, &axis_value, &error_msg);

		std::vector<std::string> values;
		if (axis_value.val_size > 0 && !error_msg) {
			std::string val_str(axis_values_buf.begin(), axis_values_buf.begin() + axis_value.val_size);
			eckit::Tokenizer parse(",");
			parse(val_str, values);
		}

		axes_.insert(name, values);
	}
	axes_.sort();
}

bool ParallaxIndex::get(const Key &key, const Key &remapKey, Field &field) const
{
	std::string query = key.valuesToString();

	int field_loc_max_len = 512;
	std::vector<char> loc_data(field_loc_max_len);

	struct par_key pkey;
	pkey.size = query.size() + 1;
	pkey.data = query.c_str();

	struct par_value value;
	value.val_buffer = loc_data.data();
	value.val_buffer_size = field_loc_max_len;
	value.val_size = 0;

	size_t hash = std::hash<std::string>{}(query);
	int db_index = hash % PARALLAX_DB_COUNT;

	std::string db_name = "par_db" + std::to_string(db_index);
	par_handle db_handle = par_get_db(db_name);
	const char *error_msg = nullptr;

	if (!db_handle) {
		throw eckit::Exception("Failed to open Parallax index database");
	}
	par_get(db_handle, &pkey, &value, &error_msg);
	if (error_msg != nullptr) {
		return false;
	}
	if (value.val_size == 0) {
		return false;
	}

	eckit::MemoryStream ms(loc_data.data(), (size_t)value.val_size);
	time_t ts;
	ms >> ts;
	auto *loc = eckit::Reanimator<fdb5::FieldLocation>::reanimate(ms);
	field = fdb5::Field(std::move(*loc), ts, fdb5::FieldDetails());

	return true;
}

void ParallaxIndex::add(const Key &key, const Field &field)
{
	eckit::MemoryHandle h{ (size_t)PATH_MAX };
	eckit::HandleStream hs{ h };
	h.openForWrite(eckit::Length(0));
	{
		eckit::AutoClose closer(h);
		takeTimestamp();
		hs << timestamp();
		hs << field.location();
	}

	int field_loc_max_len = 512;
	if (hs.bytesWritten() > field_loc_max_len) {
		throw eckit::Exception("Serialized field location exceeded maximum allowed length.");
	}

	std::string keyStr = key.valuesToString();
	struct par_value valueData;
	valueData.val_size = hs.bytesWritten();
	valueData.val_buffer_size = h.size();
	valueData.val_buffer = reinterpret_cast<char *>(const_cast<void *>(h.data()));
	if (!valueData.val_buffer) {
		throw eckit::Exception("Memory allocation failed for Parallax index storage");
	}
	struct par_key_value kv;
	kv.k.size = keyStr.size() + 1;
	kv.k.data = keyStr.c_str();
	kv.v = valueData;

	size_t hash = std::hash<std::string>{}(keyStr);
	int db_index = hash % PARALLAX_DB_COUNT;

	std::string db_name = "par_db" + std::to_string(db_index);
	par_handle db_handle = par_get_db(db_name);
	const char *error_msg = nullptr;

	if (!db_handle) {
		throw eckit::Exception("Failed to get Parallax database handle.");
	}

	par_put(db_handle, &kv, &error_msg);

	if (error_msg != nullptr) {
		throw eckit::Exception(std::string("Parallax index insertion failed: ") + error_msg);
	}
}

}
