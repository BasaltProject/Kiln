//! HUD area (abi/areas/hud.h): scoreboards, boss bars, holograms, NPCs, tab and chat surfaces. Specs are encoded values
//! built with `values.Writer`; an `out` buffer receives an encoded value (UTF-8 text for team and display lookups).
const core = @import("core.zig");
const raw = @import("raw/hud.zig").raw;

fn len(s: []const u8) i32 {
    return @intCast(s.len);
}

fn cap(out: []u8) i32 {
    return @intCast(out.len);
}

pub fn scoreboardMain() i32 {
    return raw.hud_scoreboard_main();
}

pub fn scoreboardCreate() i32 {
    return raw.hud_scoreboard_create();
}

pub fn scoreboardRelease(board: i32) i32 {
    return raw.hud_scoreboard_release(board);
}

pub fn scoreboardList(board: i32, what: []const u8, out: []u8) i32 {
    return raw.hud_scoreboard_list(board, what.ptr, len(what), out.ptr, cap(out));
}

pub fn objectiveAdd(board: i32, name: []const u8, spec: []const u8) i32 {
    return raw.hud_objective_add(board, name.ptr, len(name), spec.ptr, len(spec));
}

pub fn objectiveRemove(board: i32, name: []const u8) i32 {
    return raw.hud_objective_remove(board, name.ptr, len(name));
}

pub fn displaySet(board: i32, slot: []const u8, objective: []const u8) i32 {
    return raw.hud_display_set(board, slot.ptr, len(slot), objective.ptr, len(objective));
}

pub fn displayGet(board: i32, slot: []const u8, out: []u8) i32 {
    return raw.hud_display_get(board, slot.ptr, len(slot), out.ptr, cap(out));
}

pub fn scoreGet(board: i32, entry: []const u8, objective: []const u8, out: *i32) i32 {
    return raw.hud_score_get(board, entry.ptr, len(entry), objective.ptr, len(objective), @ptrCast(out));
}

pub fn scoreSet(board: i32, entry: []const u8, objective: []const u8, value: i32) i32 {
    return raw.hud_score_set(board, entry.ptr, len(entry), objective.ptr, len(objective), value);
}

pub fn scoreReset(board: i32, entry: []const u8, objective: []const u8) i32 {
    return raw.hud_score_reset(board, entry.ptr, len(entry), objective.ptr, len(objective));
}

pub fn teamAdd(board: i32, name: []const u8) i32 {
    return raw.hud_team_add(board, name.ptr, len(name));
}

pub fn teamSet(board: i32, name: []const u8, spec: []const u8) i32 {
    return raw.hud_team_set(board, name.ptr, len(name), spec.ptr, len(spec));
}

pub fn teamRemove(board: i32, name: []const u8) i32 {
    return raw.hud_team_remove(board, name.ptr, len(name));
}

pub fn teamJoin(board: i32, team: []const u8, entry: []const u8) i32 {
    return raw.hud_team_join(board, team.ptr, len(team), entry.ptr, len(entry));
}

pub fn teamLeave(board: i32, team: []const u8, entry: []const u8) i32 {
    return raw.hud_team_leave(board, team.ptr, len(team), entry.ptr, len(entry));
}

pub fn teamOf(board: i32, entry: []const u8, out: []u8) i32 {
    return raw.hud_team_of(board, entry.ptr, len(entry), out.ptr, cap(out));
}

pub fn scoreboardShow(player: i32, board: i32) i32 {
    return raw.hud_scoreboard_show(player, board);
}

pub fn scoreboardShown(player: i32) i32 {
    return raw.hud_scoreboard_shown(player);
}

pub fn bossbarCreate(spec: []const u8) i32 {
    return raw.hud_bossbar_create(spec.ptr, len(spec));
}

pub fn bossbarAttach(id: []const u8) i32 {
    return raw.hud_bossbar_attach(id.ptr, len(id));
}

pub fn bossbarUpdate(bar: i32, spec: []const u8) i32 {
    return raw.hud_bossbar_update(bar, spec.ptr, len(spec));
}

pub fn bossbarRemove(bar: i32) i32 {
    return raw.hud_bossbar_remove(bar);
}

pub fn bossbarPlayerAdd(bar: i32, player: i32) i32 {
    return raw.hud_bossbar_player_add(bar, player);
}

pub fn bossbarPlayerRemove(bar: i32, player: i32) i32 {
    return raw.hud_bossbar_player_remove(bar, player);
}

pub fn bossbarPlayers(bar: i32, out: []u8) i32 {
    return raw.hud_bossbar_players(bar, out.ptr, cap(out));
}

pub fn bossbarGet(bar: i32, out: []u8) i32 {
    return raw.hud_bossbar_get(bar, out.ptr, cap(out));
}

pub fn bossbarKept(out: []u8) i32 {
    return raw.hud_bossbar_kept(out.ptr, cap(out));
}

pub fn hologramCreate(world: []const u8, text: []const u8, x: f64, y: f64, z: f64) i32 {
    return raw.hud_hologram_create(world.ptr, len(world), text.ptr, len(text), x, y, z);
}

pub fn hologramSet(hologram: i32, spec: []const u8) i32 {
    return raw.hud_hologram_set(hologram, spec.ptr, len(spec));
}

pub fn hologramGet(hologram: i32, out: []u8) i32 {
    return raw.hud_hologram_get(hologram, out.ptr, cap(out));
}

pub fn hologramRemove(hologram: i32) i32 {
    return raw.hud_hologram_remove(hologram);
}

pub fn npcCreate(spec: []const u8, handler: ?core.Handler) i32 {
    const token = core.tokenFor(handler);
    if (token < 0) return -3;
    return raw.hud_npc_create(spec.ptr, len(spec), token);
}

pub fn npcSet(npc: i32, spec: []const u8) i32 {
    return raw.hud_npc_set(npc, spec.ptr, len(spec));
}

pub fn npcGet(npc: i32, out: []u8) i32 {
    return raw.hud_npc_get(npc, out.ptr, cap(out));
}

pub fn npcItem(npc: i32, slot: i32, out: []u8) i32 {
    return raw.hud_npc_item(npc, slot, out.ptr, cap(out));
}

pub fn npcAttribute(npc: i32, name: []const u8, out: *f64) i32 {
    return raw.hud_npc_attribute(npc, name.ptr, len(name), @ptrCast(out));
}

pub fn npcAnimate(npc: i32, action: i32) i32 {
    return raw.hud_npc_animate(npc, action);
}

pub fn npcRemove(npc: i32) i32 {
    return raw.hud_npc_remove(npc);
}

pub fn playerSet(player: i32, spec: []const u8) i32 {
    return raw.hud_player_set(player, spec.ptr, len(spec));
}

pub fn playerGet(player: i32, out: []u8) i32 {
    return raw.hud_player_get(player, out.ptr, cap(out));
}

pub fn tabEntry(viewer: i32, entity_id: i32, listed: i32) i32 {
    return raw.hud_tab_entry(viewer, entity_id, listed);
}

pub fn surfaceClaim(surface: []const u8, priority: i32, spec: []const u8, handler: ?core.Handler) i32 {
    const token = core.tokenFor(handler);
    if (token < 0) return -3;
    return raw.hud_surface_claim(surface.ptr, len(surface), priority, spec.ptr, len(spec), token);
}

pub fn surfaceUpdate(claim: i32, spec: []const u8) i32 {
    return raw.hud_surface_update(claim, spec.ptr, len(spec));
}

pub fn surfaceRelease(claim: i32) i32 {
    return raw.hud_surface_release(claim);
}

pub fn surfaceAnswer(ctx: i32, spec: []const u8) i32 {
    return raw.hud_surface_answer(ctx, spec.ptr, len(spec));
}
