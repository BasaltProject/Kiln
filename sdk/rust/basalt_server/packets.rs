//! Plugin channels and raw packets (abi/areas/packets.h). Calls return the import result: a count or handle, or a
//! negative ABI code. The out-calls fill your buffer and return the length the full answer needs; open the slice with
//! `values::Value::open` for the value ones.
#![allow(dead_code)]

use crate::raw::packets as raw;
use crate::values::Writer;
use crate::core::{token_for, Ctx, Player};

fn p(text: &str) -> (*const u8, i32) {
    (text.as_ptr(), text.len() as i32)
}

fn b(bytes: &[u8]) -> (*const u8, i32) {
    (bytes.as_ptr(), bytes.len() as i32)
}

pub fn channel_listen(name: &str, handler: impl FnMut(Ctx) + 'static) -> i32 {
    let (np, nl) = p(name);
    unsafe { raw::channel_listen(np, nl, token_for(Box::new(handler))) }
}

pub fn channel_send(player: Player, name: &str, body: &[u8]) -> i32 {
    let (np, nl) = p(name);
    let (bp, bl) = b(body);
    unsafe { raw::channel_send(player, np, nl, bp, bl) }
}

/// `priority` is "first", "early", "normal", "late", "last" or None for the default.
pub fn packet_listen(pattern: &str, priority: Option<&str>, observe: bool, handler: impl FnMut(Ctx) + 'static) -> i32 {
    let mut document = Vec::new();
    if priority.is_some() || observe {
        let mut writer = Writer::new();
        writer.begin_map();
        if let Some(text) = priority {
            writer.key("priority").string(text);
        }
        if observe {
            writer.key("observe").boolean(true);
        }
        writer.end();
        match writer.into_vec() {
            Ok(bytes) => document = bytes,
            Err(_) => return -3,
        }
    }
    let (pp, pl) = p(pattern);
    let (dp, dl) = b(&document);
    unsafe { raw::packet_listen(pp, pl, dp, dl, token_for(Box::new(handler))) }
}

pub fn packet_unlisten(listener: i32) -> i32 {
    unsafe { raw::packet_unlisten(listener) }
}

pub fn packet_send(player: Player, name: &str, body: &[u8], intercept: bool) -> i32 {
    let (np, nl) = p(name);
    let (bp, bl) = b(body);
    unsafe { raw::packet_send(player, np, nl, bp, bl, intercept as i32) }
}

pub fn packet_receive(player: Player, name: &str, body: &[u8], intercept: bool) -> i32 {
    let (np, nl) = p(name);
    let (bp, bl) = b(body);
    unsafe { raw::packet_receive(player, np, nl, bp, bl, intercept as i32) }
}

pub fn packet_lookup(name: &str, out: &mut [u8]) -> i32 {
    let (np, nl) = p(name);
    unsafe { raw::packet_lookup(np, nl, out.as_mut_ptr(), out.len() as i32) }
}

pub fn packet_names(prefix: &str, out: &mut [u8]) -> i32 {
    let (pp, pl) = p(prefix);
    unsafe { raw::packet_names(pp, pl, out.as_mut_ptr(), out.len() as i32) }
}

pub fn packet_player_actions(out: &mut [u8]) -> i32 {
    unsafe { raw::packet_player_actions(out.as_mut_ptr(), out.len() as i32) }
}

/// `args` is an encoded list value.
pub fn packet_player_receive(player: Player, action: &str, args: &[u8]) -> i32 {
    let (ap, al) = p(action);
    let (gp, gl) = b(args);
    unsafe { raw::packet_player_receive(player, ap, al, gp, gl) }
}

pub fn packet_ctx_info(ctx: Ctx, out: &mut [u8]) -> i32 {
    unsafe { raw::packet_ctx_info(ctx, out.as_mut_ptr(), out.len() as i32) }
}

pub fn packet_ctx_body(ctx: Ctx, out: &mut [u8]) -> i32 {
    unsafe { raw::packet_ctx_body(ctx, out.as_mut_ptr(), out.len() as i32) }
}

pub fn packet_ctx_player(ctx: Ctx) -> Player {
    unsafe { raw::packet_ctx_player(ctx) }
}

pub fn packet_ctx_cancel(ctx: Ctx) -> i32 {
    unsafe { raw::packet_ctx_cancel(ctx) }
}

pub fn packet_ctx_set_body(ctx: Ctx, body: &[u8]) -> i32 {
    let (bp, bl) = b(body);
    unsafe { raw::packet_ctx_set_body(ctx, bp, bl) }
}

pub fn packet_ctx_send(ctx: Ctx, name: &str, body: &[u8], intercept: bool) -> i32 {
    let (np, nl) = p(name);
    let (bp, bl) = b(body);
    unsafe { raw::packet_ctx_send(ctx, np, nl, bp, bl, intercept as i32) }
}

pub fn packet_ctx_receive(ctx: Ctx, name: &str, body: &[u8], intercept: bool) -> i32 {
    let (np, nl) = p(name);
    let (bp, bl) = b(body);
    unsafe { raw::packet_ctx_receive(ctx, np, nl, bp, bl, intercept as i32) }
}
