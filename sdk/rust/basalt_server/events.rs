#[path = "../common/events.rs"]
mod shared;
pub use shared::*;
use crate::raw::core_values as core_values;
use crate::raw::events as raw;
use crate::values::{call_out, call_value, CallError, OwnedValue};
use crate::core::{token_for, with_args, Args, Ctx, Handler, Player};

pub const POST: i32 = 1;
pub const CANCELLED: i32 = 2;

fn p(text: &str) -> (*const u8, i32) {
    (text.as_ptr(), text.len() as i32)
}

fn b(data: &[u8]) -> (*const u8, i32) {
    (data.as_ptr(), data.len() as i32)
}

fn text(capacity: usize, call: impl FnMut(*mut u8, i32) -> i32) -> Option<String> {
    String::from_utf8(call_out(capacity, call).ok()?).ok()
}

/// Listens to `name`; `filters` is a finished map document or empty.
pub fn on_ex(name: &str, priority: i32, flags: i32, filters: &[u8], handler: impl FnMut(Ctx) + 'static) -> i32 {
    let (n, nl) = p(name);
    let (f, fl) = b(filters);
    let boxed: Handler = Box::new(handler);
    unsafe { raw::event_on_ex(n, nl, priority, flags, f, fl, token_for(boxed)) }
}

pub fn event_name(ctx: Ctx) -> Option<String> {
    text(64, |out, cap| unsafe { raw::event_name(ctx, out, cap) })
}

pub fn event_get(ctx: Ctx, field: &str, initial: usize) -> Result<OwnedValue, CallError> {
    let (f, fl) = p(field);
    call_value(initial, |out, cap| unsafe { raw::event_get(ctx, f, fl, out, cap) })
}

pub fn event_set(ctx: Ctx, field: &str, value: &[u8]) -> i32 {
    let (f, fl) = p(field);
    let (v, vl) = b(value);
    unsafe { raw::event_set(ctx, f, fl, v, vl) }
}

pub fn event_fields(ctx: Ctx, initial: usize) -> Result<OwnedValue, CallError> {
    call_value(initial, |out, cap| unsafe { raw::event_fields(ctx, out, cap) })
}

pub fn event_payload(ctx: Ctx, initial: usize) -> Result<OwnedValue, CallError> {
    call_value(initial, |out, cap| unsafe { raw::event_payload(ctx, out, cap) })
}

pub fn event_describe(name: &str, initial: usize) -> Result<OwnedValue, CallError> {
    let (n, nl) = p(name);
    call_value(initial, |out, cap| unsafe { raw::event_describe(n, nl, out, cap) })
}

pub fn event_names(initial: usize) -> Result<OwnedValue, CallError> {
    call_value(initial, |out, cap| unsafe { raw::event_names(out, cap) })
}

pub fn event_cancelled(ctx: Ctx) -> i32 {
    unsafe { raw::event_cancelled(ctx) }
}

pub fn event_cancel_with(ctx: Ctx, message: &str, args: &Args) -> i32 {
    let (t, tl) = p(message);
    with_args(args, |a, al| unsafe { raw::event_cancel_with(ctx, t, tl, a, al) })
}

pub fn event_cancel_message(ctx: Ctx) -> Option<String> {
    text(256, |out, cap| unsafe { raw::event_cancel_message(ctx, out, cap) })
}

pub fn event_is_post(ctx: Ctx) -> i32 {
    unsafe { raw::event_is_post(ctx) }
}

pub fn event_emit(name: &str, value: &[u8]) -> i32 {
    let (n, nl) = p(name);
    let (v, vl) = b(value);
    unsafe { raw::event_emit(n, nl, v, vl) }
}

pub fn player_by_uuid(uuid: &str) -> Player {
    let (u, ul) = p(uuid);
    unsafe { raw::player_by_uuid(u, ul) }
}

pub fn player_uuid(player: Player) -> Option<String> {
    text(64, |out, cap| unsafe { raw::player_uuid(player, out, cap) })
}

pub fn player_get(player: Player, name: &str, initial: usize) -> Result<OwnedValue, CallError> {
    let (n, nl) = p(name);
    call_value(initial, |out, cap| unsafe { raw::player_get(player, n, nl, out, cap) })
}

pub fn player_set(player: Player, name: &str, value: &[u8]) -> i32 {
    let (n, nl) = p(name);
    let (v, vl) = b(value);
    unsafe { raw::player_set(player, n, nl, v, vl) }
}

/// Runs a named operation; `args` is a finished list document or empty.
pub fn player_call(player: Player, op: &str, args: &[u8], initial: usize) -> Result<OwnedValue, CallError> {
    let (o, ol) = p(op);
    let (a, al) = b(args);
    call_value(initial, |out, cap| unsafe { raw::player_call(player, o, ol, a, al, out, cap) })
}

pub fn player_describe(initial: usize) -> Result<OwnedValue, CallError> {
    call_value(initial, |out, cap| unsafe { raw::player_describe(out, cap) })
}

pub fn player_health(player: Player) -> Option<f64> {
    let mut health = 0.0f64;
    if unsafe { raw::player_health(player, &mut health) } < 0 {
        None
    } else {
        Some(health)
    }
}

pub fn player_set_health(player: Player, health: f64) -> i32 {
    unsafe { raw::player_set_health(player, health) }
}

pub fn player_gamemode(player: Player) -> Option<String> {
    text(16, |out, cap| unsafe { raw::player_gamemode(player, out, cap) })
}

pub fn player_set_gamemode(player: Player, mode: &str) -> i32 {
    let (m, ml) = p(mode);
    unsafe { raw::player_set_gamemode(player, m, ml) }
}

pub fn player_teleport(player: Player, world: &str, x: f64, y: f64, z: f64, yaw: f32, pitch: f32) -> i32 {
    let (w, wl) = p(world);
    unsafe { raw::player_teleport(player, w, wl, x, y, z, yaw, pitch) }
}

pub fn player_title(player: Player, title: &str, subtitle: &str, fade_in: i32, stay: i32, fade_out: i32, args: &Args) -> i32 {
    let (t, tl) = p(title);
    let (s, sl) = p(subtitle);
    with_args(args, |a, al| unsafe { raw::player_title(player, t, tl, s, sl, fade_in, stay, fade_out, a, al) })
}

pub fn player_kick(player: Player, message: &str, args: &Args) -> i32 {
    let (t, tl) = p(message);
    with_args(args, |a, al| unsafe { raw::player_kick(player, t, tl, a, al) })
}

pub fn player_effect(player: Player, effect: &str, ticks: i32, amplifier: i32) -> i32 {
    let (e, el) = p(effect);
    unsafe { raw::player_effect(player, e, el, ticks, amplifier) }
}

pub fn player_open_inventory(player: Player, kind: &str, spec: &[u8]) -> i32 {
    let (k, kl) = p(kind);
    let (s, sl) = b(spec);
    unsafe { raw::player_open_inventory(player, k, kl, s, sl) }
}

pub fn player_close_window(player: Player) -> i32 {
    unsafe { raw::player_close_window(player) }
}

/// Shows a form (a finished document); the handler runs once with the answer, read with `completion_value`.
pub fn form_open(player: Player, form: &[u8], handler: impl FnMut(Ctx) + 'static) -> i32 {
    let (f, fl) = b(form);
    let boxed: Handler = Box::new(handler);
    unsafe { raw::form_open(player, f, fl, token_for(boxed)) }
}

pub fn form_close(player: Player) -> i32 {
    unsafe { raw::form_close(player) }
}

pub fn dialog_show(player: Player, dialog: &str, handler: impl FnMut(Ctx) + 'static) -> i32 {
    let (d, dl) = p(dialog);
    let boxed: Handler = Box::new(handler);
    unsafe { raw::dialog_show(player, d, dl, token_for(boxed)) }
}

pub fn dialog_clear(player: Player) -> i32 {
    unsafe { raw::dialog_clear(player) }
}

pub fn completion_value(ctx: Ctx, initial: usize) -> Result<OwnedValue, CallError> {
    call_value(initial, |out, cap| unsafe { core_values::value_result(ctx, out, cap) })
}
