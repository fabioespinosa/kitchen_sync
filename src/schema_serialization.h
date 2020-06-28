#ifndef SCHEMA_SERIALIZATION_H
#define SCHEMA_SERIALIZATION_H

#include "schema.h"
#include "message_pack/unpack.h"
#include "protocol_versions.h"
#include "legacy_schema_serialization.h"

template <typename OutputStream>
void operator << (Packer<OutputStream> &packer, const Column &column) {
	if (packer.stream().protocol_version <= LAST_LEGACY_SCHEMA_FORMAT_VERSION) {
		legacy_serialize(packer, column);
		return;
	}

	int fields = 2;
	if (column.size) fields++;
	if (column.scale) fields++;
	if (!column.nullable) fields++;
	if (!column.subtype.empty()) fields++;
	if (!column.reference_system.empty()) fields++;
	if (!column.enumeration_values.empty()) fields++;
	if (column.default_type != DefaultType::no_default) fields++;
	if (column.auto_update_type != AutoUpdateType::no_auto_update) fields++;
	pack_map_length(packer, fields);
	packer << string("name");
	packer << column.name;
	packer << string("column_type");
	packer << ColumnTypeNames.at(column.column_type);
	if (column.size) {
		packer << string("size");
		packer << column.size;
	}
	if (column.scale) {
		packer << string("scale");
		packer << column.scale;
	}
	if (!column.nullable) {
		packer << string("nullable");
		packer << column.nullable;
	}
	if (!column.subtype.empty()) {
		packer << string("subtype");
		packer << column.subtype;
	}
	if (!column.reference_system.empty()) {
		packer << string("reference_system");
		packer << column.reference_system;
	}
	if (!column.enumeration_values.empty()) {
		packer << string("enumeration_values");
		packer << column.enumeration_values;
	}
	switch (column.default_type) {
		case DefaultType::no_default:
			break;

		case DefaultType::default_value:
			packer << string("default_value");
			packer << column.default_value;
			break;

		case DefaultType::generated_by_sequence:
			packer << string("generated_by_sequence");
			packer << column.default_value;
			break;

		case DefaultType::generated_by_default_as_identity:
			packer << string("generated_by_default_as_identity");
			packer << column.default_value; // currently unused, but allowed for forward compatibility
			break;

		case DefaultType::generated_always_as_identity:
			packer << string("generated_always_as_identity");
			packer << column.default_value; // currently unused, but allowed for forward compatibility
			break;

		case DefaultType::default_expression:
			packer << string("default_expression");
			packer << column.default_value;
			break;

		case DefaultType::generated_always_virtual:
			packer << string("generated_always_virtual");
			packer << column.default_value;
			break;

		case DefaultType::generated_always_stored:
			packer << string("generated_always_stored");
			packer << column.default_value;
			break;
	}
	switch (column.auto_update_type) {
		case AutoUpdateType::no_auto_update:
			break;

		case AutoUpdateType::current_timestamp:
			packer << string("auto_update");
			packer << string("current_timestamp");
			break;
	}
}

template <typename VersionedFDWriteStream>
void operator << (Packer<VersionedFDWriteStream> &packer, const Key &key) {
	if (packer.stream().protocol_version <= LAST_LEGACY_SCHEMA_FORMAT_VERSION) {
		legacy_serialize(packer, key);
		return;
	}

	pack_map_length(packer, key.standard() ? 2 : 3);
	packer << string("name");
	packer << key.name;
	switch (key.key_type) {
		case KeyType::standard_key:
			break;

		case KeyType::unique_key:
			packer << string("key_type");
			packer << string("unique");
			break;

		case KeyType::spatial_key:
			packer << string("key_type");
			packer << string("spatial");
			break;
	}
	packer << string("columns");
	packer << key.columns;
}

template <typename OutputStream>
void operator << (Packer<OutputStream> &packer, const Table &table) {
	if (packer.stream().protocol_version <= LAST_LEGACY_SCHEMA_FORMAT_VERSION) {
		legacy_serialize(packer, table);
		return;
	}
	if (table.schema_name.empty()) {
		pack_map_length(packer, 5);
	} else {
		pack_map_length(packer, 6);
		packer << string("schema_name");
		packer << table.schema_name;
	}
	packer << string("name");
	packer << table.name;
	packer << string("columns");
	packer << table.columns;
	packer << string("primary_key_columns");
	packer << table.primary_key_columns;
	packer << string("primary_key_type");
	switch (table.primary_key_type) {
		case PrimaryKeyType::explicit_primary_key:
			packer << string("explicit_primary_key");
			break;

		case PrimaryKeyType::suitable_unique_key:
			packer << string("suitable_unique_key");
			break;

		case PrimaryKeyType::no_available_key:
			packer << string("no_available_key");
			break;

		case PrimaryKeyType::entire_row_as_key:
			packer << string("entire_row_as_key");
			break;
	}
	packer << string("keys");
	packer << table.keys;
}

template <typename OutputStream>
void operator << (Packer<OutputStream> &packer, const Database &database) {
	pack_map_length(packer, database.errors.empty() ? 1 : 2);
	packer << string("tables");
	packer << database.tables;
	if (!database.errors.empty()) {
		packer << string("errors");
		packer << database.errors;
	}
}

template <typename InputStream>
void operator >> (Unpacker<InputStream> &unpacker, Column &column) {
	if (unpacker.stream().protocol_version <= LAST_LEGACY_SCHEMA_FORMAT_VERSION) {
		legacy_deserialize(unpacker, column);
		return;
	}

	size_t map_length = unpacker.next_map_length(); // checks type

	while (map_length--) {
		string attr_key = unpacker.template next<string>();

		if (attr_key == "name") {
			unpacker >> column.name;
		} else if (attr_key == "column_type") {
			column.column_type = ColumnTypesByName.at(unpacker.template next<string>()); // non-present entries shouldn't get hit since we negotiate supported types
		} else if (attr_key == "size") {
			unpacker >> column.size;
		} else if (attr_key == "scale") {
			unpacker >> column.scale;
		} else if (attr_key == "nullable") {
			unpacker >> column.nullable;
		} else if (attr_key == "subtype") {
			unpacker >> column.subtype;
		} else if (attr_key == "reference_system") {
			unpacker >> column.reference_system;
		} else if (attr_key == "enumeration_values") {
			unpacker >> column.enumeration_values;
		} else if (attr_key == "default_value") {
			column.default_type = DefaultType::default_value;
			unpacker >> column.default_value;
		} else if (attr_key == "default_function") { // legacy name for protocol version 7 and earlier
			column.default_type = DefaultType::default_expression;
			unpacker >> column.default_value;
		} else if (attr_key == "default_expression") {
			column.default_type = DefaultType::default_expression;
			unpacker >> column.default_value;
		} else if (attr_key == "generated_by_sequence") {
			column.default_type = DefaultType::generated_by_sequence;
			unpacker >> column.default_value;
		} else if (attr_key == "generated_by_default_as_identity") {
			column.default_type = DefaultType::generated_by_default_as_identity;
			unpacker >> column.default_value; // currently unused, but allowed for forward compatibility
		} else if (attr_key == "generated_always_as_identity") {
			column.default_type = DefaultType::generated_always_as_identity;
			unpacker >> column.default_value; // currently unused, but allowed for forward compatibility
		} else if (attr_key == "generated_always_virtual") {
			column.default_type = DefaultType::generated_always_virtual;
			unpacker >> column.default_value;
		} else if (attr_key == "generated_always_stored") {
			column.default_type = DefaultType::generated_always_stored;
			unpacker >> column.default_value;
		} else if (attr_key == "auto_update") {
			string value;
			unpacker >> value;
			column.auto_update_type = (value == "current_timestamp" ? AutoUpdateType::current_timestamp : AutoUpdateType::no_auto_update);
		} else {
			// ignore anything else, for forward compatibility
			unpacker.skip();
		}
	}
}

template <typename InputStream>
void operator >> (Unpacker<InputStream> &unpacker, Key &key) {
	size_t map_length = unpacker.next_map_length(); // checks type

	while (map_length--) {
		string attr_key = unpacker.template next<string>();

		if (attr_key == "name") {
			unpacker >> key.name;
		} else if (attr_key == "unique") {
			key.key_type = (unpacker.template next<bool>() ? KeyType::unique_key : KeyType::standard_key);
		} else if (attr_key == "key_type") {
			string key_type(unpacker.template next<string>());
			if (key_type == "standard") {
				key.key_type = KeyType::standard_key;
			} else if (key_type == "unique") {
				key.key_type = KeyType::unique_key;
			} else if (key_type == "spatial") {
				key.key_type = KeyType::spatial_key;
			}
		} else if (attr_key == "columns") {
			unpacker >> key.columns;
		} else {
			// ignore anything else, for forward compatibility
			unpacker.skip();
		}
	}
}

template <typename InputStream>
void operator >> (Unpacker<InputStream> &unpacker, Table &table) {
	if (unpacker.stream().protocol_version <= LAST_LEGACY_SCHEMA_FORMAT_VERSION) {
		legacy_deserialize(unpacker, table);
		return;
	}

	size_t map_length = unpacker.next_map_length(); // checks type

	while (map_length--) {
		string attr_key = unpacker.template next<string>();

		if (attr_key == "schema_name") {
			unpacker >> table.schema_name;
		} else if (attr_key == "name") {
			unpacker >> table.name;
		} else if (attr_key == "columns") {
			unpacker >> table.columns;
		} else if (attr_key == "primary_key_columns") {
			unpacker >> table.primary_key_columns;
		} else if (attr_key == "primary_key_type") {
			string primary_key_type(unpacker.template next<string>());
			if (primary_key_type == "no_available_key") {
				table.primary_key_type = PrimaryKeyType::no_available_key;
			} else if (primary_key_type == "explicit_primary_key") {
				table.primary_key_type = PrimaryKeyType::explicit_primary_key;
			} else if (primary_key_type == "suitable_unique_key") {
				table.primary_key_type = PrimaryKeyType::suitable_unique_key;
			} else if (primary_key_type == "entire_row_as_key") {
				table.primary_key_type = PrimaryKeyType::entire_row_as_key;
			}
		} else if (attr_key == "keys") {
			unpacker >> table.keys;
		} else {
			// ignore anything else, for forward compatibility
			unpacker.skip();
		}
	}
}

template <typename InputStream>
void operator >> (Unpacker<InputStream> &unpacker, Database &database) {
	size_t map_length = unpacker.next_map_length(); // checks type

	while (map_length--) {
		string attr_key = unpacker.template next<string>();

		if (attr_key == "tables") {
			unpacker >> database.tables;
		} else if (attr_key == "errors") {
			unpacker >> database.errors;
		} else {
			// ignore anything else, for forward compatibility
			unpacker.skip();
		}
	}
}

#endif
