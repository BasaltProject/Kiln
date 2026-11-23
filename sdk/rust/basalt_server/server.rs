//! Server area: permissions, dynamic commands, broadcast, plugin files and storage, services and scheduler completions.
#![allow(dead_code)]

use crate::raw::core_values as core;
use crate::raw::server as raw;
use crate::values::{call_value, CallError, OwnedValue, Writer};
use crate::core::{token_for, with_args, Args, Ctx, Player};

pub const BROADCAST_PLAYERS: i32 = 1;
pub const BROADCAST_CONSOLE: i32 = 2;
pub const PROVIDE_CHECK: i32 = 1;
pub const PROVIDE_META: i32 = 2;

const BUFFER: usize = 1024;
const ERROR_INVALID: i32 = -3;

/// A value answer, or why there is none.
pub type Reply = Result<OwnedValue, CallError>;

fn p(text: &str) -> (*const u8, i32) {
    (text.as_ptr(), text.len() as i32)
}

fn fill(cap: usize, call: impl Fn(*mut u8, i32) -> i32) -> Result<String, i32> {
    let mut buffer = vec![0u8; cap];
    loop {
        let n = call(buffer.as_mut_ptr(), buffer.len() as i32);
        if n < 0 {
            return Err(n);
        }
        if n as usize <= buffer.len() {
            buffer.truncate(n as usize);
            return Ok(String::from_utf8_lossy(&buffer).into_owned());
        }
        buffer.resize(n as usize, 0);
    }
}

/// The import status of a call: 0 for an answer, the negative error otherwise.
pub fn status<T>(result: &Result<T, CallError>) -> i32 {
    match result {
        Ok(_) => 0,
        Err(CallError::Absent) => -1,
        Err(CallError::Stale) => -2,
        Err(CallError::Invalid) => ERROR_INVALID,
        Err(CallError::Denied) => -4,
        Err(CallError::Failed(code)) => *code,
        Err(CallError::TooLarge) => -100,
    }
}

/// The value a completion callback (a wait, a lookup, a service call) carries.
pub fn result(ctx: Ctx) -> Reply {
    call_value(BUFFER, |out, cap| unsafe { core::value_result(ctx, out, cap) })
}

pub fn permission_check(subject: &str, node: &str) -> i32 {
    let (s, sl) = p(subject);
    let (n, nl) = p(node);
    unsafe { raw::permission_check(s, sl, n, nl) }
}

pub fn permission_has(player: Player, node: &str) -> i32 {
    let (n, nl) = p(node);
    unsafe { raw::permission_has(player, n, nl) }
}

fn permission_set(player: Player, node: &str, mode: i32) -> i32 {
    let (n, nl) = p(node);
    unsafe { raw::permission_set(player, n, nl, mode) }
}

pub fn permission_grant(player: Player, node: &str) -> i32 {
    permission_set(player, node, 1)
}

pub fn permission_deny(player: Player, node: &str) -> i32 {
    permission_set(player, node, 0)
}

/// Drops the plugin's own entry.
pub fn permission_drop(player: Player, node: &str) -> i32 {
    permission_set(player, node, -1)
}

pub fn permission_meta(subject: &str, key: &str) -> Result<String, i32> {
    let (s, sl) = p(subject);
    let (k, kl) = p(key);
    fill(128, |out, cap| unsafe { raw::permission_meta(s, sl, k, kl, out, cap) })
}

pub fn permission_provide(priority: i32, kinds: i32, handler: impl FnMut(Ctx) + 'static) -> i32 {
    unsafe { raw::permission_provide(priority, kinds, token_for(Box::new(handler))) }
}

pub fn permission_provider_remove(provider: i32) -> i32 {
    unsafe { raw::permission_provider_remove(provider) }
}

pub fn permission_answer(ctx: Ctx, answer: i32) -> i32 {
    unsafe { raw::permission_answer(ctx, answer) }
}

pub fn permission_answer_meta(ctx: Ctx, text: &str) -> i32 {
    let (t, tl) = p(text);
    unsafe { raw::permission_answer_meta(ctx, t, tl) }
}

pub fn permission_changed() -> i32 {
    unsafe { raw::permission_changed() }
}

/// The node or meta key a provider callback is asked about (`"node"`, `"kind"`), cut to 128 bytes.
pub fn permission_query(ctx: Ctx, field: &str) -> Result<String, i32> {
    let reply = result(ctx).map_err(|error| status(&Err::<(), _>(error)))?;
    let text = reply.root().find(field).and_then(|value| value.as_str_bytes()).ok_or(ERROR_INVALID)?;
    let text = &text[..text.len().min(128)];
    Ok(String::from_utf8_lossy(text).into_owned())
}

pub fn command(spec: &str, permission: &str, handler: impl FnMut(Ctx) + 'static) -> i32 {
    command_ex(spec, permission, handler, None::<fn(Ctx)>)
}

/// A dynamic command whose `complete` callback adds candidates with `complete_add`.
pub fn command_ex(
    spec: &str,
    permission: &str,
    handler: impl FnMut(Ctx) + 'static,
    complete: Option<impl FnMut(Ctx) + 'static>,
) -> i32 {
    let (s, sl) = p(spec);
    let (pm, pl) = p(permission);
    let token = token_for(Box::new(handler));
    let complete_token = match complete {
        Some(handler) => token_for(Box::new(handler)),
        None => -1,
    };
    unsafe { raw::command_register_ex(s, sl, pm, pl, token, complete_token) }
}

pub fn command_alias(name: &str, alias: &str) -> i32 {
    let (n, nl) = p(name);
    let (a, al) = p(alias);
    unsafe { raw::command_alias(n, nl, a, al) }
}

pub fn command_unregister(name: &str) -> i32 {
    let (n, nl) = p(name);
    unsafe { raw::command_unregister(n, nl) }
}

/// Runs a command line as the console (player 0) or as `player`.
pub fn command_run(line: &str, player: Player) -> i32 {
    let (l, ll) = p(line);
    unsafe { raw::command_run(l, ll, player) }
}

pub fn complete_input(ctx: Ctx) -> Result<String, i32> {
    fill(256, |out, cap| unsafe { raw::command_complete_input(ctx, out, cap) })
}

pub fn complete_add(ctx: Ctx, candidate: &str) -> i32 {
    let (c, cl) = p(candidate);
    unsafe { raw::command_complete_add(ctx, c, cl) }
}

pub fn command_arg_f64(ctx: Ctx, name: &str) -> Option<f64> {
    let (n, nl) = p(name);
    let mut value = 0.0f64;
    (unsafe { raw::command_arg_f64(ctx, n, nl, &mut value) } >= 0).then_some(value)
}

pub fn command_arg_bool(ctx: Ctx, name: &str) -> Option<bool> {
    let (n, nl) = p(name);
    let mut value = 0i32;
    (unsafe { raw::command_arg_bool(ctx, n, nl, &mut value) } >= 0).then_some(value != 0)
}

/// `text` may be a "@key" lang key.
pub fn broadcast(text: &str, args: &Args, flags: i32) -> i32 {
    let (t, tl) = p(text);
    with_args(args, |a, al| unsafe { raw::server_broadcast(t, tl, a, al, flags) })
}

pub fn placeholder_fill(text: &str, player: Player) -> Result<String, i32> {
    let (t, tl) = p(text);
    fill(256, |out, cap| unsafe { raw::placeholder_fill(t, tl, player, out, cap) })
}

pub fn info() -> Reply {
    call_value(BUFFER, |out, cap| unsafe { raw::server_info(out, cap) })
}

pub fn call(op: &str, args: Option<&Writer>) -> Reply {
    let document = match args {
        Some(writer) => writer.finish().map_err(|_| CallError::Invalid)?,
        None => &[],
    };
    let (o, ol) = p(op);
    let (a, al) = if args.is_some() { p_bytes(document) } else { (std::ptr::null(), 0) };
    call_value(BUFFER, |out, cap| unsafe { raw::server_call(o, ol, a, al, out, cap) })
}

fn p_bytes(bytes: &[u8]) -> (*const u8, i32) {
    (bytes.as_ptr(), bytes.len() as i32)
}

pub fn profile_lookup(name: &str, handler: impl FnMut(Ctx) + 'static) -> i32 {
    let (n, nl) = p(name);
    unsafe { raw::profile_lookup(n, nl, token_for(Box::new(handler))) }
}

/// Reads a plugin file into `out`; the length it needs or the negative error.
pub fn file_read(path: &str, out: &mut [u8]) -> i32 {
    let (pa, pl) = p(path);
    unsafe { raw::files_read(pa, pl, out.as_mut_ptr(), out.len() as i32) }
}

pub fn file_write(path: &str, data: &str) -> i32 {
    let (pa, pl) = p(path);
    let (d, dl) = p(data);
    unsafe { raw::files_write(pa, pl, d, dl) }
}

pub fn file_exists(path: &str) -> bool {
    let (pa, pl) = p(path);
    unsafe { raw::files_exists(pa, pl) == 1 }
}

pub fn file_delete(path: &str, recursive: bool) -> i32 {
    let (pa, pl) = p(path);
    unsafe { raw::files_delete(pa, pl, recursive as i32) }
}

pub fn file_list(prefix: &str) -> Reply {
    let (pa, pl) = p(prefix);
    call_value(BUFFER, |out, cap| unsafe { raw::files_list(pa, pl, out, cap) })
}

pub fn file_stat(path: &str) -> Reply {
    let (pa, pl) = p(path);
    call_value(BUFFER, |out, cap| unsafe { raw::files_stat(pa, pl, out, cap) })
}

pub fn file_walk(path: &str) -> Reply {
    let (pa, pl) = p(path);
    call_value(BUFFER, |out, cap| unsafe { raw::files_walk(pa, pl, out, cap) })
}

pub fn store_set_i64(key: &str, value: i64) -> i32 {
    let mut writer = Writer::with_limit(16);
    writer.int(value);
    store_set(key, &writer)
}

pub fn store_set_str(key: &str, value: &str) -> i32 {
    let mut writer = Writer::new();
    writer.string(value);
    store_set(key, &writer)
}

fn store_set(key: &str, writer: &Writer) -> i32 {
    let Ok(document) = writer.finish() else { return ERROR_INVALID };
    let (k, kl) = p(key);
    let (v, vl) = p_bytes(document);
    unsafe { raw::store_set(k, kl, v, vl) }
}

pub fn store_get(key: &str) -> Reply {
    let (k, kl) = p(key);
    call_value(BUFFER, |out, cap| unsafe { raw::store_get(k, kl, out, cap) })
}

pub fn data_write(name: &str, doc: &Writer) -> i32 {
    let Ok(document) = doc.finish() else { return ERROR_INVALID };
    let (n, nl) = p(name);
    let (v, vl) = p_bytes(document);
    unsafe { raw::data_write(n, nl, v, vl) }
}

pub fn data_read(name: &str) -> Reply {
    let (n, nl) = p(name);
    call_value(BUFFER, |out, cap| unsafe { raw::data_read(n, nl, out, cap) })
}

pub fn config_value(key: &str) -> Reply {
    let (k, kl) = p(key);
    call_value(BUFFER, |out, cap| unsafe { raw::config_value(k, kl, out, cap) })
}

pub fn service_register(name: &str, handler: impl FnMut(Ctx) + 'static) -> i32 {
    let (n, nl) = p(name);
    unsafe { raw::service_register(n, nl, token_for(Box::new(handler))) }
}

pub fn service_reply(ctx: Ctx, doc: &Writer) -> i32 {
    let Ok(document) = doc.finish() else { return ERROR_INVALID };
    let (v, vl) = p_bytes(document);
    unsafe { raw::service_reply(ctx, v, vl) }
}

pub fn service_reply_null(ctx: Ctx) -> i32 {
    unsafe { raw::service_reply(ctx, std::ptr::null(), 0) }
}

pub fn service_remove(service: i32) -> i32 {
    unsafe { raw::service_remove(service) }
}

pub fn service_has(name: &str) -> bool {
    let (n, nl) = p(name);
    unsafe { raw::service_has(n, nl) == 1 }
}

pub fn service_list() -> Reply {
    call_value(BUFFER, |out, cap| unsafe { raw::service_list(out, cap) })
}

pub fn service_call(name: &str, args: &Writer, timeout: i32, handler: impl FnMut(Ctx) + 'static) -> i32 {
    let Ok(document) = args.finish() else { return ERROR_INVALID };
    let (n, nl) = p(name);
    let (a, al) = p_bytes(document);
    unsafe { raw::service_call(n, nl, a, al, timeout, token_for(Box::new(handler))) }
}

pub fn wait(ticks: i32, handler: impl FnMut(Ctx) + 'static) -> i32 {
    unsafe { raw::task_wait(ticks, token_for(Box::new(handler))) }
}

pub fn at(world: &str, x: f64, z: f64, handler: impl FnMut(Ctx) + 'static) -> i32 {
    let (w, wl) = p(world);
    unsafe { raw::task_at(w, wl, x, z, token_for(Box::new(handler))) }
}
