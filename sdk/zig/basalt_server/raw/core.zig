//! The core imports of basalt_abi.h that only the server profile offers.
pub extern "basalt" fn player_particle(player: i32, id: [*]const u8, idlen: i32, count: i32, spread: f64) i32;
pub extern "basalt" fn player_sound(player: i32, id: [*]const u8, idlen: i32, volume: f32, pitch: f32) i32;
pub extern "basalt" fn store_top(key: [*]const u8, klen: i32, limit: i32, out: [*]u8, cap: i32) i32;
pub extern "basalt" fn placeholder_register(name: [*]const u8, nlen: i32, token: i32) i32;
pub extern "basalt" fn placeholder_player(ctx: i32) i32;
pub extern "basalt" fn placeholder_result(ctx: i32, text: [*]const u8, len: i32) i32;
pub extern "basalt" fn menu_create(title: [*]const u8, tlen: i32, rows: i32) i32;
pub extern "basalt" fn menu_set(menu: i32, slot: i32, item: [*]const u8, ilen: i32, name: [*]const u8, nlen: i32, lore: [*]const u8, llen: i32, token: i32) i32;
pub extern "basalt" fn menu_open(menu: i32, player: i32) i32;
pub extern "basalt" fn menu_click_player(ctx: i32) i32;
