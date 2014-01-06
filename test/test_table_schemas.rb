module TestTableSchemas
  def create_footbl
    execute(<<-SQL)
      CREATE TABLE footbl (
        col1 INT NOT NULL,
        another_col INT,
        col3 VARCHAR(10),
        PRIMARY KEY(col1))
SQL
  end

  def footbl_def
    { "name"    => "footbl",
      "columns" => [
        {"name" => "col1"},
        {"name" => "another_col"},
        {"name" => "col3"}],
      "primary_key_columns" => [0],
      "keys" => [] }
  end

  def create_secondtbl
    execute(<<-SQL)
      CREATE TABLE secondtbl (
        pri1 INT NOT NULL,
        pri2 CHAR(2) NOT NULL,
        sec INT,
        tri INT,
        PRIMARY KEY(pri2, pri1))
SQL
    execute(<<-SQL)
      CREATE INDEX secidx ON secondtbl (sec)
SQL
  end

  def secondtbl_def
    { "name"    => "secondtbl",
      "columns" => [
        {"name" => "pri1"},
        {"name" => "pri2"},
        {"name" => "sec"},
        {"name" => "tri"}],
      "primary_key_columns" => [1, 0], # note order is that listed in the key, not the index of the column in the table
      "keys" => [
        {"name" => "secidx", "unique" => false, "columns" => [2]}] }
  end

  def create_middletbl
    execute(<<-SQL)
      CREATE TABLE middletbl (
        pri INT NOT NULL,
        PRIMARY KEY(pri))
SQL
  end

  def middletbl_def
    { "name"    => "middletbl",
      "columns" => [
        {"name" => "pri"}],
      "primary_key_columns" => [0],
      "keys" => [] }
  end

  def create_texttbl
    execute(<<-SQL)
      CREATE TABLE texttbl (
        pri INT NOT NULL,
        textfield TEXT#{'(268435456)' if @database_server == 'mysql'},
        PRIMARY KEY(pri))
SQL
  end

  def texttbl_def
    { "name"    => "texttbl",
      "columns" => [
        {"name" => "pri"},
        {"name" => "textfield"}],
      "primary_key_columns" => [0],
      "keys" => [] }
  end

  def create_noprimarytbl(create_suitable_keys = true)
    execute(<<-SQL)
      CREATE TABLE noprimarytbl (
        nullable INT,
        version VARCHAR(255) NOT NULL,
        name VARCHAR(255),
        non_nullable INT NOT NULL)
SQL
    execute "CREATE INDEX everything_key ON noprimarytbl (name, nullable, version)"
    execute "CREATE UNIQUE INDEX ignored_key ON noprimarytbl (nullable, version)"
    execute "CREATE UNIQUE INDEX correct_key ON noprimarytbl (version)" if create_suitable_keys
    execute "CREATE UNIQUE INDEX version_and_name_key ON noprimarytbl (version, name)"
    execute "CREATE UNIQUE INDEX non_nullable_key ON noprimarytbl (non_nullable)" if create_suitable_keys
    execute "CREATE INDEX not_unique_key ON noprimarytbl (non_nullable)"
  end

  def noprimarytbl_def
    { "name" => "noprimarytbl",
      "columns" => [
        {"name" => "nullable"},
        {"name" => "version"},
        {"name" => "name"},
        {"name" => "non_nullable"}],
      "primary_key_columns" => [1],
      "keys" => [ # sorted in uniqueness then alphabetic name order, but otherwise a transcription of the above create index statements
        {"name" => "correct_key",          "unique" => true,  "columns" => [1]},
        {"name" => "ignored_key",          "unique" => true,  "columns" => [0, 1]},
        {"name" => "non_nullable_key",     "unique" => true,  "columns" => [3]},
        {"name" => "version_and_name_key", "unique" => true,  "columns" => [1, 2]},
        {"name" => "everything_key",       "unique" => false, "columns" => [2, 0, 1]},
        {"name" => "not_unique_key",       "unique" => false, "columns" => [3]} ] }
  end

  def create_some_tables
    clear_schema
    create_footbl
    create_secondtbl
  end

  def some_table_defs
    [footbl_def, secondtbl_def]
  end
end
