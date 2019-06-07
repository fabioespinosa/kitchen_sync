require File.expand_path(File.join(File.dirname(__FILE__), 'test_helper'))

class SpatialToTest < KitchenSync::EndpointTestCase
  include TestTableSchemas

  def from_or_to
    :to
  end

  def before
    program_env["ENDPOINT_ONLY_TABLES"] = "spatialtbl"
    cleanup_spatialtbl
    uninstall_spatial_support
    clear_schema
    install_spatial_support
    create_spatialtbl
  end

  def after
    spawner.stop_binary # done automatically by teardown, but we need to end the transaction before we can remove the extension
    cleanup_spatialtbl
    uninstall_spatial_support
  end

  def force_long_lat_option
    ", 'axis-order=long-lat'" if spatial_axis_order_depends_on_srs?
  end

  test_each "runs on empty spatial tables" do
    expect_handshake_commands
    expect_command Commands::SCHEMA
    send_command   Commands::SCHEMA, ["tables" => [spatialtbl_def]]
    expect_sync_start_commands
    expect_command Commands::RANGE, ["spatialtbl"]
    send_command   Commands::RANGE, ["spatialtbl", [], []]
    expect_quit_and_close

    assert_equal [],
                 query("SELECT id, ST_SRID(plainspat) AS plainspat_stid, ST_AsBinary(plainspat#{force_long_lat_option}) AS paingeom_wkb, ST_SRID(pointspat) AS pointspat_srid, ST_AsBinary(pointspat#{force_long_lat_option}) AS pointspat_wkb FROM spatialtbl ORDER BY id")
  end

  test_each "retrieves and saves row data without SRIDs in WKB format with a 4-byte zero SRID prefix" do
    @rows = [[1, ["00000000010100000000000000000024400000000000003440"].pack("H*"), ["00000000010100000000000000000034400000000000003E40"].pack("H*")],
             [2, ["00000000010700000002000000010200000002000000000000000000F83F000000000000024000000000000009400000000000401040010600000002000000010300000001000000040000000000000000003E40000000000000344000000000008046400000000000004440000000000000244000000000000044400000000000003E400000000000003440010300000001000000050000000000000000002E4000000000000014400000000000004440000000000000244000000000000024400000000000003440000000000000144000000000000024400000000000002E400000000000001440"].pack("H*"), nil]]

    expect_handshake_commands
    expect_command Commands::SCHEMA
    send_command   Commands::SCHEMA, ["tables" => [spatialtbl_def]]
    expect_sync_start_commands
    expect_command Commands::RANGE, ["spatialtbl"]
    send_command   Commands::RANGE, ["spatialtbl", [1], [2]]
    expect_command Commands::ROWS,
                   ["spatialtbl", [], [2]]
    send_results   Commands::ROWS,
                   ["spatialtbl", [], [2]],
                   *@rows
    expect_quit_and_close

    assert_equal [[1, 0, ["010100000000000000000024400000000000003440"].pack("H*"), 0, ["010100000000000000000034400000000000003E40"].pack("H*")],
                  [2, 0, ["010700000002000000010200000002000000000000000000F83F000000000000024000000000000009400000000000401040010600000002000000010300000001000000040000000000000000003E40000000000000344000000000008046400000000000004440000000000000244000000000000044400000000000003E400000000000003440010300000001000000050000000000000000002E4000000000000014400000000000004440000000000000244000000000000024400000000000003440000000000000144000000000000024400000000000002E400000000000001440"].pack("H*"), nil, nil]],
                 query("SELECT id, ST_SRID(plainspat) AS plainspat_stid, ST_AsBinary(plainspat#{force_long_lat_option}) AS paingeom_wkb, ST_SRID(pointspat) AS pointspat_srid, ST_AsBinary(pointspat#{force_long_lat_option}) AS pointspat_wkb FROM spatialtbl ORDER BY id")
  end

  test_each "retrieves and saves row data with SRIDs in WKB format with a 4-byte SRID prefix" do
    @rows = [[1, ["E6100000010100000000000000000024400000000000003440"].pack("H*"), ["E6100000010100000000000000000034400000000000003E40"].pack("H*")],
             [2, ["E6100000010700000002000000010200000002000000000000000000F83F000000000000024000000000000009400000000000401040010600000002000000010300000001000000040000000000000000003E40000000000000344000000000008046400000000000004440000000000000244000000000000044400000000000003E400000000000003440010300000001000000050000000000000000002E4000000000000014400000000000004440000000000000244000000000000024400000000000003440000000000000144000000000000024400000000000002E400000000000001440"].pack("H*"), nil]]

    expect_handshake_commands
    expect_command Commands::SCHEMA
    send_command   Commands::SCHEMA, ["tables" => [spatialtbl_def]]
    expect_sync_start_commands
    expect_command Commands::RANGE, ["spatialtbl"]
    send_command   Commands::RANGE, ["spatialtbl", [1], [2]]
    expect_command Commands::ROWS,
                   ["spatialtbl", [], [2]]
    send_results   Commands::ROWS,
                   ["spatialtbl", [], [2]],
                   *@rows
    expect_quit_and_close

    assert_equal [[1, 4326, ["010100000000000000000024400000000000003440"].pack("H*"), 4326, ["010100000000000000000034400000000000003E40"].pack("H*")],
                  [2, 4326, ["010700000002000000010200000002000000000000000000F83F000000000000024000000000000009400000000000401040010600000002000000010300000001000000040000000000000000003E40000000000000344000000000008046400000000000004440000000000000244000000000000044400000000000003E400000000000003440010300000001000000050000000000000000002E4000000000000014400000000000004440000000000000244000000000000024400000000000003440000000000000144000000000000024400000000000002E400000000000001440"].pack("H*"), nil, nil]],
                 query("SELECT id, ST_SRID(plainspat) AS plainspat_stid, ST_AsBinary(plainspat#{force_long_lat_option}) AS paingeom_wkb, ST_SRID(pointspat) AS pointspat_srid, ST_AsBinary(pointspat#{force_long_lat_option}) AS pointspat_wkb FROM spatialtbl ORDER BY id")
  end
end
