//! The core imports of basalt_proxy_abi.h (module "basalt_proxy").
pub extern "basalt_proxy" fn event_set_str(ctx: i32, field: [*]const u8, flen: i32, value: [*]const u8, vlen: i32) i32;
pub extern "basalt_proxy" fn event_set_i64(ctx: i32, field: [*]const u8, flen: i32, value: i64) i32;
pub extern "basalt_proxy" fn player_uuid(player: i32, out: [*]u8, cap: i32) i32;
pub extern "basalt_proxy" fn player_server(player: i32, out: [*]u8, cap: i32) i32;
pub extern "basalt_proxy" fn player_ping(player: i32) i32;
pub extern "basalt_proxy" fn player_setting_str(player: i32, field: [*]const u8, flen: i32, out: [*]u8, cap: i32) i32;
pub extern "basalt_proxy" fn player_setting_i64(player: i32, field: [*]const u8, flen: i32, out: *i64) i32;
pub extern "basalt_proxy" fn player_has_permission(player: i32, node: [*]const u8, nlen: i32) i32;
pub extern "basalt_proxy" fn player_title(player: i32, title: [*]const u8, tlen: i32, subtitle: [*]const u8, slen: i32, fade_in: i32, stay: i32, fade_out: i32) i32;
pub extern "basalt_proxy" fn player_connect(player: i32, server: [*]const u8, slen: i32) i32;
pub extern "basalt_proxy" fn player_kick(player: i32, reason: [*]const u8, rlen: i32, args: ?[*]const u8, alen: i32) i32;
pub extern "basalt_proxy" fn servers_list(out: [*]u8, cap: i32) i32;
pub extern "basalt_proxy" fn server_address(name: [*]const u8, nlen: i32, out: [*]u8, cap: i32) i32;
pub extern "basalt_proxy" fn players_on(server: [*]const u8, slen: i32, out: [*]i32, cap: i32) i32;
