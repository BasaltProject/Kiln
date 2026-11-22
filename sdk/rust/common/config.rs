//! The plugin's config.toml, read by dotted key with a default.
use crate::core::{fetch, p};
use crate::raw;

pub fn i64(key: &str, def: i64) -> i64 {
    let (k, kl) = p(key);
    unsafe { raw::config_i64(k, kl, def) }
}

pub fn f64(key: &str, def: f64) -> f64 {
    let (k, kl) = p(key);
    unsafe { raw::config_f64(k, kl, def) }
}

pub fn bool(key: &str, def: bool) -> bool {
    let (k, kl) = p(key);
    unsafe { raw::config_bool(k, kl, def as i32) != 0 }
}

pub fn str(key: &str) -> Option<String> {
    let (k, kl) = p(key);
    fetch(|out, cap| unsafe { raw::config_str(k, kl, out, cap) })
}
