// Raw imports of basalt_proxy_abi.h.
// @ts-ignore: decorator
@external("basalt_proxy", "event_set_str") export declare function event_set_str(ctx: i32, field: usize, flen: i32, value: usize, vlen: i32): i32;
// @ts-ignore: decorator
@external("basalt_proxy", "event_set_i64") export declare function event_set_i64(ctx: i32, field: usize, flen: i32, value: i64): i32;
// @ts-ignore: decorator
@external("basalt_proxy", "player_uuid") export declare function player_uuid(player: i32, out: usize, cap: i32): i32;
// @ts-ignore: decorator
@external("basalt_proxy", "player_server") export declare function player_server(player: i32, out: usize, cap: i32): i32;
// @ts-ignore: decorator
@external("basalt_proxy", "player_ping") export declare function player_ping(player: i32): i32;
// @ts-ignore: decorator
@external("basalt_proxy", "player_setting_str") export declare function player_setting_str(player: i32, field: usize, flen: i32, out: usize, cap: i32): i32;
// @ts-ignore: decorator
@external("basalt_proxy", "player_setting_i64") export declare function player_setting_i64(player: i32, field: usize, flen: i32, out: usize): i32;
// @ts-ignore: decorator
@external("basalt_proxy", "player_has_permission") export declare function player_has_permission(player: i32, node: usize, nlen: i32): i32;
// @ts-ignore: decorator
@external("basalt_proxy", "player_title") export declare function player_title(player: i32, title: usize, tlen: i32, subtitle: usize, slen: i32, fade_in: i32, stay: i32, fade_out: i32): i32;
// @ts-ignore: decorator
@external("basalt_proxy", "player_connect") export declare function player_connect(player: i32, server: usize, slen: i32): i32;
// @ts-ignore: decorator
@external("basalt_proxy", "player_kick") export declare function player_kick(player: i32, reason: usize, rlen: i32, args: usize, alen: i32): i32;
// @ts-ignore: decorator
@external("basalt_proxy", "servers_list") export declare function servers_list(out: usize, cap: i32): i32;
// @ts-ignore: decorator
@external("basalt_proxy", "server_address") export declare function server_address(name: usize, nlen: i32, out: usize, cap: i32): i32;
// @ts-ignore: decorator
@external("basalt_proxy", "players_on") export declare function players_on(server: usize, slen: i32, out: usize, cap: i32): i32;
