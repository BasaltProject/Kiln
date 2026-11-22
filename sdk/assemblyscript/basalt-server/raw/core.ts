// Raw imports of basalt_abi.h that only server plugins may use.
// @ts-ignore: decorator
@external("basalt", "player_particle") export declare function player_particle(player: i32, id: usize, idlen: i32, count: i32, spread: f64): i32;
// @ts-ignore: decorator
@external("basalt", "player_sound") export declare function player_sound(player: i32, id: usize, idlen: i32, volume: f32, pitch: f32): i32;
// @ts-ignore: decorator
@external("basalt", "store_top") export declare function store_top(key: usize, klen: i32, limit: i32, out: usize, cap: i32): i32;
// @ts-ignore: decorator
@external("basalt", "placeholder_register") export declare function placeholder_register(name: usize, nlen: i32, token: i32): i32;
// @ts-ignore: decorator
@external("basalt", "placeholder_player") export declare function placeholder_player(ctx: i32): i32;
// @ts-ignore: decorator
@external("basalt", "placeholder_result") export declare function placeholder_result(ctx: i32, text: usize, len: i32): i32;
// @ts-ignore: decorator
@external("basalt", "menu_create") export declare function menu_create(title: usize, tlen: i32, rows: i32): i32;
// @ts-ignore: decorator
@external("basalt", "menu_set") export declare function menu_set(menu: i32, slot: i32, item: usize, ilen: i32, name: usize, nlen: i32, lore: usize, llen: i32, token: i32): i32;
// @ts-ignore: decorator
@external("basalt", "menu_open") export declare function menu_open(menu: i32, player: i32): i32;
// @ts-ignore: decorator
@external("basalt", "menu_click_player") export declare function menu_click_player(ctx: i32): i32;
