
use crate::raw::core_values as core_raw;
use crate::raw::interaction as raw;
use crate::values::{call_value, CallError, OwnedValue, Writer};
use crate::core::{fetch, p, token_for, with_args, Args, Ctx, Player};

const INVALID: i32 = -3;

/// Observes player.resource_pack: the value holds player, player_name, id and status.
pub fn watch_resource_packs(handler: impl FnMut(Ctx) + 'static) -> i32 {
    let (n, nl) = p("player.resource_pack");
    unsafe { raw::interaction_watch(n, nl, token_for(Box::new(handler))) }
}

/// Observes player.settings: the value holds player, player_name and settings.
pub fn watch_settings(handler: impl FnMut(Ctx) + 'static) -> i32 {
    let (n, nl) = p("player.settings");
    unsafe { raw::interaction_watch(n, nl, token_for(Box::new(handler))) }
}

/// The value of an observed event or a completion.
pub fn value_result(ctx: Ctx) -> Result<OwnedValue, CallError> {
    call_value(256, |out, cap| unsafe { core_raw::value_result(ctx, out, cap) })
}

/// kind is "notice", "confirmation" or "multi_action"; each button is a (label, command) pair.
pub fn dialog_show(
    player: Player,
    kind: &str,
    title: &str,
    body: Option<&str>,
    buttons: &[(&str, &str)],
    closable: bool,
) -> i32 {
    let mut w = Writer::new();
    w.begin_map().key("kind").string(kind).key("title").string(title);
    if let Some(line) = body {
        w.key("body").string(line);
    }
    w.key("buttons").begin_list();
    for (label, command) in buttons {
        w.begin_map().key("label").string(label).key("command").string(command).end();
    }
    w.end().key("can_close_with_escape").boolean(closable).end();
    match w.finish() {
        Ok(bytes) => unsafe { raw::dialog_show(player, bytes.as_ptr(), bytes.len() as i32) },
        Err(_) => INVALID,
    }
}

pub fn dialog_clear(player: Player) -> i32 {
    unsafe { raw::dialog_clear(player) }
}

/// Pushes a pack and returns its id text; the Err is the ABI code.
pub fn resource_pack_push(
    player: Player,
    url: &str,
    hash: Option<&str>,
    required: bool,
    prompt: Option<&str>,
    id: Option<&str>,
) -> Result<String, i32> {
    let mut w = Writer::new();
    w.begin_map().key("url").string(url);
    if let Some(text) = hash {
        w.key("hash").string(text);
    }
    w.key("required").boolean(required);
    if let Some(text) = prompt {
        w.key("prompt").string(text);
    }
    if let Some(text) = id {
        w.key("id").string(text);
    }
    w.end();
    let bytes = w.finish().map_err(|_| INVALID)?;
    let pack = fetch(|out, cap| unsafe { raw::resource_pack_push(player, bytes.as_ptr(), bytes.len() as i32, out, cap) });
    pack.ok_or(INVALID)
}

pub fn resource_pack_remove(player: Player, id: &str) -> i32 {
    let (i, il) = p(id);
    unsafe { raw::resource_pack_remove(player, i, il) }
}

pub fn resource_pack_clear(player: Player) -> i32 {
    unsafe { raw::resource_pack_clear(player) }
}

pub fn cookie_set(player: Player, key: &str, data: &[u8]) -> i32 {
    let (k, kl) = p(key);
    unsafe { raw::cookie_set(player, k, kl, data.as_ptr(), data.len() as i32) }
}

/// Returns the request handle; the handler reads the cookie with `cookie_result`.
pub fn cookie_request(player: Player, key: &str, handler: impl FnMut(Ctx) + 'static) -> i32 {
    let (k, kl) = p(key);
    unsafe { raw::cookie_request(player, k, kl, token_for(Box::new(handler))) }
}

/// The cookie bytes of a completion; None when the client had none.
pub fn cookie_result(ctx: Ctx) -> Option<Vec<u8>> {
    let value = value_result(ctx).ok()?;
    value.root().as_bytes().map(|bytes| bytes.to_vec())
}

pub fn request_cancel(request: i32) -> i32 {
    unsafe { core_raw::request_cancel(request) }
}

pub fn player_transfer(player: Player, host: &str, port: i32) -> i32 {
    let (h, hl) = p(host);
    unsafe { raw::player_transfer(player, h, hl, port) }
}

pub fn players_by_uuid(uuid: &str) -> Player {
    let (u, ul) = p(uuid);
    unsafe { raw::players_by_uuid(u, ul) }
}

/// Players on the proxy, or on `server` when it is given.
pub fn players_count(server: Option<&str>) -> i32 {
    let (s, sl) = p(server.unwrap_or(""));
    unsafe { raw::players_count(s, sl) }
}

pub fn players_broadcast(text: &str, args: &Args) -> i32 {
    let (t, tl) = p(text);
    with_args(args, |a, al| unsafe { raw::players_broadcast(t, tl, a, al) })
}

pub fn player_address(player: Player) -> Option<String> {
    fetch(|out, cap| unsafe { raw::player_address(player, out, cap) })
}

/// The client port, a negative code when unknown.
pub fn player_port(player: Player) -> i32 {
    unsafe { raw::player_port(player) }
}

/// The client settings map; Err(CallError::Absent) until the client sent it.
pub fn player_settings(player: Player) -> Result<OwnedValue, CallError> {
    call_value(256, |out, cap| unsafe { raw::player_settings(player, out, cap) })
}

pub fn player_data_get_f64(player: Player, key: &str, fallback: f64) -> f64 {
    let (k, kl) = p(key);
    unsafe { raw::player_data_get_f64(player, k, kl, fallback) }
}

pub fn player_data_set_f64(player: Player, key: &str, value: f64) -> i32 {
    let (k, kl) = p(key);
    unsafe { raw::player_data_set_f64(player, k, kl, value) }
}
