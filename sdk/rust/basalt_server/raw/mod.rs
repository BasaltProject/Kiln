
    #[link(wasm_import_module = "basalt")]
    extern "C" {
        pub fn log(level: i32, msg: *const u8, len: i32);
        pub fn config_i64(key: *const u8, klen: i32, def: i64) -> i64;
        pub fn config_f64(key: *const u8, klen: i32, def: f64) -> f64;
        pub fn config_bool(key: *const u8, klen: i32, def: i32) -> i32;
        pub fn config_str(key: *const u8, klen: i32, out: *mut u8, cap: i32) -> i32;
        pub fn translate(key: *const u8, klen: i32, args: *const u8, alen: i32, out: *mut u8, cap: i32) -> i32;
        pub fn player_by_name(name: *const u8, len: i32) -> i32;
        pub fn players_online(out: *mut i32, cap: i32) -> i32;
        pub fn player_name(player: i32, out: *mut u8, cap: i32) -> i32;
        pub fn player_send(player: i32, text: *const u8, tlen: i32, args: *const u8, alen: i32) -> i32;
        pub fn player_action_bar(player: i32, text: *const u8, tlen: i32, args: *const u8, alen: i32) -> i32;
        pub fn player_particle(player: i32, id: *const u8, idlen: i32, count: i32, spread: f64) -> i32;
        pub fn player_sound(player: i32, id: *const u8, idlen: i32, volume: f32, pitch: f32) -> i32;
        pub fn data_get_i64(player: i32, key: *const u8, klen: i32, def: i64) -> i64;
        pub fn data_set_i64(player: i32, key: *const u8, klen: i32, value: i64) -> i32;
        pub fn data_get_bool(player: i32, key: *const u8, klen: i32, def: i32) -> i32;
        pub fn data_set_bool(player: i32, key: *const u8, klen: i32, value: i32) -> i32;
        pub fn data_get_str(player: i32, key: *const u8, klen: i32, out: *mut u8, cap: i32) -> i32;
        pub fn data_set_str(player: i32, key: *const u8, klen: i32, value: *const u8, vlen: i32) -> i32;
        pub fn data_remove(player: i32, key: *const u8, klen: i32) -> i32;
        pub fn store_top(key: *const u8, klen: i32, limit: i32, out: *mut u8, cap: i32) -> i32;
        pub fn event_on(name: *const u8, nlen: i32, priority: i32, token: i32) -> i32;
        pub fn event_player(ctx: i32) -> i32;
        pub fn event_field_str(ctx: i32, field: *const u8, flen: i32, out: *mut u8, cap: i32) -> i32;
        pub fn event_field_i64(ctx: i32, field: *const u8, flen: i32, out: *mut i64) -> i32;
        pub fn event_cancel(ctx: i32, cancelled: i32) -> i32;
        pub fn command_register(spec: *const u8, slen: i32, permission: *const u8, plen: i32, token: i32) -> i32;
        pub fn command_source_name(ctx: i32, out: *mut u8, cap: i32) -> i32;
        pub fn command_is_console(ctx: i32) -> i32;
        pub fn command_reply(ctx: i32, text: *const u8, tlen: i32, args: *const u8, alen: i32) -> i32;
        pub fn command_arg_str(ctx: i32, name: *const u8, nlen: i32, out: *mut u8, cap: i32) -> i32;
        pub fn command_arg_i64(ctx: i32, name: *const u8, nlen: i32, out: *mut i64) -> i32;
        pub fn task_after(delay: i32, token: i32) -> i32;
        pub fn task_every(delay: i32, period: i32, token: i32) -> i32;
        pub fn task_cancel(task: i32) -> i32;
        pub fn placeholder_register(name: *const u8, nlen: i32, token: i32) -> i32;
        pub fn placeholder_player(ctx: i32) -> i32;
        pub fn placeholder_result(ctx: i32, text: *const u8, len: i32) -> i32;
        pub fn menu_create(title: *const u8, tlen: i32, rows: i32) -> i32;
        pub fn menu_set(menu: i32, slot: i32, item: *const u8, ilen: i32, name: *const u8, nlen: i32, lore: *const u8, llen: i32, token: i32) -> i32;
        pub fn menu_open(menu: i32, player: i32) -> i32;
        pub fn menu_click_player(ctx: i32) -> i32;
    }

// Generated area modules
pub mod core_values;
pub mod entities;
pub mod events;
pub mod hud;
pub mod items;
pub mod packets;
pub mod server;
pub mod worlds;
