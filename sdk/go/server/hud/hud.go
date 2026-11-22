package hud

import "basaltmc.org/sdk/server/internal/runtime"

func HudScoreboardMain() int32 { return runtime.HudScoreboardMain() }

func HudScoreboardCreate() int32 { return runtime.HudScoreboardCreate() }

func HudScoreboardRelease(board int32) int32 { return runtime.HudScoreboardRelease(board) }

func HudScoreboardList(board int32, what string, out []byte) int32 {
	return runtime.HudScoreboardList(board, what, out)
}

func HudObjectiveAdd(board int32, name string, spec []byte) int32 {
	return runtime.HudObjectiveAdd(board, name, spec)
}

func HudObjectiveRemove(board int32, name string) int32 {
	return runtime.HudObjectiveRemove(board, name)
}

func HudDisplaySet(board int32, slot string, objective string) int32 {
	return runtime.HudDisplaySet(board, slot, objective)
}

func HudDisplayGet(board int32, slot string, out []byte) int32 {
	return runtime.HudDisplayGet(board, slot, out)
}

func HudScoreGet(board int32, entry string, objective string, out *int32) int32 {
	return runtime.HudScoreGet(board, entry, objective, out)
}

func HudScoreSet(board int32, entry string, objective string, value int32) int32 {
	return runtime.HudScoreSet(board, entry, objective, value)
}

func HudScoreReset(board int32, entry string, objective string) int32 {
	return runtime.HudScoreReset(board, entry, objective)
}

func HudTeamAdd(board int32, name string) int32 { return runtime.HudTeamAdd(board, name) }

func HudTeamSet(board int32, name string, spec []byte) int32 {
	return runtime.HudTeamSet(board, name, spec)
}

func HudTeamRemove(board int32, name string) int32 { return runtime.HudTeamRemove(board, name) }

func HudTeamJoin(board int32, team string, entry string) int32 {
	return runtime.HudTeamJoin(board, team, entry)
}

func HudTeamLeave(board int32, team string, entry string) int32 {
	return runtime.HudTeamLeave(board, team, entry)
}

func HudTeamOf(board int32, entry string, out []byte) int32 {
	return runtime.HudTeamOf(board, entry, out)
}

func HudScoreboardShow(player int32, board int32) int32 {
	return runtime.HudScoreboardShow(player, board)
}

func HudScoreboardShown(player int32) int32 { return runtime.HudScoreboardShown(player) }

func HudBossbarCreate(spec []byte) int32 { return runtime.HudBossbarCreate(spec) }

func HudBossbarAttach(id string) int32 { return runtime.HudBossbarAttach(id) }

func HudBossbarUpdate(bar int32, spec []byte) int32 { return runtime.HudBossbarUpdate(bar, spec) }

func HudBossbarRemove(bar int32) int32 { return runtime.HudBossbarRemove(bar) }

func HudBossbarPlayerAdd(bar int32, player int32) int32 {
	return runtime.HudBossbarPlayerAdd(bar, player)
}

func HudBossbarPlayerRemove(bar int32, player int32) int32 {
	return runtime.HudBossbarPlayerRemove(bar, player)
}

func HudBossbarPlayers(bar int32, out []byte) int32 { return runtime.HudBossbarPlayers(bar, out) }

func HudBossbarGet(bar int32, out []byte) int32 { return runtime.HudBossbarGet(bar, out) }

func HudBossbarKept(out []byte) int32 { return runtime.HudBossbarKept(out) }

func HudHologramCreate(world string, text string, x float64, y float64, z float64) int32 {
	return runtime.HudHologramCreate(world, text, x, y, z)
}

func HudHologramSet(hologram int32, spec []byte) int32 { return runtime.HudHologramSet(hologram, spec) }

func HudHologramGet(hologram int32, out []byte) int32 { return runtime.HudHologramGet(hologram, out) }

func HudHologramRemove(hologram int32) int32 { return runtime.HudHologramRemove(hologram) }

func HudNpcCreate(spec []byte, handler runtime.Handler) int32 {
	return runtime.HudNpcCreate(spec, handler)
}

func HudNpcSet(npc int32, spec []byte) int32 { return runtime.HudNpcSet(npc, spec) }

func HudNpcGet(npc int32, out []byte) int32 { return runtime.HudNpcGet(npc, out) }

func HudNpcItem(npc int32, slot int32, out []byte) int32 { return runtime.HudNpcItem(npc, slot, out) }

func HudNpcAttribute(npc int32, name string, out *float64) int32 {
	return runtime.HudNpcAttribute(npc, name, out)
}

func HudNpcAnimate(npc int32, action int32) int32 { return runtime.HudNpcAnimate(npc, action) }

func HudNpcRemove(npc int32) int32 { return runtime.HudNpcRemove(npc) }

func HudPlayerSet(player int32, spec []byte) int32 { return runtime.HudPlayerSet(player, spec) }

func HudPlayerGet(player int32, out []byte) int32 { return runtime.HudPlayerGet(player, out) }

func HudTabEntry(viewer int32, entity_id int32, listed int32) int32 {
	return runtime.HudTabEntry(viewer, entity_id, listed)
}

func HudSurfaceClaim(surface string, priority int32, spec []byte, handler runtime.Handler) int32 {
	return runtime.HudSurfaceClaim(surface, priority, spec, handler)
}

func HudSurfaceUpdate(claim int32, spec []byte) int32 { return runtime.HudSurfaceUpdate(claim, spec) }

func HudSurfaceRelease(claim int32) int32 { return runtime.HudSurfaceRelease(claim) }

func HudSurfaceAnswer(ctx int32, spec []byte) int32 { return runtime.HudSurfaceAnswer(ctx, spec) }
