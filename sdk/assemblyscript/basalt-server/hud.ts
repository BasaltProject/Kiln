// HUD area (abi/areas/hud.h): scoreboards, boss bars, holograms, NPCs, tab and chat surfaces. Specs are encoded values;
// an out buffer receives an encoded value (UTF-8 text for team and display lookups).
import { Handler, tokenFor } from "../basalt-internal/index";
import { raw_hud as raw } from "./raw/hud";

export function hudScoreboardMain(): i32 {
  return raw.hud_scoreboard_main();
}

export function hudScoreboardCreate(): i32 {
  return raw.hud_scoreboard_create();
}

export function hudScoreboardRelease(board: i32): i32 {
  return raw.hud_scoreboard_release(board);
}

export function hudScoreboardList(board: i32, what: string, out: Uint8Array): i32 {
  const whatB = String.UTF8.encode(what);
  return raw.hud_scoreboard_list(board, changetype<usize>(whatB), whatB.byteLength, out.dataStart, out.length);
}

export function hudObjectiveAdd(board: i32, name: string, spec: Uint8Array): i32 {
  const nameB = String.UTF8.encode(name);
  return raw.hud_objective_add(board, changetype<usize>(nameB), nameB.byteLength, spec.dataStart, spec.length);
}

export function hudObjectiveRemove(board: i32, name: string): i32 {
  const nameB = String.UTF8.encode(name);
  return raw.hud_objective_remove(board, changetype<usize>(nameB), nameB.byteLength);
}

export function hudDisplaySet(board: i32, slot: string, objective: string): i32 {
  const slotB = String.UTF8.encode(slot);
  const objectiveB = String.UTF8.encode(objective);
  return raw.hud_display_set(board, changetype<usize>(slotB), slotB.byteLength, changetype<usize>(objectiveB), objectiveB.byteLength);
}

export function hudDisplayGet(board: i32, slot: string, out: Uint8Array): i32 {
  const slotB = String.UTF8.encode(slot);
  return raw.hud_display_get(board, changetype<usize>(slotB), slotB.byteLength, out.dataStart, out.length);
}

export function hudScoreGet(board: i32, entry: string, objective: string, out: Int32Array): i32 {
  const entryB = String.UTF8.encode(entry);
  const objectiveB = String.UTF8.encode(objective);
  return raw.hud_score_get(board, changetype<usize>(entryB), entryB.byteLength, changetype<usize>(objectiveB), objectiveB.byteLength, out.dataStart);
}

export function hudScoreSet(board: i32, entry: string, objective: string, value: i32): i32 {
  const entryB = String.UTF8.encode(entry);
  const objectiveB = String.UTF8.encode(objective);
  return raw.hud_score_set(board, changetype<usize>(entryB), entryB.byteLength, changetype<usize>(objectiveB), objectiveB.byteLength, value);
}

export function hudScoreReset(board: i32, entry: string, objective: string): i32 {
  const entryB = String.UTF8.encode(entry);
  const objectiveB = String.UTF8.encode(objective);
  return raw.hud_score_reset(board, changetype<usize>(entryB), entryB.byteLength, changetype<usize>(objectiveB), objectiveB.byteLength);
}

export function hudTeamAdd(board: i32, name: string): i32 {
  const nameB = String.UTF8.encode(name);
  return raw.hud_team_add(board, changetype<usize>(nameB), nameB.byteLength);
}

export function hudTeamSet(board: i32, name: string, spec: Uint8Array): i32 {
  const nameB = String.UTF8.encode(name);
  return raw.hud_team_set(board, changetype<usize>(nameB), nameB.byteLength, spec.dataStart, spec.length);
}

export function hudTeamRemove(board: i32, name: string): i32 {
  const nameB = String.UTF8.encode(name);
  return raw.hud_team_remove(board, changetype<usize>(nameB), nameB.byteLength);
}

export function hudTeamJoin(board: i32, team: string, entry: string): i32 {
  const teamB = String.UTF8.encode(team);
  const entryB = String.UTF8.encode(entry);
  return raw.hud_team_join(board, changetype<usize>(teamB), teamB.byteLength, changetype<usize>(entryB), entryB.byteLength);
}

export function hudTeamLeave(board: i32, team: string, entry: string): i32 {
  const teamB = String.UTF8.encode(team);
  const entryB = String.UTF8.encode(entry);
  return raw.hud_team_leave(board, changetype<usize>(teamB), teamB.byteLength, changetype<usize>(entryB), entryB.byteLength);
}

export function hudTeamOf(board: i32, entry: string, out: Uint8Array): i32 {
  const entryB = String.UTF8.encode(entry);
  return raw.hud_team_of(board, changetype<usize>(entryB), entryB.byteLength, out.dataStart, out.length);
}

export function hudScoreboardShow(player: i32, board: i32): i32 {
  return raw.hud_scoreboard_show(player, board);
}

export function hudScoreboardShown(player: i32): i32 {
  return raw.hud_scoreboard_shown(player);
}

export function hudBossbarCreate(spec: Uint8Array): i32 {
  return raw.hud_bossbar_create(spec.dataStart, spec.length);
}

export function hudBossbarAttach(id: string): i32 {
  const idB = String.UTF8.encode(id);
  return raw.hud_bossbar_attach(changetype<usize>(idB), idB.byteLength);
}

export function hudBossbarUpdate(bar: i32, spec: Uint8Array): i32 {
  return raw.hud_bossbar_update(bar, spec.dataStart, spec.length);
}

export function hudBossbarRemove(bar: i32): i32 {
  return raw.hud_bossbar_remove(bar);
}

export function hudBossbarPlayerAdd(bar: i32, player: i32): i32 {
  return raw.hud_bossbar_player_add(bar, player);
}

export function hudBossbarPlayerRemove(bar: i32, player: i32): i32 {
  return raw.hud_bossbar_player_remove(bar, player);
}

export function hudBossbarPlayers(bar: i32, out: Uint8Array): i32 {
  return raw.hud_bossbar_players(bar, out.dataStart, out.length);
}

export function hudBossbarGet(bar: i32, out: Uint8Array): i32 {
  return raw.hud_bossbar_get(bar, out.dataStart, out.length);
}

export function hudBossbarKept(out: Uint8Array): i32 {
  return raw.hud_bossbar_kept(out.dataStart, out.length);
}

export function hudHologramCreate(world: string, text: string, x: f64, y: f64, z: f64): i32 {
  const worldB = String.UTF8.encode(world);
  const textB = String.UTF8.encode(text);
  return raw.hud_hologram_create(changetype<usize>(worldB), worldB.byteLength, changetype<usize>(textB), textB.byteLength, x, y, z);
}

export function hudHologramSet(hologram: i32, spec: Uint8Array): i32 {
  return raw.hud_hologram_set(hologram, spec.dataStart, spec.length);
}

export function hudHologramGet(hologram: i32, out: Uint8Array): i32 {
  return raw.hud_hologram_get(hologram, out.dataStart, out.length);
}

export function hudHologramRemove(hologram: i32): i32 {
  return raw.hud_hologram_remove(hologram);
}

export function hudNpcCreate(spec: Uint8Array, handler: Handler | null): i32 {
  return raw.hud_npc_create(spec.dataStart, spec.length, tokenFor(handler));
}

export function hudNpcSet(npc: i32, spec: Uint8Array): i32 {
  return raw.hud_npc_set(npc, spec.dataStart, spec.length);
}

export function hudNpcGet(npc: i32, out: Uint8Array): i32 {
  return raw.hud_npc_get(npc, out.dataStart, out.length);
}

export function hudNpcItem(npc: i32, slot: i32, out: Uint8Array): i32 {
  return raw.hud_npc_item(npc, slot, out.dataStart, out.length);
}

export function hudNpcAttribute(npc: i32, name: string, out: Float64Array): i32 {
  const nameB = String.UTF8.encode(name);
  return raw.hud_npc_attribute(npc, changetype<usize>(nameB), nameB.byteLength, out.dataStart);
}

export function hudNpcAnimate(npc: i32, action: i32): i32 {
  return raw.hud_npc_animate(npc, action);
}

export function hudNpcRemove(npc: i32): i32 {
  return raw.hud_npc_remove(npc);
}

export function hudPlayerSet(player: i32, spec: Uint8Array): i32 {
  return raw.hud_player_set(player, spec.dataStart, spec.length);
}

export function hudPlayerGet(player: i32, out: Uint8Array): i32 {
  return raw.hud_player_get(player, out.dataStart, out.length);
}

export function hudTabEntry(viewer: i32, entity_id: i32, listed: i32): i32 {
  return raw.hud_tab_entry(viewer, entity_id, listed);
}

export function hudSurfaceClaim(surface: string, priority: i32, spec: Uint8Array, handler: Handler | null): i32 {
  const surfaceB = String.UTF8.encode(surface);
  return raw.hud_surface_claim(changetype<usize>(surfaceB), surfaceB.byteLength, priority, spec.dataStart, spec.length, tokenFor(handler));
}

export function hudSurfaceUpdate(claim: i32, spec: Uint8Array): i32 {
  return raw.hud_surface_update(claim, spec.dataStart, spec.length);
}

export function hudSurfaceRelease(claim: i32): i32 {
  return raw.hud_surface_release(claim);
}

export function hudSurfaceAnswer(ctx: i32, spec: Uint8Array): i32 {
  return raw.hud_surface_answer(ctx, spec.dataStart, spec.length);
}
