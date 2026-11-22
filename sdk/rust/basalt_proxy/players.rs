use crate::raw;
use crate::core::*;
#[path = "../common/players.rs"]
mod shared;
pub use shared::*;

pub fn player_uuid(player: Player) -> Option<String> {
    fetch(|out, cap| unsafe { crate::raw::player_uuid(player, out, cap) })
}

pub fn player_server(player: Player) -> Option<String> {
    fetch(|out, cap| unsafe { crate::raw::player_server(player, out, cap) })
}

pub fn player_ping(player: Player) -> i32 {
    unsafe { crate::raw::player_ping(player) }
}

pub fn player_setting_str(player: Player, field: &str) -> Option<String> {
    let (f, fl) = p(field);
    fetch(|out, cap| unsafe { crate::raw::player_setting_str(player, f, fl, out, cap) })
}

pub fn player_setting_i64(player: Player, field: &str) -> Option<i64> {
    let (f, fl) = p(field);
    let mut value = 0i64;
    if unsafe { crate::raw::player_setting_i64(player, f, fl, &mut value) } < 0 { None } else { Some(value) }
}

pub fn player_has_permission(player: Player, node: &str) -> bool {
    let (n, nl) = p(node);
    unsafe { crate::raw::player_has_permission(player, n, nl) == 1 }
}

pub fn player_title(player: Player, title: &str, subtitle: &str, fade_in: i32, stay: i32, fade_out: i32) -> i32 {
    let (t, tl) = p(title);
    let (s, sl) = p(subtitle);
    unsafe { crate::raw::player_title(player, t, tl, s, sl, fade_in, stay, fade_out) }
}

pub fn player_connect(player: Player, server: &str) -> i32 {
    let (s, sl) = p(server);
    unsafe { crate::raw::player_connect(player, s, sl) }
}

pub fn player_kick(player: Player, reason: &str, args: &Args) -> i32 {
    let (r, rl) = p(reason);
    with_args(args, |a, al| unsafe { crate::raw::player_kick(player, r, rl, a, al) })
}
