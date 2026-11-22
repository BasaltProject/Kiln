package org.basaltmc.server.hud;
import org.basaltmc.server.internal.*;
import org.basaltmc.server.core.*;

import org.teavm.interop.Address;

/** HUD area (abi/areas/hud.h): scoreboards, boss bars, holograms, NPCs, tab and chat surfaces. Specs are encoded values; an out array receives an encoded value. */
public final class Hud {
    private Hud() {}

    public static int scoreboardMain() {
        return RawHud.rawHudScoreboardMain();
    }

    public static int scoreboardCreate() {
        return RawHud.rawHudScoreboardCreate();
    }

    public static int scoreboardRelease(int board) {
        return RawHud.rawHudScoreboardRelease(board);
    }

    public static int scoreboardList(int board, String what, byte[] out) {
        byte[] whatBytes = Native.utf8(what);
        return RawHud.rawHudScoreboardList(board, Native.at(whatBytes), whatBytes.length, Native.at(out), out.length);
    }

    public static int objectiveAdd(int board, String name, byte[] spec) {
        byte[] nameBytes = Native.utf8(name);
        return RawHud.rawHudObjectiveAdd(board, Native.at(nameBytes), nameBytes.length, Native.at(spec), spec.length);
    }

    public static int objectiveRemove(int board, String name) {
        byte[] nameBytes = Native.utf8(name);
        return RawHud.rawHudObjectiveRemove(board, Native.at(nameBytes), nameBytes.length);
    }

    public static int displaySet(int board, String slot, String objective) {
        byte[] slotBytes = Native.utf8(slot);
        byte[] objectiveBytes = Native.utf8(objective);
        return RawHud.rawHudDisplaySet(board, Native.at(slotBytes), slotBytes.length, Native.at(objectiveBytes), objectiveBytes.length);
    }

    public static int displayGet(int board, String slot, byte[] out) {
        byte[] slotBytes = Native.utf8(slot);
        return RawHud.rawHudDisplayGet(board, Native.at(slotBytes), slotBytes.length, Native.at(out), out.length);
    }

    public static int scoreGet(int board, String entry, String objective, int[] out) {
        byte[] entryBytes = Native.utf8(entry);
        byte[] objectiveBytes = Native.utf8(objective);
        return RawHud.rawHudScoreGet(board, Native.at(entryBytes), entryBytes.length, Native.at(objectiveBytes), objectiveBytes.length, Address.ofData(out));
    }

    public static int scoreSet(int board, String entry, String objective, int value) {
        byte[] entryBytes = Native.utf8(entry);
        byte[] objectiveBytes = Native.utf8(objective);
        return RawHud.rawHudScoreSet(board, Native.at(entryBytes), entryBytes.length, Native.at(objectiveBytes), objectiveBytes.length, value);
    }

    public static int scoreReset(int board, String entry, String objective) {
        byte[] entryBytes = Native.utf8(entry);
        byte[] objectiveBytes = Native.utf8(objective);
        return RawHud.rawHudScoreReset(board, Native.at(entryBytes), entryBytes.length, Native.at(objectiveBytes), objectiveBytes.length);
    }

    public static int teamAdd(int board, String name) {
        byte[] nameBytes = Native.utf8(name);
        return RawHud.rawHudTeamAdd(board, Native.at(nameBytes), nameBytes.length);
    }

    public static int teamSet(int board, String name, byte[] spec) {
        byte[] nameBytes = Native.utf8(name);
        return RawHud.rawHudTeamSet(board, Native.at(nameBytes), nameBytes.length, Native.at(spec), spec.length);
    }

    public static int teamRemove(int board, String name) {
        byte[] nameBytes = Native.utf8(name);
        return RawHud.rawHudTeamRemove(board, Native.at(nameBytes), nameBytes.length);
    }

    public static int teamJoin(int board, String team, String entry) {
        byte[] teamBytes = Native.utf8(team);
        byte[] entryBytes = Native.utf8(entry);
        return RawHud.rawHudTeamJoin(board, Native.at(teamBytes), teamBytes.length, Native.at(entryBytes), entryBytes.length);
    }

    public static int teamLeave(int board, String team, String entry) {
        byte[] teamBytes = Native.utf8(team);
        byte[] entryBytes = Native.utf8(entry);
        return RawHud.rawHudTeamLeave(board, Native.at(teamBytes), teamBytes.length, Native.at(entryBytes), entryBytes.length);
    }

    public static int teamOf(int board, String entry, byte[] out) {
        byte[] entryBytes = Native.utf8(entry);
        return RawHud.rawHudTeamOf(board, Native.at(entryBytes), entryBytes.length, Native.at(out), out.length);
    }

    public static int scoreboardShow(int player, int board) {
        return RawHud.rawHudScoreboardShow(player, board);
    }

    public static int scoreboardShown(int player) {
        return RawHud.rawHudScoreboardShown(player);
    }

    public static int bossbarCreate(byte[] spec) {
        return RawHud.rawHudBossbarCreate(Native.at(spec), spec.length);
    }

    public static int bossbarAttach(String id) {
        byte[] idBytes = Native.utf8(id);
        return RawHud.rawHudBossbarAttach(Native.at(idBytes), idBytes.length);
    }

    public static int bossbarUpdate(int bar, byte[] spec) {
        return RawHud.rawHudBossbarUpdate(bar, Native.at(spec), spec.length);
    }

    public static int bossbarRemove(int bar) {
        return RawHud.rawHudBossbarRemove(bar);
    }

    public static int bossbarPlayerAdd(int bar, int player) {
        return RawHud.rawHudBossbarPlayerAdd(bar, player);
    }

    public static int bossbarPlayerRemove(int bar, int player) {
        return RawHud.rawHudBossbarPlayerRemove(bar, player);
    }

    public static int bossbarPlayers(int bar, byte[] out) {
        return RawHud.rawHudBossbarPlayers(bar, Native.at(out), out.length);
    }

    public static int bossbarGet(int bar, byte[] out) {
        return RawHud.rawHudBossbarGet(bar, Native.at(out), out.length);
    }

    public static int bossbarKept(byte[] out) {
        return RawHud.rawHudBossbarKept(Native.at(out), out.length);
    }

    public static int hologramCreate(String world, String text, double x, double y, double z) {
        byte[] worldBytes = Native.utf8(world);
        byte[] textBytes = Native.utf8(text);
        return RawHud.rawHudHologramCreate(Native.at(worldBytes), worldBytes.length, Native.at(textBytes), textBytes.length, x, y, z);
    }

    public static int hologramSet(int hologram, byte[] spec) {
        return RawHud.rawHudHologramSet(hologram, Native.at(spec), spec.length);
    }

    public static int hologramGet(int hologram, byte[] out) {
        return RawHud.rawHudHologramGet(hologram, Native.at(out), out.length);
    }

    public static int hologramRemove(int hologram) {
        return RawHud.rawHudHologramRemove(hologram);
    }

    public static int npcCreate(byte[] spec, Handler handler) {
        return RawHud.rawHudNpcCreate(Native.at(spec), spec.length, handler == null ? 0 : Callbacks.tokenFor(handler));
    }

    public static int npcSet(int npc, byte[] spec) {
        return RawHud.rawHudNpcSet(npc, Native.at(spec), spec.length);
    }

    public static int npcGet(int npc, byte[] out) {
        return RawHud.rawHudNpcGet(npc, Native.at(out), out.length);
    }

    public static int npcItem(int npc, int slot, byte[] out) {
        return RawHud.rawHudNpcItem(npc, slot, Native.at(out), out.length);
    }

    public static int npcAttribute(int npc, String name, double[] out) {
        byte[] nameBytes = Native.utf8(name);
        return RawHud.rawHudNpcAttribute(npc, Native.at(nameBytes), nameBytes.length, Address.ofData(out));
    }

    public static int npcAnimate(int npc, int action) {
        return RawHud.rawHudNpcAnimate(npc, action);
    }

    public static int npcRemove(int npc) {
        return RawHud.rawHudNpcRemove(npc);
    }

    public static int playerSet(int player, byte[] spec) {
        return RawHud.rawHudPlayerSet(player, Native.at(spec), spec.length);
    }

    public static int playerGet(int player, byte[] out) {
        return RawHud.rawHudPlayerGet(player, Native.at(out), out.length);
    }

    public static int tabEntry(int viewer, int entity_id, int listed) {
        return RawHud.rawHudTabEntry(viewer, entity_id, listed);
    }

    public static int surfaceClaim(String surface, int priority, byte[] spec, Handler handler) {
        byte[] surfaceBytes = Native.utf8(surface);
        return RawHud.rawHudSurfaceClaim(Native.at(surfaceBytes), surfaceBytes.length, priority, Native.at(spec), spec.length, handler == null ? 0 : Callbacks.tokenFor(handler));
    }

    public static int surfaceUpdate(int claim, byte[] spec) {
        return RawHud.rawHudSurfaceUpdate(claim, Native.at(spec), spec.length);
    }

    public static int surfaceRelease(int claim) {
        return RawHud.rawHudSurfaceRelease(claim);
    }

    public static int surfaceAnswer(int ctx, byte[] spec) {
        return RawHud.rawHudSurfaceAnswer(ctx, Native.at(spec), spec.length);
    }

}
