use crate::core::*;
#[path = "../common/events.rs"]
mod shared;
pub use shared::*;

use crate::raw::events as raw;
use crate::core::{fetch, p, token_for, Ctx};

const FALSE: u8 = 1;
const TRUE: u8 = 2;
const INT: u8 = 3;
const STRING: u8 = 5;

fn fetch_value(call: impl Fn(*mut u8, i32) -> i32) -> Result<Vec<u8>, i32> {
    let mut buffer = vec![0u8; 256];
    loop {
        let needed = call(buffer.as_mut_ptr(), buffer.len() as i32);
        if needed < 0 {
            return Err(needed);
        }
        if needed as usize <= buffer.len() {
            buffer.truncate(needed as usize);
            return Ok(buffer);
        }
        buffer.resize(needed as usize, 0);
    }
}

fn read_leb(bytes: &[u8], at: &mut usize) -> Option<u64> {
    let mut result = 0u64;
    let mut shift = 0;
    while shift < 70 && *at < bytes.len() {
        let byte = bytes[*at];
        *at += 1;
        result |= ((byte & 0x7f) as u64) << shift;
        if byte & 0x80 == 0 {
            return Some(result);
        }
        shift += 7;
    }
    None
}

fn write_leb(out: &mut Vec<u8>, mut value: u64) {
    loop {
        let byte = (value & 0x7f) as u8;
        value >>= 7;
        out.push(if value != 0 { byte | 0x80 } else { byte });
        if value == 0 {
            break;
        }
    }
}

/// Decides or observes `event`: an observer sees the finished event, its context is read only.
pub fn watch(event: &str, handler: impl FnMut(Ctx) + 'static) -> i32 {
    let (e, el) = p(event);
    unsafe { raw::event_watch(e, el, token_for(Box::new(handler))) }
}

/// The encoded value of a field; an empty name gives a map of every readable field. Negative codes are the Err.
pub fn event_get_value(ctx: Ctx, field: &str) -> Result<Vec<u8>, i32> {
    let (f, fl) = p(field);
    fetch_value(|out, cap| unsafe { raw::event_get(ctx, f, fl, out, cap) })
}

pub fn event_get_str(ctx: Ctx, field: &str) -> Option<String> {
    let bytes = event_get_value(ctx, field).ok()?;
    let mut at = 1;
    if bytes.first() != Some(&STRING) {
        return None;
    }
    let len = read_leb(&bytes, &mut at)? as usize;
    String::from_utf8(bytes.get(at..at + len)?.to_vec()).ok()
}

pub fn event_get_i64(ctx: Ctx, field: &str) -> Option<i64> {
    let bytes = event_get_value(ctx, field).ok()?;
    let mut at = 1;
    if bytes.first() != Some(&INT) {
        return None;
    }
    let zigzag = read_leb(&bytes, &mut at)?;
    Some(((zigzag >> 1) as i64) ^ -((zigzag & 1) as i64))
}

pub fn event_get_bool(ctx: Ctx, field: &str) -> Option<bool> {
    match event_get_value(ctx, field).ok()?.as_slice() {
        [TRUE] => Some(true),
        [FALSE] => Some(false),
        _ => None,
    }
}

/// The list of maps (name, type, readable, writable) describing every field of the event.
pub fn event_fields(ctx: Ctx) -> Result<Vec<u8>, i32> {
    fetch_value(|out, cap| unsafe { raw::event_fields(ctx, out, cap) })
}

pub fn event_set_value(ctx: Ctx, field: &str, value: &[u8]) -> i32 {
    let (f, fl) = p(field);
    unsafe { raw::event_set(ctx, f, fl, value.as_ptr(), value.len() as i32) }
}

pub fn event_set_text(ctx: Ctx, field: &str, text: &str) -> i32 {
    let mut value = vec![STRING];
    write_leb(&mut value, text.len() as u64);
    value.extend_from_slice(text.as_bytes());
    event_set_value(ctx, field, &value)
}

pub fn event_set_int(ctx: Ctx, field: &str, number: i64) -> i32 {
    let mut value = vec![INT];
    write_leb(&mut value, ((number << 1) ^ (number >> 63)) as u64);
    event_set_value(ctx, field, &value)
}

pub fn event_set_flag(ctx: Ctx, field: &str, flag: bool) -> i32 {
    event_set_value(ctx, field, &[if flag { TRUE } else { FALSE }])
}

/// Cancels a decided event and shows `reason` (MiniMessage, or a '@' lang key) to the player.
pub fn event_deny(ctx: Ctx, reason: &str) -> i32 {
    let shown = if reason.is_empty() { 0 } else { event_set_text(ctx, "reason", reason) };
    if shown < 0 { shown } else { event_set_flag(ctx, "cancelled", true) }
}

/// `field` is "id", "name" or "version".
pub fn plugin_info(field: &str) -> Option<String> {
    let (f, fl) = p(field);
    fetch(|out, cap| unsafe { raw::plugin_info(f, fl, out, cap) })
}

pub fn plugin_manifest() -> Result<Vec<u8>, i32> {
    fetch_value(|out, cap| unsafe { raw::plugin_manifest(out, cap) })
}

/// The config.toml value at a dotted key (arrays are lists, tables are maps); an empty key is the whole file.
pub fn config_value(key: &str) -> Result<Vec<u8>, i32> {
    let (k, kl) = p(key);
    fetch_value(|out, cap| unsafe { raw::config_value(k, kl, out, cap) })
}

pub fn command_arg_f64(ctx: Ctx, name: &str) -> Option<f64> {
    let (n, nl) = p(name);
    let mut value = 0f64;
    if unsafe { raw::command_arg_f64(ctx, n, nl, &mut value) } == 0 { Some(value) } else { None }
}

pub fn command_arg_bool(ctx: Ctx, name: &str) -> Option<bool> {
    let (n, nl) = p(name);
    let mut value = 0i32;
    if unsafe { raw::command_arg_bool(ctx, n, nl, &mut value) } == 0 { Some(value != 0) } else { None }
}

pub fn command_args(ctx: Ctx) -> Result<Vec<u8>, i32> {
    fetch_value(|out, cap| unsafe { raw::command_args(ctx, out, cap) })
}

pub fn event_set_str(ctx: Ctx, field: &str, value: &str) -> i32 {
    let (f, fl) = p(field);
    let (v, vl) = p(value);
    unsafe { crate::raw::event_set_str(ctx, f, fl, v, vl) }
}

pub fn event_set_i64(ctx: Ctx, field: &str, value: i64) -> i32 {
    let (f, fl) = p(field);
    unsafe { crate::raw::event_set_i64(ctx, f, fl, value) }
}
