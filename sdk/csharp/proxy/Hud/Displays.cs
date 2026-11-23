using Basalt.Proxy.Core;
using Basalt.Proxy.Internal;
using Basalt.Proxy.Values;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using static Basalt.Proxy.Core.Level;
using static Basalt.Proxy.Core.Priority;
using static Basalt.Proxy.Internal.Native;

namespace Basalt.Proxy.Hud;

// Proxy hud area (abi/areas/proxy_hud.h): boss bars, sidebar, tab list, tab rows, server links and chat completions.
// A null optional text leaves that part unchanged; every call answers 0 or a negative code.

public static unsafe class Displays
{
    const int Invalid = -3;

    static byte[] Utf8(string text) => Encoding.UTF8.GetBytes(text);

    static void Key(ValueWriter w, string name, string text)
    {
        if (text != null) w.Key(name).String(text);
    }

    public static int ClearTitle(int player, bool reset) => RawProxyHud.HudClearTitle(player, reset ? 1 : 0);

    public static int TabList(int player, string header, string footer)
    {
        var w = new ValueWriter().BeginMap();
        Key(w, "header", header);
        Key(w, "footer", footer);
        byte[] doc = w.End().Finish();
        if (doc == null) return Invalid;
        fixed (byte* d = doc) return RawProxyHud.HudTabList(player, d, doc.Length);
    }

    public static int BossbarShow(int player, string id, string text, double progress, string color, string overlay)
    {
        var w = new ValueWriter().BeginMap();
        Key(w, "text", text);
        w.Key("progress").Float(progress);
        Key(w, "color", color);
        Key(w, "overlay", overlay);
        byte[] doc = w.End().Finish();
        if (doc == null) return Invalid;
        byte[] i = Utf8(id);
        fixed (byte* ip = i)
        fixed (byte* d = doc)
            return RawProxyHud.HudBossbarShow(player, ip, i.Length, d, doc.Length);
    }

    public static int BossbarHide(int player, string id)
    {
        byte[] i = Utf8(id);
        fixed (byte* ip = i) return RawProxyHud.HudBossbarHide(player, ip, i.Length);
    }

    public static int BossbarProgress(int player, string id, double progress)
    {
        byte[] i = Utf8(id);
        fixed (byte* ip = i) return RawProxyHud.HudBossbarProgress(player, ip, i.Length, progress);
    }

    public static int BossbarTitle(int player, string id, string text)
    {
        byte[] i = Utf8(id);
        byte[] t = Utf8(text);
        fixed (byte* ip = i)
        fixed (byte* tp = t)
            return RawProxyHud.HudBossbarTitle(player, ip, i.Length, tp, t.Length, null, 0);
    }

    public static int BossbarStyle(int player, string id, string color, string overlay)
    {
        byte[] i = Utf8(id);
        byte[] c = Utf8(color);
        byte[] o = Utf8(overlay);
        fixed (byte* ip = i)
        fixed (byte* cp = c)
        fixed (byte* op = o)
            return RawProxyHud.HudBossbarStyle(player, ip, i.Length, cp, c.Length, op, o.Length);
    }

    public static int Sidebar(int player, string title, string[] lines)
    {
        var w = new ValueWriter().BeginMap();
        Key(w, "title", title);
        w.Key("lines").BeginList();
        foreach (string line in lines) w.String(line);
        byte[] doc = w.End().End().Finish();
        if (doc == null) return Invalid;
        fixed (byte* d = doc) return RawProxyHud.HudSidebar(player, d, doc.Length);
    }

    public static int SidebarClear(int player) => RawProxyHud.HudSidebarClear(player);

    public static int TabEntry(int player, string id, string name, string text, long latency)
    {
        var w = new ValueWriter().BeginMap();
        Key(w, "name", name);
        Key(w, "text", text);
        w.Key("latency").Int(latency);
        byte[] doc = w.End().Finish();
        if (doc == null) return Invalid;
        byte[] i = Utf8(id);
        fixed (byte* ip = i)
        fixed (byte* d = doc)
            return RawProxyHud.HudTabEntry(player, ip, i.Length, d, doc.Length);
    }

    public static int TabEntryRemove(int player, string id)
    {
        byte[] i = Utf8(id);
        fixed (byte* ip = i) return RawProxyHud.HudTabEntryRemove(player, ip, i.Length);
    }

    /// <summary>Label then url for each link.</summary>
    public static int ServerLinks(int player, string[] pairs)
    {
        var w = new ValueWriter().BeginList();
        for (int at = 0; at + 1 < pairs.Length; at += 2) w.BeginList().String(pairs[at]).String(pairs[at + 1]).End();
        byte[] doc = w.End().Finish();
        if (doc == null) return Invalid;
        fixed (byte* d = doc) return RawProxyHud.HudServerLinks(player, d, doc.Length);
    }

    public static int ChatCompletions(int player, string action, string[] words)
    {
        var w = new ValueWriter().BeginList();
        foreach (string word in words) w.String(word);
        byte[] doc = w.End().Finish();
        if (doc == null) return Invalid;
        byte[] a = Utf8(action);
        fixed (byte* ap = a)
        fixed (byte* d = doc)
            return RawProxyHud.HudChatCompletions(player, ap, a.Length, d, doc.Length);
    }
}
