//! HUD area (abi/areas/hud.h): scoreboards, boss bars, holograms, NPCs, tab and chat surfaces. Specs are encoded values
//! built with `values::Writer`; an `out` buffer receives an encoded value (UTF-8 text for team and display lookups).
#![allow(dead_code)]

use crate::raw::hud as raw;
use crate::core::{p, token_for, Ctx};

pub fn scoreboard_main() -> i32 {
    unsafe { raw::hud_scoreboard_main() }
}

pub fn scoreboard_create() -> i32 {
    unsafe { raw::hud_scoreboard_create() }
}

pub fn scoreboard_release(board: i32) -> i32 {
    unsafe { raw::hud_scoreboard_release(board) }
}

pub fn scoreboard_list(board: i32, what: &str, out: &mut [u8]) -> i32 {
    let (what_ptr, what_len) = p(what);
    unsafe { raw::hud_scoreboard_list(board, what_ptr, what_len, out.as_mut_ptr(), out.len() as i32) }
}

pub fn objective_add(board: i32, name: &str, spec: &[u8]) -> i32 {
    let (name_ptr, name_len) = p(name);
    unsafe { raw::hud_objective_add(board, name_ptr, name_len, spec.as_ptr(), spec.len() as i32) }
}

pub fn objective_remove(board: i32, name: &str) -> i32 {
    let (name_ptr, name_len) = p(name);
    unsafe { raw::hud_objective_remove(board, name_ptr, name_len) }
}

pub fn display_set(board: i32, slot: &str, objective: &str) -> i32 {
    let (slot_ptr, slot_len) = p(slot);
    let (objective_ptr, objective_len) = p(objective);
    unsafe { raw::hud_display_set(board, slot_ptr, slot_len, objective_ptr, objective_len) }
}

pub fn display_get(board: i32, slot: &str, out: &mut [u8]) -> i32 {
    let (slot_ptr, slot_len) = p(slot);
    unsafe { raw::hud_display_get(board, slot_ptr, slot_len, out.as_mut_ptr(), out.len() as i32) }
}

pub fn score_get(board: i32, entry: &str, objective: &str, out: &mut i32) -> i32 {
    let (entry_ptr, entry_len) = p(entry);
    let (objective_ptr, objective_len) = p(objective);
    unsafe { raw::hud_score_get(board, entry_ptr, entry_len, objective_ptr, objective_len, out) }
}

pub fn score_set(board: i32, entry: &str, objective: &str, value: i32) -> i32 {
    let (entry_ptr, entry_len) = p(entry);
    let (objective_ptr, objective_len) = p(objective);
    unsafe { raw::hud_score_set(board, entry_ptr, entry_len, objective_ptr, objective_len, value) }
}

pub fn score_reset(board: i32, entry: &str, objective: &str) -> i32 {
    let (entry_ptr, entry_len) = p(entry);
    let (objective_ptr, objective_len) = p(objective);
    unsafe { raw::hud_score_reset(board, entry_ptr, entry_len, objective_ptr, objective_len) }
}

pub fn team_add(board: i32, name: &str) -> i32 {
    let (name_ptr, name_len) = p(name);
    unsafe { raw::hud_team_add(board, name_ptr, name_len) }
}

pub fn team_set(board: i32, name: &str, spec: &[u8]) -> i32 {
    let (name_ptr, name_len) = p(name);
    unsafe { raw::hud_team_set(board, name_ptr, name_len, spec.as_ptr(), spec.len() as i32) }
}

pub fn team_remove(board: i32, name: &str) -> i32 {
    let (name_ptr, name_len) = p(name);
    unsafe { raw::hud_team_remove(board, name_ptr, name_len) }
}

pub fn team_join(board: i32, team: &str, entry: &str) -> i32 {
    let (team_ptr, team_len) = p(team);
    let (entry_ptr, entry_len) = p(entry);
    unsafe { raw::hud_team_join(board, team_ptr, team_len, entry_ptr, entry_len) }
}

pub fn team_leave(board: i32, team: &str, entry: &str) -> i32 {
    let (team_ptr, team_len) = p(team);
    let (entry_ptr, entry_len) = p(entry);
    unsafe { raw::hud_team_leave(board, team_ptr, team_len, entry_ptr, entry_len) }
}

pub fn team_of(board: i32, entry: &str, out: &mut [u8]) -> i32 {
    let (entry_ptr, entry_len) = p(entry);
    unsafe { raw::hud_team_of(board, entry_ptr, entry_len, out.as_mut_ptr(), out.len() as i32) }
}

pub fn scoreboard_show(player: i32, board: i32) -> i32 {
    unsafe { raw::hud_scoreboard_show(player, board) }
}

pub fn scoreboard_shown(player: i32) -> i32 {
    unsafe { raw::hud_scoreboard_shown(player) }
}

pub fn bossbar_create(spec: &[u8]) -> i32 {
    unsafe { raw::hud_bossbar_create(spec.as_ptr(), spec.len() as i32) }
}

pub fn bossbar_attach(id: &str) -> i32 {
    let (id_ptr, id_len) = p(id);
    unsafe { raw::hud_bossbar_attach(id_ptr, id_len) }
}

pub fn bossbar_update(bar: i32, spec: &[u8]) -> i32 {
    unsafe { raw::hud_bossbar_update(bar, spec.as_ptr(), spec.len() as i32) }
}

pub fn bossbar_remove(bar: i32) -> i32 {
    unsafe { raw::hud_bossbar_remove(bar) }
}

pub fn bossbar_player_add(bar: i32, player: i32) -> i32 {
    unsafe { raw::hud_bossbar_player_add(bar, player) }
}

pub fn bossbar_player_remove(bar: i32, player: i32) -> i32 {
    unsafe { raw::hud_bossbar_player_remove(bar, player) }
}

pub fn bossbar_players(bar: i32, out: &mut [u8]) -> i32 {
    unsafe { raw::hud_bossbar_players(bar, out.as_mut_ptr(), out.len() as i32) }
}

pub fn bossbar_get(bar: i32, out: &mut [u8]) -> i32 {
    unsafe { raw::hud_bossbar_get(bar, out.as_mut_ptr(), out.len() as i32) }
}

pub fn bossbar_kept(out: &mut [u8]) -> i32 {
    unsafe { raw::hud_bossbar_kept(out.as_mut_ptr(), out.len() as i32) }
}

pub fn hologram_create(world: &str, text: &str, x: f64, y: f64, z: f64) -> i32 {
    let (world_ptr, world_len) = p(world);
    let (text_ptr, text_len) = p(text);
    unsafe { raw::hud_hologram_create(world_ptr, world_len, text_ptr, text_len, x, y, z) }
}

pub fn hologram_set(hologram: i32, spec: &[u8]) -> i32 {
    unsafe { raw::hud_hologram_set(hologram, spec.as_ptr(), spec.len() as i32) }
}

pub fn hologram_get(hologram: i32, out: &mut [u8]) -> i32 {
    unsafe { raw::hud_hologram_get(hologram, out.as_mut_ptr(), out.len() as i32) }
}

pub fn hologram_remove(hologram: i32) -> i32 {
    unsafe { raw::hud_hologram_remove(hologram) }
}

pub fn npc_create(spec: &[u8], handler: Option<fn(Ctx)>) -> i32 {
    unsafe { raw::hud_npc_create(spec.as_ptr(), spec.len() as i32, handler.map_or(0, |h| token_for(Box::new(h)))) }
}

pub fn npc_set(npc: i32, spec: &[u8]) -> i32 {
    unsafe { raw::hud_npc_set(npc, spec.as_ptr(), spec.len() as i32) }
}

pub fn npc_get(npc: i32, out: &mut [u8]) -> i32 {
    unsafe { raw::hud_npc_get(npc, out.as_mut_ptr(), out.len() as i32) }
}

pub fn npc_item(npc: i32, slot: i32, out: &mut [u8]) -> i32 {
    unsafe { raw::hud_npc_item(npc, slot, out.as_mut_ptr(), out.len() as i32) }
}

pub fn npc_attribute(npc: i32, name: &str, out: &mut f64) -> i32 {
    let (name_ptr, name_len) = p(name);
    unsafe { raw::hud_npc_attribute(npc, name_ptr, name_len, out) }
}

pub fn npc_animate(npc: i32, action: i32) -> i32 {
    unsafe { raw::hud_npc_animate(npc, action) }
}

pub fn npc_remove(npc: i32) -> i32 {
    unsafe { raw::hud_npc_remove(npc) }
}

pub fn player_set(player: i32, spec: &[u8]) -> i32 {
    unsafe { raw::hud_player_set(player, spec.as_ptr(), spec.len() as i32) }
}

pub fn player_get(player: i32, out: &mut [u8]) -> i32 {
    unsafe { raw::hud_player_get(player, out.as_mut_ptr(), out.len() as i32) }
}

pub fn tab_entry(viewer: i32, entity_id: i32, listed: i32) -> i32 {
    unsafe { raw::hud_tab_entry(viewer, entity_id, listed) }
}

pub fn surface_claim(surface: &str, priority: i32, spec: &[u8], handler: Option<fn(Ctx)>) -> i32 {
    let (surface_ptr, surface_len) = p(surface);
    unsafe { raw::hud_surface_claim(surface_ptr, surface_len, priority, spec.as_ptr(), spec.len() as i32, handler.map_or(0, |h| token_for(Box::new(h)))) }
}

pub fn surface_update(claim: i32, spec: &[u8]) -> i32 {
    unsafe { raw::hud_surface_update(claim, spec.as_ptr(), spec.len() as i32) }
}

pub fn surface_release(claim: i32) -> i32 {
    unsafe { raw::hud_surface_release(claim) }
}

pub fn surface_answer(ctx: i32, spec: &[u8]) -> i32 {
    unsafe { raw::hud_surface_answer(ctx, spec.as_ptr(), spec.len() as i32) }
}
