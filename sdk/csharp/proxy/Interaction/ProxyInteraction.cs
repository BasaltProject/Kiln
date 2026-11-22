using System.Text;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Basalt.Proxy.Core;
using Basalt.Proxy.Internal;
using Basalt.Proxy.Values;
using static Basalt.Proxy.Internal.Native;
using static Basalt.Proxy.Core.Level;
using static Basalt.Proxy.Core.Priority;
namespace Basalt.Proxy.Interaction;

// Proxy interaction area (abi/areas/proxy_interaction.h): dialogs, resource packs, cookies, transfer and extra player
// operations. Observed events and completions are read with ValueResult and the Value reader.

/// <summary>A dialog button: the label and the command the client runs as the player when it is clicked.</summary>
public sealed class DialogButton
{
    public readonly string Label;
    public readonly string Command;

    public DialogButton(string label, string command)
    {
        Label = label;
        Command = command;
    }
}

public static unsafe class ProxyInteraction
{
    const int Invalid = -3;

    static string FetchText(ValueFill call)
    {
        int cap = 256;
        while (true)
        {
            byte[] buffer = new byte[cap];
            int n;
            fixed (byte* p = buffer) n = call(p, cap);
            if (n < 0)
            {
                return null;
            }
            if (n > cap)
            {
                cap = n;
                continue;
            }
            return Encoding.UTF8.GetString(buffer, 0, n);
        }
    }

    /// <summary>Observes player.resource_pack: the value holds player, player_name, id and status.</summary>
    public static int WatchResourcePacks(Handler handler)
    {
        byte[] n = Encoding.UTF8.GetBytes("player.resource_pack");
        int token = Native.TokenFor(handler);
        fixed (byte* p = n) return RawProxyInteraction.InteractionWatch(p, n.Length, token);
    }

    /// <summary>Observes player.settings: the value holds player, player_name and settings.</summary>
    public static int WatchSettings(Handler handler)
    {
        byte[] n = Encoding.UTF8.GetBytes("player.settings");
        int token = Native.TokenFor(handler);
        fixed (byte* p = n) return RawProxyInteraction.InteractionWatch(p, n.Length, token);
    }

    /// <summary>The value of an observed event or a completion.</summary>
    public static bool TryValueResult(int ctx, out Value root)
    {
        return ValueCall.Out(256, (o, c) => RawCoreValues.ValueResult(ctx, o, c)).TryValue(out root);
    }

    /// <summary>kind is "notice", "confirmation" or "multi_action"; body may be null.</summary>
    public static int DialogShow(int player, string kind, string title, string body, DialogButton[] buttons, bool closable = true)
    {
        ValueWriter w = new ValueWriter();
        w.BeginMap().Key("kind").String(kind).Key("title").String(title);
        if (body != null)
        {
            w.Key("body").String(body);
        }
        w.Key("buttons").BeginList();
        foreach (DialogButton button in buttons)
        {
            w.BeginMap().Key("label").String(button.Label).Key("command").String(button.Command).End();
        }
        w.End().Key("can_close_with_escape").Bool(closable).End();
        byte[] doc = w.Finish();
        if (doc == null)
        {
            return Invalid;
        }
        fixed (byte* d = doc) return RawProxyInteraction.DialogShow(player, d, doc.Length);
    }

    public static int DialogClear(int player) => RawProxyInteraction.DialogClear(player);

    /// <summary>Pushes a pack and returns its id text, or null when it failed; hash, prompt and id may be null.</summary>
    public static string ResourcePackPush(int player, string url, string hash, bool required, string prompt, string id)
    {
        ValueWriter w = new ValueWriter();
        w.BeginMap().Key("url").String(url);
        if (hash != null)
        {
            w.Key("hash").String(hash);
        }
        w.Key("required").Bool(required);
        if (prompt != null)
        {
            w.Key("prompt").String(prompt);
        }
        if (id != null)
        {
            w.Key("id").String(id);
        }
        w.End();
        byte[] doc = w.Finish();
        if (doc == null)
        {
            return null;
        }
        return FetchText((o, c) => { fixed (byte* d = doc) return RawProxyInteraction.ResourcePackPush(player, d, doc.Length, o, c); });
    }

    public static int ResourcePackRemove(int player, string id)
    {
        byte[] i = Encoding.UTF8.GetBytes(id);
        fixed (byte* p = i) return RawProxyInteraction.ResourcePackRemove(player, p, i.Length);
    }

    public static int ResourcePackClear(int player) => RawProxyInteraction.ResourcePackClear(player);

    public static int CookieSet(int player, string key, byte[] data)
    {
        byte[] k = Encoding.UTF8.GetBytes(key);
        fixed (byte* kp = k)
        fixed (byte* dp = data)
        {
            return RawProxyInteraction.CookieSet(player, kp, k.Length, dp, data.Length);
        }
    }

    /// <summary>Returns the request handle; the handler reads the cookie with CookieResult.</summary>
    public static int CookieRequest(int player, string key, Handler handler)
    {
        byte[] k = Encoding.UTF8.GetBytes(key);
        int token = Native.TokenFor(handler);
        fixed (byte* p = k) return RawProxyInteraction.CookieRequest(player, p, k.Length, token);
    }

    /// <summary>The cookie of a completion, or null when the client had none.</summary>
    public static byte[] CookieResult(int ctx)
    {
        return TryValueResult(ctx, out Value root) && root.TryBytes(out byte[] data) ? data : null;
    }

    public static int RequestCancel(int request) => RawCoreValues.RequestCancel(request);

    public static int PlayerTransfer(int player, string host, int port = 25565)
    {
        byte[] h = Encoding.UTF8.GetBytes(host);
        fixed (byte* p = h) return RawProxyInteraction.PlayerTransfer(player, p, h.Length, port);
    }

    public static int PlayersByUuid(string uuid)
    {
        byte[] u = Encoding.UTF8.GetBytes(uuid);
        fixed (byte* p = u) return RawProxyInteraction.PlayersByUuid(p, u.Length);
    }

    /// <summary>Players on the proxy, or on <paramref name="server"/> when it is not null or empty.</summary>
    public static int PlayersCount(string server = "")
    {
        byte[] s = Encoding.UTF8.GetBytes(server ?? "");
        fixed (byte* p = s) return RawProxyInteraction.PlayersCount(p, s.Length);
    }

    public static int PlayersBroadcast(string text, Args args = null)
    {
        byte[] t = Encoding.UTF8.GetBytes(text);
        byte[] a = args?.Packed();
        fixed (byte* tp = t)
        fixed (byte* ap = a)
        {
            return RawProxyInteraction.PlayersBroadcast(tp, t.Length, ap, a == null ? 0 : a.Length);
        }
    }

    public static string PlayerAddress(int player) => FetchText((o, c) => RawProxyInteraction.PlayerAddress(player, o, c));

    /// <summary>The client port, a negative code when unknown.</summary>
    public static int PlayerPort(int player) => RawProxyInteraction.PlayerPort(player);

    /// <summary>The client settings map; false until the client sent it.</summary>
    public static bool TryPlayerSettings(int player, out Value root)
    {
        return ValueCall.Out(256, (o, c) => RawProxyInteraction.PlayerSettings(player, o, c)).TryValue(out root);
    }

    public static double PlayerDataGetF64(int player, string key, double fallback)
    {
        byte[] k = Encoding.UTF8.GetBytes(key);
        fixed (byte* p = k) return RawProxyInteraction.PlayerDataGetF64(player, p, k.Length, fallback);
    }

    public static int PlayerDataSetF64(int player, string key, double value)
    {
        byte[] k = Encoding.UTF8.GetBytes(key);
        fixed (byte* p = k) return RawProxyInteraction.PlayerDataSetF64(player, p, k.Length, value);
    }
}
