//! Values the plugin keeps per player (capability `players_data`).
use crate::core::{fetch, p, Player};
use crate::raw;

pub fn get_i64(player: Player, key: &str, def: i64) -> i64 {
    let (k, kl) = p(key);
    unsafe { raw::data_get_i64(player, k, kl, def) }
}

pub fn set_i64(player: Player, key: &str, value: i64) -> i32 {
    let (k, kl) = p(key);
    unsafe { raw::data_set_i64(player, k, kl, value) }
}

pub fn get_bool(player: Player, key: &str, def: bool) -> bool {
    let (k, kl) = p(key);
    unsafe { raw::data_get_bool(player, k, kl, def as i32) != 0 }
}

pub fn set_bool(player: Player, key: &str, value: bool) -> i32 {
    let (k, kl) = p(key);
    unsafe { raw::data_set_bool(player, k, kl, value as i32) }
}

pub fn get_str(player: Player, key: &str) -> Option<String> {
    let (k, kl) = p(key);
    fetch(|out, cap| unsafe { raw::data_get_str(player, k, kl, out, cap) })
}

pub fn set_str(player: Player, key: &str, value: &str) -> i32 {
    let (k, kl) = p(key);
    let (v, vl) = p(value);
    unsafe { raw::data_set_str(player, k, kl, v, vl) }
}

pub fn remove(player: Player, key: &str) -> i32 {
    let (k, kl) = p(key);
    unsafe { raw::data_remove(player, k, kl) }
}
