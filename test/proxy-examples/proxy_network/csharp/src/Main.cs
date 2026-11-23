using Basalt.Proxy.Config;
using Basalt.Proxy.Core;
using Basalt.Proxy.Events;
using Basalt.Proxy.Log;
using Basalt.Proxy.Network;
using Basalt.Proxy.Values;
using System;
using System.Text;

public static class Plugin
{
    static string rewriteMarker = "";
    static string cancelMarker = "";
    static string channel = "";
    static bool refusedOnce;

    static byte[] Utf8(string s) => Encoding.UTF8.GetBytes(s);

    static int Find(byte[] hay, string needle)
    {
        byte[] n = Utf8(needle);
        for (int at = 0; n.Length > 0 && at + n.Length <= hay.Length; at++)
        {
            int i = 0;
            while (i < n.Length && hay[at + i] == n[i]) i++;
            if (i == n.Length) return at;
        }
        return -1;
    }

    static string Text(Value map, string key) => map.TryFind(key, out Value v) && v.TryString(out string s) ? s : "";

    static bool Open(byte[] doc, out Value value)
    {
        value = default;
        return doc != null && Value.TryOpen(doc, out value);
    }

    static byte[] ChatBody(string s)
    {
        byte[] t = Utf8(s);
        byte[] o = new byte[t.Length + 4];
        o[0] = 0x08;
        o[1] = (byte)(t.Length >> 8);
        o[2] = (byte)t.Length;
        Array.Copy(t, 0, o, 3, t.Length);
        return o;
    }

    static byte[] StringPrefixed(string first, string rest)
    {
        byte[] a = Utf8(first), b = Utf8(rest);
        byte[] o = new byte[1 + a.Length + b.Length];
        o[0] = (byte)a.Length;
        Array.Copy(a, 0, o, 1, a.Length);
        Array.Copy(b, 0, o, 1 + a.Length, b.Length);
        return o;
    }

    static void OnMessage(int ctx)
    {
        if (!Open(Transport.MessageGet(ctx), out Value map))
        {
            Logger.LogError("message read failed");
            return;
        }
        string data = map.TryFind("data", out Value d) && d.TryBytes(out byte[] raw) ? Encoding.UTF8.GetString(raw) : "";
        Logger.LogInfo("net: message " + Text(map, "source") + " " + Text(map, "channel") + " " + data);
        int player = EventBus.EventPlayer(ctx);
        if (player > 0 && Transport.MessageSend(player, "basalt:demo", Utf8("pong"), true) == 0)
            Logger.LogInfo("net: reply sent");
    }

    static void OnDecider(int ctx)
    {
        byte[] body = Transport.PacketBody(ctx);
        if (!Open(Transport.PacketGet(ctx), out Value map) || body == null)
        {
            Logger.LogError("packet read failed");
            return;
        }
        int at = Find(body, rewriteMarker);
        if (Find(body, cancelMarker) >= 0)
        {
            if (Transport.PacketCancel(ctx) == 0) Logger.LogInfo("net: cancel for " + Text(map, "player_name"));
        }
        else if (at >= 0)
        {
            for (int k = 0; k < Utf8(rewriteMarker).Length; k++)
                if (body[at + k] >= 'a' && body[at + k] <= 'z') body[at + k] -= 32;
            if (Transport.PacketSetBody(ctx, body) == 0) Logger.LogInfo("net: rewrite for " + Text(map, "player_name"));
        }
    }

    static void OnWatch(int ctx)
    {
        if (!Open(Transport.PacketGet(ctx), out Value map)) return;
        if (!refusedOnce && Transport.PacketCancel(ctx) < 0)
        {
            refusedOnce = true;
            Logger.LogInfo("net: observer cancel refused");
        }
        if (map.TryFind("cancelled", out Value c) && c.TryBool(out bool seen) && seen)
            Logger.LogInfo("net: watch saw a cancelled packet");
    }

    static void OnAntibot(int ctx)
    {
        if (Open(Transport.AntibotEventGet(ctx), out Value map))
            Logger.LogInfo("net: antibot " + Text(map, "kind") + " " + Text(map, "action"));
    }

    static void Idle(int ctx) { }

    static void OnJoin(int ctx)
    {
        int player = EventBus.EventPlayer(ctx);
        if (player <= 0) return;
        byte[] body = ChatBody("net welcome");
        if (Transport.PacketSend(player, "clientbound.system_chat", body) == 0) Logger.LogInfo("net: sent welcome");
        if (Transport.PacketSend(player, "serverbound.play.chat_command", body) < 0)
            Logger.LogInfo("net: wrong direction refused");
        if (Transport.MessageSend(player, channel, Utf8("hello"), true) == 0) Logger.LogInfo("net: sent hello message");
        if (Transport.PacketReceive(player, "serverbound.play.custom_payload", StringPrefixed(channel, "ping")) == 0)
            Logger.LogInfo("net: injected custom payload");
        if (Transport.PacketReceive(player, "serverbound.play.chat_command", StringPrefixed("list", "")) == 0)
            Logger.LogInfo("net: injected command");
    }

    static void OnEnable(int ctx)
    {
        if (Open(Transport.PacketNames(), out Value names) && names.Count > 100) Logger.LogInfo("net: packets known");
        if (Open(Transport.PacketResolve("clientbound.system_chat"), out Value map) &&
            map.TryFind("serverbound", out Value sb) && sb.TryBool(out bool toServer) && !toServer)
            Logger.LogInfo("net: resolved a clientbound play packet");
        if (Transport.PacketResolve("nope.nothing") == null) Logger.LogInfo("net: unknown packet refused");
        if (Transport.ChannelRegister(channel) == 0) Logger.LogInfo("net: channel registered");
        if (Transport.ChannelRegister("BungeeCord") < 0) Logger.LogInfo("net: bungee channel refused");
        if (Transport.ChannelRegister("basalt:temp") == 0 && Transport.ChannelUnregister("basalt:temp") == 0 &&
            Transport.ChannelUnregister("basalt:temp") < 0)
            Logger.LogInfo("net: temp channel unregistered");
        Transport.MessageOn("basalt:demo", OnMessage);
        byte[] opts = new ValueWriter().BeginMap().Key("priority").String("late").Key("fail").String("closed").End().Finish();
        if (Transport.PacketOn("clientbound.system_chat", opts, OnDecider) > 0) Logger.LogInfo("net: decider listening");
        Transport.PacketWatch("clientbound.system_chat", new byte[0], OnWatch);
        int listener = Transport.PacketOn("serverbound.play.*", new byte[0], Idle);
        if (listener > 0 && Transport.PacketRemove(listener) == 0 && Transport.PacketRemove(listener) < 0)
            Logger.LogInfo("net: listener removed");
        if (Transport.PacketOn("nope.nothing", new byte[0], Idle) < 0) Logger.LogInfo("net: bad listener name refused");
        if (Transport.PacketGet(9999) == null) Logger.LogInfo("net: bad context refused");
        Transport.AntibotOn(OnAntibot);
        Transport.AntibotVerify("NetBot");
        Logger.LogInfo(Transport.AntibotIsVerified("NetBot") == 1 ? "net: verified yes" : "net: verified no");
        Transport.AntibotUnverify("NetBot");
        Logger.LogInfo(Transport.AntibotIsVerified("NetBot") == 0 ? "net: unverified" : "net: still verified");
        if (Transport.AntibotBan("203.0.113.9") == 0) Logger.LogInfo("net: ban ok");
        if (Transport.AntibotUnban("203.0.113.9") == 0) Logger.LogInfo("net: unban ok");
        if (Transport.AntibotUnban("203.0.113.9") < 0) Logger.LogInfo("net: unban absent");
        if (Transport.AntibotBan("not an address") < 0) Logger.LogInfo("net: bad address refused");
        Logger.LogInfo(Transport.AntibotUnderAttack() == 0 ? "net: no attack" : "net: under attack");
    }

    public static void Init()
    {
        rewriteMarker = Settings.ConfigStr("rewrite-marker") ?? "";
        cancelMarker = Settings.ConfigStr("cancel-marker") ?? "";
        channel = Settings.ConfigStr("channel") ?? "";
        EventBus.On("plugin.enable", Priority.Normal, OnEnable);
        EventBus.On("player.join", Priority.Normal, OnJoin);
    }
}
