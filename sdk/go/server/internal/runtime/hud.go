package runtime

import "unsafe"

// HUD area (abi/areas/hud.h): scoreboards, boss bars, holograms, NPCs, tab and chat surfaces. Specs are encoded values
// built with the values Writer; an out slice receives an encoded value (UTF-8 text for team and display lookups).

func hudBytes(data []byte) (*byte, int32) {
	if len(data) == 0 {
		return nil, 0
	}
	return unsafe.SliceData(data), int32(len(data))
}

func HudScoreboardMain() int32 {
	return rawHudScoreboardMain()
}

func HudScoreboardCreate() int32 {
	return rawHudScoreboardCreate()
}

func HudScoreboardRelease(board int32) int32 {
	return rawHudScoreboardRelease(board)
}

func HudScoreboardList(board int32, what string, out []byte) int32 {
	whatPtr, whatLen := ptr(what)
	outPtr, outCap := hudBytes(out)
	return rawHudScoreboardList(board, whatPtr, whatLen, outPtr, outCap)
}

func HudObjectiveAdd(board int32, name string, spec []byte) int32 {
	namePtr, nameLen := ptr(name)
	specPtr, specLen := hudBytes(spec)
	return rawHudObjectiveAdd(board, namePtr, nameLen, specPtr, specLen)
}

func HudObjectiveRemove(board int32, name string) int32 {
	namePtr, nameLen := ptr(name)
	return rawHudObjectiveRemove(board, namePtr, nameLen)
}

func HudDisplaySet(board int32, slot string, objective string) int32 {
	slotPtr, slotLen := ptr(slot)
	objectivePtr, objectiveLen := ptr(objective)
	return rawHudDisplaySet(board, slotPtr, slotLen, objectivePtr, objectiveLen)
}

func HudDisplayGet(board int32, slot string, out []byte) int32 {
	slotPtr, slotLen := ptr(slot)
	outPtr, outCap := hudBytes(out)
	return rawHudDisplayGet(board, slotPtr, slotLen, outPtr, outCap)
}

func HudScoreGet(board int32, entry string, objective string, out *int32) int32 {
	entryPtr, entryLen := ptr(entry)
	objectivePtr, objectiveLen := ptr(objective)
	return rawHudScoreGet(board, entryPtr, entryLen, objectivePtr, objectiveLen, out)
}

func HudScoreSet(board int32, entry string, objective string, value int32) int32 {
	entryPtr, entryLen := ptr(entry)
	objectivePtr, objectiveLen := ptr(objective)
	return rawHudScoreSet(board, entryPtr, entryLen, objectivePtr, objectiveLen, value)
}

func HudScoreReset(board int32, entry string, objective string) int32 {
	entryPtr, entryLen := ptr(entry)
	objectivePtr, objectiveLen := ptr(objective)
	return rawHudScoreReset(board, entryPtr, entryLen, objectivePtr, objectiveLen)
}

func HudTeamAdd(board int32, name string) int32 {
	namePtr, nameLen := ptr(name)
	return rawHudTeamAdd(board, namePtr, nameLen)
}

func HudTeamSet(board int32, name string, spec []byte) int32 {
	namePtr, nameLen := ptr(name)
	specPtr, specLen := hudBytes(spec)
	return rawHudTeamSet(board, namePtr, nameLen, specPtr, specLen)
}

func HudTeamRemove(board int32, name string) int32 {
	namePtr, nameLen := ptr(name)
	return rawHudTeamRemove(board, namePtr, nameLen)
}

func HudTeamJoin(board int32, team string, entry string) int32 {
	teamPtr, teamLen := ptr(team)
	entryPtr, entryLen := ptr(entry)
	return rawHudTeamJoin(board, teamPtr, teamLen, entryPtr, entryLen)
}

func HudTeamLeave(board int32, team string, entry string) int32 {
	teamPtr, teamLen := ptr(team)
	entryPtr, entryLen := ptr(entry)
	return rawHudTeamLeave(board, teamPtr, teamLen, entryPtr, entryLen)
}

func HudTeamOf(board int32, entry string, out []byte) int32 {
	entryPtr, entryLen := ptr(entry)
	outPtr, outCap := hudBytes(out)
	return rawHudTeamOf(board, entryPtr, entryLen, outPtr, outCap)
}

func HudScoreboardShow(player int32, board int32) int32 {
	return rawHudScoreboardShow(player, board)
}

func HudScoreboardShown(player int32) int32 {
	return rawHudScoreboardShown(player)
}

func HudBossbarCreate(spec []byte) int32 {
	specPtr, specLen := hudBytes(spec)
	return rawHudBossbarCreate(specPtr, specLen)
}

func HudBossbarAttach(id string) int32 {
	idPtr, idLen := ptr(id)
	return rawHudBossbarAttach(idPtr, idLen)
}

func HudBossbarUpdate(bar int32, spec []byte) int32 {
	specPtr, specLen := hudBytes(spec)
	return rawHudBossbarUpdate(bar, specPtr, specLen)
}

func HudBossbarRemove(bar int32) int32 {
	return rawHudBossbarRemove(bar)
}

func HudBossbarPlayerAdd(bar int32, player int32) int32 {
	return rawHudBossbarPlayerAdd(bar, player)
}

func HudBossbarPlayerRemove(bar int32, player int32) int32 {
	return rawHudBossbarPlayerRemove(bar, player)
}

func HudBossbarPlayers(bar int32, out []byte) int32 {
	outPtr, outCap := hudBytes(out)
	return rawHudBossbarPlayers(bar, outPtr, outCap)
}

func HudBossbarGet(bar int32, out []byte) int32 {
	outPtr, outCap := hudBytes(out)
	return rawHudBossbarGet(bar, outPtr, outCap)
}

func HudBossbarKept(out []byte) int32 {
	outPtr, outCap := hudBytes(out)
	return rawHudBossbarKept(outPtr, outCap)
}

func HudHologramCreate(world string, text string, x float64, y float64, z float64) int32 {
	worldPtr, worldLen := ptr(world)
	textPtr, textLen := ptr(text)
	return rawHudHologramCreate(worldPtr, worldLen, textPtr, textLen, x, y, z)
}

func HudHologramSet(hologram int32, spec []byte) int32 {
	specPtr, specLen := hudBytes(spec)
	return rawHudHologramSet(hologram, specPtr, specLen)
}

func HudHologramGet(hologram int32, out []byte) int32 {
	outPtr, outCap := hudBytes(out)
	return rawHudHologramGet(hologram, outPtr, outCap)
}

func HudHologramRemove(hologram int32) int32 {
	return rawHudHologramRemove(hologram)
}

func HudNpcCreate(spec []byte, handler Handler) int32 {
	specPtr, specLen := hudBytes(spec)
	return rawHudNpcCreate(specPtr, specLen, tokenFor(handler))
}

func HudNpcSet(npc int32, spec []byte) int32 {
	specPtr, specLen := hudBytes(spec)
	return rawHudNpcSet(npc, specPtr, specLen)
}

func HudNpcGet(npc int32, out []byte) int32 {
	outPtr, outCap := hudBytes(out)
	return rawHudNpcGet(npc, outPtr, outCap)
}

func HudNpcItem(npc int32, slot int32, out []byte) int32 {
	outPtr, outCap := hudBytes(out)
	return rawHudNpcItem(npc, slot, outPtr, outCap)
}

func HudNpcAttribute(npc int32, name string, out *float64) int32 {
	namePtr, nameLen := ptr(name)
	return rawHudNpcAttribute(npc, namePtr, nameLen, out)
}

func HudNpcAnimate(npc int32, action int32) int32 {
	return rawHudNpcAnimate(npc, action)
}

func HudNpcRemove(npc int32) int32 {
	return rawHudNpcRemove(npc)
}

func HudPlayerSet(player int32, spec []byte) int32 {
	specPtr, specLen := hudBytes(spec)
	return rawHudPlayerSet(player, specPtr, specLen)
}

func HudPlayerGet(player int32, out []byte) int32 {
	outPtr, outCap := hudBytes(out)
	return rawHudPlayerGet(player, outPtr, outCap)
}

func HudTabEntry(viewer int32, entity_id int32, listed int32) int32 {
	return rawHudTabEntry(viewer, entity_id, listed)
}

func HudSurfaceClaim(surface string, priority int32, spec []byte, handler Handler) int32 {
	surfacePtr, surfaceLen := ptr(surface)
	specPtr, specLen := hudBytes(spec)
	return rawHudSurfaceClaim(surfacePtr, surfaceLen, priority, specPtr, specLen, tokenFor(handler))
}

func HudSurfaceUpdate(claim int32, spec []byte) int32 {
	specPtr, specLen := hudBytes(spec)
	return rawHudSurfaceUpdate(claim, specPtr, specLen)
}

func HudSurfaceRelease(claim int32) int32 {
	return rawHudSurfaceRelease(claim)
}

func HudSurfaceAnswer(ctx int32, spec []byte) int32 {
	specPtr, specLen := hudBytes(spec)
	return rawHudSurfaceAnswer(ctx, specPtr, specLen)
}
