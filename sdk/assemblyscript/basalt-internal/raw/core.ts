// Raw imports of basalt_abi.h that both profiles may use.
// @ts-ignore: decorator
@external("basalt", "log") export declare function log(level: i32, msg: usize, len: i32): void;
// @ts-ignore: decorator
@external("basalt", "config_i64") export declare function config_i64(key: usize, klen: i32, def: i64): i64;
// @ts-ignore: decorator
@external("basalt", "config_f64") export declare function config_f64(key: usize, klen: i32, def: f64): f64;
// @ts-ignore: decorator
@external("basalt", "config_bool") export declare function config_bool(key: usize, klen: i32, def: i32): i32;
// @ts-ignore: decorator
@external("basalt", "config_str") export declare function config_str(key: usize, klen: i32, out: usize, cap: i32): i32;
// @ts-ignore: decorator
@external("basalt", "translate") export declare function translate(key: usize, klen: i32, args: usize, alen: i32, out: usize, cap: i32): i32;
// @ts-ignore: decorator
@external("basalt", "player_by_name") export declare function player_by_name(name: usize, len: i32): i32;
// @ts-ignore: decorator
@external("basalt", "players_online") export declare function players_online(out: usize, cap: i32): i32;
// @ts-ignore: decorator
@external("basalt", "player_name") export declare function player_name(player: i32, out: usize, cap: i32): i32;
// @ts-ignore: decorator
@external("basalt", "player_send") export declare function player_send(player: i32, text: usize, tlen: i32, args: usize, alen: i32): i32;
// @ts-ignore: decorator
@external("basalt", "player_action_bar") export declare function player_action_bar(player: i32, text: usize, tlen: i32, args: usize, alen: i32): i32;
// @ts-ignore: decorator
@external("basalt", "data_get_i64") export declare function data_get_i64(player: i32, key: usize, klen: i32, def: i64): i64;
// @ts-ignore: decorator
@external("basalt", "data_set_i64") export declare function data_set_i64(player: i32, key: usize, klen: i32, value: i64): i32;
// @ts-ignore: decorator
@external("basalt", "data_get_bool") export declare function data_get_bool(player: i32, key: usize, klen: i32, def: i32): i32;
// @ts-ignore: decorator
@external("basalt", "data_set_bool") export declare function data_set_bool(player: i32, key: usize, klen: i32, value: i32): i32;
// @ts-ignore: decorator
@external("basalt", "data_get_str") export declare function data_get_str(player: i32, key: usize, klen: i32, out: usize, cap: i32): i32;
// @ts-ignore: decorator
@external("basalt", "data_set_str") export declare function data_set_str(player: i32, key: usize, klen: i32, value: usize, vlen: i32): i32;
// @ts-ignore: decorator
@external("basalt", "data_remove") export declare function data_remove(player: i32, key: usize, klen: i32): i32;
// @ts-ignore: decorator
@external("basalt", "event_on") export declare function event_on(name: usize, nlen: i32, priority: i32, token: i32): i32;
// @ts-ignore: decorator
@external("basalt", "event_player") export declare function event_player(ctx: i32): i32;
// @ts-ignore: decorator
@external("basalt", "event_field_str") export declare function event_field_str(ctx: i32, field: usize, flen: i32, out: usize, cap: i32): i32;
// @ts-ignore: decorator
@external("basalt", "event_field_i64") export declare function event_field_i64(ctx: i32, field: usize, flen: i32, out: usize): i32;
// @ts-ignore: decorator
@external("basalt", "event_cancel") export declare function event_cancel(ctx: i32, cancelled: i32): i32;
// @ts-ignore: decorator
@external("basalt", "command_register") export declare function command_register(spec: usize, slen: i32, permission: usize, plen: i32, token: i32): i32;
// @ts-ignore: decorator
@external("basalt", "command_source_name") export declare function command_source_name(ctx: i32, out: usize, cap: i32): i32;
// @ts-ignore: decorator
@external("basalt", "command_is_console") export declare function command_is_console(ctx: i32): i32;
// @ts-ignore: decorator
@external("basalt", "command_reply") export declare function command_reply(ctx: i32, text: usize, tlen: i32, args: usize, alen: i32): i32;
// @ts-ignore: decorator
@external("basalt", "command_arg_str") export declare function command_arg_str(ctx: i32, name: usize, nlen: i32, out: usize, cap: i32): i32;
// @ts-ignore: decorator
@external("basalt", "command_arg_i64") export declare function command_arg_i64(ctx: i32, name: usize, nlen: i32, out: usize): i32;
// @ts-ignore: decorator
@external("basalt", "task_after") export declare function task_after(delay: i32, token: i32): i32;
// @ts-ignore: decorator
@external("basalt", "task_every") export declare function task_every(delay: i32, period: i32, token: i32): i32;
// @ts-ignore: decorator
@external("basalt", "task_cancel") export declare function task_cancel(task: i32): i32;
