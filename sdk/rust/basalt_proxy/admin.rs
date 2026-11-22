
use crate::raw::core_values as core_raw;
use crate::raw::admin as raw;
use crate::values::{call_value, CallError, OwnedValue, Writer};
use crate::core::{fetch, p, token_for, Ctx, Player};

const INVALID: i32 = -3;

fn written(writer: Writer) -> Option<Vec<u8>> {
    writer.into_vec().ok()
}

/// Ok, or Err with the reason the proxy gave (INVALID).
pub fn servers_register(name: &str, host: &str, port: i32, like: &str) -> Result<(), String> {
    let (n, nl) = p(name);
    let (h, hl) = p(host);
    let (l, ll) = p(like);
    let mut reason = vec![0u8; 256];
    let status = unsafe { raw::servers_register(n, nl, h, hl, port, l, ll, reason.as_mut_ptr(), reason.len() as i32) };
    if status >= 0 {
        return Ok(());
    }
    let end = reason.iter().position(|b| *b == 0).unwrap_or(reason.len());
    reason.truncate(end);
    Err(String::from_utf8(reason).unwrap_or_default())
}

pub fn servers_unregister(name: &str) -> i32 {
    let (n, nl) = p(name);
    unsafe { raw::servers_unregister(n, nl) }
}

/// Pings a backend; the handler reads the answer with `completion_value`. Returns the request handle.
pub fn servers_ping(name: &str, handler: impl FnMut(Ctx) + 'static) -> i32 {
    let (n, nl) = p(name);
    unsafe { raw::servers_ping(n, nl, token_for(Box::new(handler))) }
}

/// Runs the handler after `ticks` ticks of 50 ms. Returns the request handle.
pub fn wait(ticks: i32, handler: impl FnMut(Ctx) + 'static) -> i32 {
    unsafe { raw::wait(ticks, token_for(Box::new(handler))) }
}

pub fn request_cancel(request: i32) -> i32 {
    unsafe { core_raw::request_cancel(request) }
}

pub fn completion_value(ctx: Ctx) -> Result<OwnedValue, CallError> {
    call_value(256, |out, cap| unsafe { core_raw::value_result(ctx, out, cap) })
}

/// 1 when the line named a command, 0 when not, DENIED (-4) without the capability and the operator's grant.
pub fn console_run(line: &str) -> i32 {
    let (l, ll) = p(line);
    unsafe { raw::console_run(l, ll) }
}

pub fn player_chat(player: Player, text: &str) -> i32 {
    let (t, tl) = p(text);
    unsafe { raw::player_chat(player, t, tl) }
}

pub fn player_send_json(player: Player, json: &str) -> i32 {
    let (j, jl) = p(json);
    unsafe { raw::player_send_json(player, j, jl) }
}

pub fn player_kick_json(player: Player, json: &str) -> i32 {
    let (j, jl) = p(json);
    unsafe { raw::player_kick_json(player, j, jl) }
}

/// 1 granted, 0 denied, ABSENT (-1) undecided. `who` "" is the console; `server` may be empty.
pub fn permission_check(who: &str, node: &str, server: &str) -> i32 {
    let (w, wl) = p(who);
    let (n, nl) = p(node);
    let mut context = Writer::new();
    if !server.is_empty() {
        context.begin_map().key("server").string(server).end();
    }
    let doc = if server.is_empty() { Some(Vec::new()) } else { written(context) };
    match doc {
        Some(bytes) => unsafe { raw::permission_check(w, wl, n, nl, bytes.as_ptr(), bytes.len() as i32) },
        None => INVALID,
    }
}

pub fn permission_meta(who: &str, key: &str) -> Option<String> {
    let (w, wl) = p(who);
    let (k, kl) = p(key);
    fetch(|out, cap| unsafe { raw::permission_meta(w, wl, k, kl, k, 0, out, cap) })
}

/// Registers provider callbacks (None for none); returns the provider handle.
pub fn permission_provide(
    check: Option<Box<dyn FnMut(Ctx)>>,
    meta: Option<Box<dyn FnMut(Ctx)>>,
    priority: i32,
) -> i32 {
    let check_token = check.map(token_for).unwrap_or(0);
    let meta_token = meta.map(token_for).unwrap_or(0);
    unsafe { raw::permission_provide(check_token, meta_token, priority) }
}

pub fn permission_unprovide(provider: i32) -> i32 {
    unsafe { raw::permission_unprovide(provider) }
}

/// The question of a provider callback: a map with kind, name, uuid, console, text and context.
pub fn permission_request(ctx: Ctx) -> Result<OwnedValue, CallError> {
    call_value(256, |out, cap| unsafe { raw::permission_request(ctx, out, cap) })
}

/// 1 grants, 0 denies, negative leaves it to the next provider.
pub fn permission_answer(ctx: Ctx, answer: i32) -> i32 {
    unsafe { raw::permission_answer(ctx, answer) }
}

pub fn permission_answer_text(ctx: Ctx, text: &str) -> i32 {
    let (t, tl) = p(text);
    unsafe { raw::permission_answer_text(ctx, t, tl) }
}

pub fn permission_changed() -> i32 {
    unsafe { raw::permission_changed() }
}

/// 1 grants, 0 denies, -1 removes.
pub fn permission_override(name: &str, node: &str, value: i32) -> i32 {
    let (n, nl) = p(name);
    let (o, ol) = p(node);
    unsafe { raw::permission_override(n, nl, o, ol, value) }
}

pub fn permission_list(name: &str) -> Result<OwnedValue, CallError> {
    let (n, nl) = p(name);
    call_value(256, |out, cap| unsafe { raw::permission_list(n, nl, out, cap) })
}

pub fn group_get(name: &str) -> Option<String> {
    let (n, nl) = p(name);
    fetch(|out, cap| unsafe { raw::group_get(n, nl, out, cap) })
}

pub fn group_set(name: &str, group: &str) -> i32 {
    let (n, nl) = p(name);
    let (g, gl) = p(group);
    unsafe { raw::group_set(n, nl, g, gl) }
}

/// Reads a store key (holder None) or a player data key of `holder`; Err(Absent) when unset.
pub fn data_value(holder: Option<&str>, key: &str) -> Result<OwnedValue, CallError> {
    let (k, kl) = p(key);
    call_value(256, |out, cap| unsafe {
        match holder {
            Some(h) => {
                let (hp, hl) = p(h);
                raw::player_data_get(hp, hl, k, kl, out, cap)
            }
            None => raw::store_get(k, kl, out, cap),
        }
    })
}

pub fn data_set_value(holder: Option<&str>, key: &str, doc: &[u8]) -> i32 {
    let (k, kl) = p(key);
    let (d, dl) = (doc.as_ptr(), doc.len() as i32);
    unsafe {
        match holder {
            Some(h) => {
                let (hp, hl) = p(h);
                raw::player_data_set(hp, hl, k, kl, d, dl)
            }
            None => raw::store_set(k, kl, d, dl),
        }
    }
}

fn data_set_with(holder: Option<&str>, key: &str, fill: impl FnOnce(&mut Writer)) -> i32 {
    let mut writer = Writer::new();
    fill(&mut writer);
    match written(writer) {
        Some(doc) => data_set_value(holder, key, &doc),
        None => INVALID,
    }
}

pub fn data_set_i64(holder: Option<&str>, key: &str, number: i64) -> i32 {
    data_set_with(holder, key, |w| {
        w.int(number);
    })
}

pub fn data_set_f64(holder: Option<&str>, key: &str, number: f64) -> i32 {
    data_set_with(holder, key, |w| {
        w.float(number);
    })
}

pub fn data_set_bool(holder: Option<&str>, key: &str, flag: bool) -> i32 {
    data_set_with(holder, key, |w| {
        w.boolean(flag);
    })
}

pub fn data_set_str(holder: Option<&str>, key: &str, text: &str) -> i32 {
    data_set_with(holder, key, |w| {
        w.string(text);
    })
}

pub fn data_remove(holder: Option<&str>, key: &str) -> i32 {
    data_set_with(holder, key, |w| {
        w.null();
    })
}

/// The best `limit` numeric values of a store key: a list of maps with uuid, name and value.
pub fn store_top(key: &str, limit: i32) -> Result<OwnedValue, CallError> {
    let (k, kl) = p(key);
    call_value(256, |out, cap| unsafe { raw::store_top(k, kl, limit, out, cap) })
}
