#ifndef RESET_TABLE_SEQUENCES_H
#define RESET_TABLE_SEQUENCES_H

template <typename DatabaseClient, bool = is_base_of<SequenceColumns, DatabaseClient>::value>
struct ResetTableSequences {
	static void execute(DatabaseClient &client, const Table &table) {
		/* nothing required */
	}
};

template <typename DatabaseClient>
struct ResetTableSequences <DatabaseClient, true> {
	static void execute(DatabaseClient &client, const Table &table) {
		for (const Column &column : table.columns) {
			if (column.default_type == DefaultType::generated_by_sequence) {
				string statement("SELECT setval(pg_get_serial_sequence('");
				statement += client.escape_string_value(client.quote_identifier(table.name));
				statement += "', '";
				statement += client.escape_string_value(column.name);
				statement += "'), COALESCE(MAX(";
				statement += client.quote_identifier(column.name);
				statement += "), 0) + 1, false) FROM ";
				statement += client.quote_identifier(table.name);
				client.execute(statement);
			}
		}
	}
};

#endif
