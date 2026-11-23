using Basalt.Server.Core;
using Basalt.Server.Internal;
using Basalt.Server.Values;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using static Basalt.Server.Core.Level;
using static Basalt.Server.Core.Priority;
using static Basalt.Server.Internal.Native;

namespace Basalt.Server.Hud;

// HUD area (abi/areas/hud.h): scoreboards, boss bars, holograms, NPCs, tab and chat surfaces. Specs are encoded values;
// an out array receives an encoded value (UTF-8 text for team and display lookups).

public static unsafe class Displays
{
    public static int ScoreboardMain()
    {
        int status;
        status = RawHud.HudScoreboardMain();
        return status;
    }

    public static int ScoreboardCreate()
    {
        int status;
        status = RawHud.HudScoreboardCreate();
        return status;
    }

    public static int ScoreboardRelease(int board)
    {
        int status;
        status = RawHud.HudScoreboardRelease(board);
        return status;
    }

    public static int ScoreboardList(int board, string what, byte[] result)
    {
        byte[] whatB = Encoding.UTF8.GetBytes(what);
        int status;
        fixed (byte* whatP = whatB)
            fixed (byte* resultP = result)
                status = RawHud.HudScoreboardList(board, whatP, whatB.Length, resultP, result.Length);
        return status;
    }

    public static int ObjectiveAdd(int board, string name, byte[] spec)
    {
        byte[] nameB = Encoding.UTF8.GetBytes(name);
        int status;
        fixed (byte* nameP = nameB)
            fixed (byte* specP = spec)
                status = RawHud.HudObjectiveAdd(board, nameP, nameB.Length, specP, spec.Length);
        return status;
    }

    public static int ObjectiveRemove(int board, string name)
    {
        byte[] nameB = Encoding.UTF8.GetBytes(name);
        int status;
        fixed (byte* nameP = nameB)
            status = RawHud.HudObjectiveRemove(board, nameP, nameB.Length);
        return status;
    }

    public static int DisplaySet(int board, string slot, string objective)
    {
        byte[] slotB = Encoding.UTF8.GetBytes(slot);
        byte[] objectiveB = Encoding.UTF8.GetBytes(objective);
        int status;
        fixed (byte* slotP = slotB)
            fixed (byte* objectiveP = objectiveB)
                status = RawHud.HudDisplaySet(board, slotP, slotB.Length, objectiveP, objectiveB.Length);
        return status;
    }

    public static int DisplayGet(int board, string slot, byte[] result)
    {
        byte[] slotB = Encoding.UTF8.GetBytes(slot);
        int status;
        fixed (byte* slotP = slotB)
            fixed (byte* resultP = result)
                status = RawHud.HudDisplayGet(board, slotP, slotB.Length, resultP, result.Length);
        return status;
    }

    public static int ScoreGet(int board, string entry, string objective, out int result)
    {
        byte[] entryB = Encoding.UTF8.GetBytes(entry);
        byte[] objectiveB = Encoding.UTF8.GetBytes(objective);
        int resultV = 0;
        int status;
        fixed (byte* entryP = entryB)
            fixed (byte* objectiveP = objectiveB)
                status = RawHud.HudScoreGet(board, entryP, entryB.Length, objectiveP, objectiveB.Length, &resultV);
        result = resultV;
        return status;
    }

    public static int ScoreSet(int board, string entry, string objective, int value)
    {
        byte[] entryB = Encoding.UTF8.GetBytes(entry);
        byte[] objectiveB = Encoding.UTF8.GetBytes(objective);
        int status;
        fixed (byte* entryP = entryB)
            fixed (byte* objectiveP = objectiveB)
                status = RawHud.HudScoreSet(board, entryP, entryB.Length, objectiveP, objectiveB.Length, value);
        return status;
    }

    public static int ScoreReset(int board, string entry, string objective)
    {
        byte[] entryB = Encoding.UTF8.GetBytes(entry);
        byte[] objectiveB = Encoding.UTF8.GetBytes(objective);
        int status;
        fixed (byte* entryP = entryB)
            fixed (byte* objectiveP = objectiveB)
                status = RawHud.HudScoreReset(board, entryP, entryB.Length, objectiveP, objectiveB.Length);
        return status;
    }

    public static int TeamAdd(int board, string name)
    {
        byte[] nameB = Encoding.UTF8.GetBytes(name);
        int status;
        fixed (byte* nameP = nameB)
            status = RawHud.HudTeamAdd(board, nameP, nameB.Length);
        return status;
    }

    public static int TeamSet(int board, string name, byte[] spec)
    {
        byte[] nameB = Encoding.UTF8.GetBytes(name);
        int status;
        fixed (byte* nameP = nameB)
            fixed (byte* specP = spec)
                status = RawHud.HudTeamSet(board, nameP, nameB.Length, specP, spec.Length);
        return status;
    }

    public static int TeamRemove(int board, string name)
    {
        byte[] nameB = Encoding.UTF8.GetBytes(name);
        int status;
        fixed (byte* nameP = nameB)
            status = RawHud.HudTeamRemove(board, nameP, nameB.Length);
        return status;
    }

    public static int TeamJoin(int board, string team, string entry)
    {
        byte[] teamB = Encoding.UTF8.GetBytes(team);
        byte[] entryB = Encoding.UTF8.GetBytes(entry);
        int status;
        fixed (byte* teamP = teamB)
            fixed (byte* entryP = entryB)
                status = RawHud.HudTeamJoin(board, teamP, teamB.Length, entryP, entryB.Length);
        return status;
    }

    public static int TeamLeave(int board, string team, string entry)
    {
        byte[] teamB = Encoding.UTF8.GetBytes(team);
        byte[] entryB = Encoding.UTF8.GetBytes(entry);
        int status;
        fixed (byte* teamP = teamB)
            fixed (byte* entryP = entryB)
                status = RawHud.HudTeamLeave(board, teamP, teamB.Length, entryP, entryB.Length);
        return status;
    }

    public static int TeamOf(int board, string entry, byte[] result)
    {
        byte[] entryB = Encoding.UTF8.GetBytes(entry);
        int status;
        fixed (byte* entryP = entryB)
            fixed (byte* resultP = result)
                status = RawHud.HudTeamOf(board, entryP, entryB.Length, resultP, result.Length);
        return status;
    }

    public static int ScoreboardShow(int player, int board)
    {
        int status;
        status = RawHud.HudScoreboardShow(player, board);
        return status;
    }

    public static int ScoreboardShown(int player)
    {
        int status;
        status = RawHud.HudScoreboardShown(player);
        return status;
    }

    public static int BossbarCreate(byte[] spec)
    {
        int status;
        fixed (byte* specP = spec)
            status = RawHud.HudBossbarCreate(specP, spec.Length);
        return status;
    }

    public static int BossbarAttach(string id)
    {
        byte[] idB = Encoding.UTF8.GetBytes(id);
        int status;
        fixed (byte* idP = idB)
            status = RawHud.HudBossbarAttach(idP, idB.Length);
        return status;
    }

    public static int BossbarUpdate(int bar, byte[] spec)
    {
        int status;
        fixed (byte* specP = spec)
            status = RawHud.HudBossbarUpdate(bar, specP, spec.Length);
        return status;
    }

    public static int BossbarRemove(int bar)
    {
        int status;
        status = RawHud.HudBossbarRemove(bar);
        return status;
    }

    public static int BossbarPlayerAdd(int bar, int player)
    {
        int status;
        status = RawHud.HudBossbarPlayerAdd(bar, player);
        return status;
    }

    public static int BossbarPlayerRemove(int bar, int player)
    {
        int status;
        status = RawHud.HudBossbarPlayerRemove(bar, player);
        return status;
    }

    public static int BossbarPlayers(int bar, byte[] result)
    {
        int status;
        fixed (byte* resultP = result)
            status = RawHud.HudBossbarPlayers(bar, resultP, result.Length);
        return status;
    }

    public static int BossbarGet(int bar, byte[] result)
    {
        int status;
        fixed (byte* resultP = result)
            status = RawHud.HudBossbarGet(bar, resultP, result.Length);
        return status;
    }

    public static int BossbarKept(byte[] result)
    {
        int status;
        fixed (byte* resultP = result)
            status = RawHud.HudBossbarKept(resultP, result.Length);
        return status;
    }

    public static int HologramCreate(string world, string text, double x, double y, double z)
    {
        byte[] worldB = Encoding.UTF8.GetBytes(world);
        byte[] textB = Encoding.UTF8.GetBytes(text);
        int status;
        fixed (byte* worldP = worldB)
            fixed (byte* textP = textB)
                status = RawHud.HudHologramCreate(worldP, worldB.Length, textP, textB.Length, x, y, z);
        return status;
    }

    public static int HologramSet(int hologram, byte[] spec)
    {
        int status;
        fixed (byte* specP = spec)
            status = RawHud.HudHologramSet(hologram, specP, spec.Length);
        return status;
    }

    public static int HologramGet(int hologram, byte[] result)
    {
        int status;
        fixed (byte* resultP = result)
            status = RawHud.HudHologramGet(hologram, resultP, result.Length);
        return status;
    }

    public static int HologramRemove(int hologram)
    {
        int status;
        status = RawHud.HudHologramRemove(hologram);
        return status;
    }

    public static int NpcCreate(byte[] spec, Handler handler)
    {
        int status;
        fixed (byte* specP = spec)
            status = RawHud.HudNpcCreate(specP, spec.Length, handler == null ? 0 : Native.TokenFor(handler));
        return status;
    }

    public static int NpcSet(int npc, byte[] spec)
    {
        int status;
        fixed (byte* specP = spec)
            status = RawHud.HudNpcSet(npc, specP, spec.Length);
        return status;
    }

    public static int NpcGet(int npc, byte[] result)
    {
        int status;
        fixed (byte* resultP = result)
            status = RawHud.HudNpcGet(npc, resultP, result.Length);
        return status;
    }

    public static int NpcItem(int npc, int slot, byte[] result)
    {
        int status;
        fixed (byte* resultP = result)
            status = RawHud.HudNpcItem(npc, slot, resultP, result.Length);
        return status;
    }

    public static int NpcAttribute(int npc, string name, out double result)
    {
        byte[] nameB = Encoding.UTF8.GetBytes(name);
        double resultV = 0;
        int status;
        fixed (byte* nameP = nameB)
            status = RawHud.HudNpcAttribute(npc, nameP, nameB.Length, &resultV);
        result = resultV;
        return status;
    }

    public static int NpcAnimate(int npc, int action)
    {
        int status;
        status = RawHud.HudNpcAnimate(npc, action);
        return status;
    }

    public static int NpcRemove(int npc)
    {
        int status;
        status = RawHud.HudNpcRemove(npc);
        return status;
    }

    public static int PlayerSet(int player, byte[] spec)
    {
        int status;
        fixed (byte* specP = spec)
            status = RawHud.HudPlayerSet(player, specP, spec.Length);
        return status;
    }

    public static int PlayerGet(int player, byte[] result)
    {
        int status;
        fixed (byte* resultP = result)
            status = RawHud.HudPlayerGet(player, resultP, result.Length);
        return status;
    }

    public static int TabEntry(int viewer, int entity_id, int listed)
    {
        int status;
        status = RawHud.HudTabEntry(viewer, entity_id, listed);
        return status;
    }

    public static int SurfaceClaim(string surface, int priority, byte[] spec, Handler handler)
    {
        byte[] surfaceB = Encoding.UTF8.GetBytes(surface);
        int status;
        fixed (byte* surfaceP = surfaceB)
            fixed (byte* specP = spec)
                status = RawHud.HudSurfaceClaim(surfaceP, surfaceB.Length, priority, specP, spec.Length, handler == null ? 0 : Native.TokenFor(handler));
        return status;
    }

    public static int SurfaceUpdate(int claim, byte[] spec)
    {
        int status;
        fixed (byte* specP = spec)
            status = RawHud.HudSurfaceUpdate(claim, specP, spec.Length);
        return status;
    }

    public static int SurfaceRelease(int claim)
    {
        int status;
        status = RawHud.HudSurfaceRelease(claim);
        return status;
    }

    public static int SurfaceAnswer(int ctx, byte[] spec)
    {
        int status;
        fixed (byte* specP = spec)
            status = RawHud.HudSurfaceAnswer(ctx, specP, spec.Length);
        return status;
    }

}
