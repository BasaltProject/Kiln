use crate::core::*;

use crate::raw::network as raw;
use crate::core::{p, token_for, Ctx, Player};

fn fetch(call: impl Fn(*mut u8, i32) -> i32) -> Result<Vec<u8>, i32> {
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

pub fn channel_register(name: &str) -> i32 {
    let (n, nl) = p(name);
    unsafe { raw::network_channel_register(n, nl) }
}

pub fn channel_unregister(name: &str) -> i32 {
    let (n, nl) = p(name);
    unsafe { raw::network_channel_unregister(n, nl) }
}

/// Observes the messages of the plugin's channels; an empty filter takes all of them.
pub fn message_on(filter: &str, handler: impl FnMut(Ctx) + 'static) -> i32 {
    let (f, fl) = p(filter);
    unsafe { raw::network_message_on(f, fl, token_for(Box::new(handler))) }
}

/// The encoded map of the message (player_name, player_uuid, channel, data, source, server).
pub fn message_get(ctx: Ctx) -> Result<Vec<u8>, i32> {
    fetch(|out, cap| unsafe { raw::network_message_get(ctx, out, cap) })
}

/// `toward_client` false sends to the player's backend, true to the client.
pub fn message_send(player: Player, channel: &str, data: &[u8], toward_client: bool) -> i32 {
    let (c, cl) = p(channel);
    unsafe { raw::network_message_send(player, c, cl, data.as_ptr(), data.len() as i32, toward_client as i32) }
}

pub fn packet_resolve(name: &str) -> Result<Vec<u8>, i32> {
    let (n, nl) = p(name);
    fetch(|out, cap| unsafe { raw::packet_resolve(n, nl, out, cap) })
}

pub fn packet_names() -> Result<Vec<u8>, i32> {
    fetch(|out, cap| unsafe { raw::packet_names(out, cap) })
}

/// A decider listener; `opts` is an encoded value map (priority, fail) or empty. Returns the listener.
pub fn packet_on(name: &str, opts: &[u8], handler: impl FnMut(Ctx) + 'static) -> i32 {
    let (n, nl) = p(name);
    unsafe { raw::packet_on(n, nl, opts.as_ptr(), opts.len() as i32, token_for(Box::new(handler))) }
}

/// An observer listener: it sees the final packet and cannot cancel or change it.
pub fn packet_watch(name: &str, opts: &[u8], handler: impl FnMut(Ctx) + 'static) -> i32 {
    let (n, nl) = p(name);
    unsafe { raw::packet_watch(n, nl, opts.as_ptr(), opts.len() as i32, token_for(Box::new(handler))) }
}

pub fn packet_remove(listener: i32) -> i32 {
    unsafe { raw::packet_remove(listener) }
}

pub fn packet_get(ctx: Ctx) -> Result<Vec<u8>, i32> {
    fetch(|out, cap| unsafe { raw::packet_get(ctx, out, cap) })
}

pub fn packet_body(ctx: Ctx) -> Result<Vec<u8>, i32> {
    fetch(|out, cap| unsafe { raw::packet_body(ctx, out, cap) })
}

pub fn packet_set_body(ctx: Ctx, body: &[u8]) -> i32 {
    unsafe { raw::packet_set_body(ctx, body.as_ptr(), body.len() as i32) }
}

pub fn packet_cancel(ctx: Ctx) -> i32 {
    unsafe { raw::packet_cancel(ctx) }
}

pub fn packet_inject(ctx: Ctx, name: &str, body: &[u8], intercept: bool) -> i32 {
    let (n, nl) = p(name);
    unsafe { raw::packet_inject(ctx, n, nl, body.as_ptr(), body.len() as i32, intercept as i32) }
}

pub fn packet_send(player: Player, name: &str, body: &[u8]) -> i32 {
    let (n, nl) = p(name);
    unsafe { raw::packet_send(player, n, nl, body.as_ptr(), body.len() as i32) }
}

pub fn packet_receive(player: Player, name: &str, body: &[u8]) -> i32 {
    let (n, nl) = p(name);
    unsafe { raw::packet_receive(player, n, nl, body.as_ptr(), body.len() as i32) }
}

pub fn antibot_verify(name: &str) -> i32 {
    let (n, nl) = p(name);
    unsafe { raw::antibot_verify(n, nl) }
}

pub fn antibot_unverify(name: &str) -> i32 {
    let (n, nl) = p(name);
    unsafe { raw::antibot_unverify(n, nl) }
}

pub fn antibot_is_verified(name: &str) -> i32 {
    let (n, nl) = p(name);
    unsafe { raw::antibot_is_verified(n, nl) }
}

pub fn antibot_ban(address: &str) -> i32 {
    let (a, al) = p(address);
    unsafe { raw::antibot_ban(a, al) }
}

pub fn antibot_unban(address: &str) -> i32 {
    let (a, al) = p(address);
    unsafe { raw::antibot_unban(a, al) }
}

pub fn antibot_under_attack() -> i32 {
    unsafe { raw::antibot_under_attack() }
}

pub fn antibot_on(handler: impl FnMut(Ctx) + 'static) -> i32 {
    unsafe { raw::antibot_on(token_for(Box::new(handler))) }
}

pub fn antibot_event_get(ctx: Ctx) -> Result<Vec<u8>, i32> {
    fetch(|out, cap| unsafe { raw::antibot_event_get(ctx, out, cap) })
}

pub fn servers_list() -> Vec<String> {
    crate::core::fetch(|out, cap| unsafe { crate::raw::servers_list(out, cap) })
        .map(|names| names.split('\n').filter(|name| !name.is_empty()).map(String::from).collect())
        .unwrap_or_default()
}

pub fn server_address(name: &str) -> Option<String> {
    let (n, nl) = p(name);
    crate::core::fetch(|out, cap| unsafe { crate::raw::server_address(n, nl, out, cap) })
}

pub fn players_on(server: &str) -> Vec<Player> {
    let (s, sl) = p(server);
    let mut out = vec![0i32; 64];
    loop {
        let count = unsafe { crate::raw::players_on(s, sl, out.as_mut_ptr(), out.len() as i32) };
        if count < 0 {
            return Vec::new();
        }
        if count as usize <= out.len() {
            out.truncate(count as usize);
            return out;
        }
        out.resize(count as usize, 0);
    }
}
