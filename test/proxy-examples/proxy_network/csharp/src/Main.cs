using Basalt.Proxy.Core;
using Basalt.Proxy.Values;
using SdkConfig = global::Basalt.Proxy.Config.Config;
using SdkEvents = global::Basalt.Proxy.Events.Events;
using SdkLogger = global::Basalt.Proxy.Log.Logger;
using SdkProxyNetwork = global::Basalt.Proxy.Network.ProxyNetwork;
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
        System.Array.Copy(t, 0, o, 3, t.Length);
        return o;
    }

    static byte[] StringPrefixed(string first, string rest)
    {
        byte[] a = Utf8(first), b = Utf8(rest);
        byte[] o = new byte[1 + a.Length + b.Length];
        o[0] = (byte)a.Length;
        System.Array.Copy(a, 0, o, 1, a.Length);
        System.Array.Copy(b, 0, o, 1 + a.Length, b.Length);
        return o;
    }

    static void OnMessage(int ctx)
    {
        if (!Open(SdkProxyNetwork.MessageGet(ctx), out Value map))
        {
            SdkLogger.LogError("message read failed");
            return;
        }
        string data = map.TryFind("data", out Value d) && d.TryBytes(out byte[] raw) ? Encoding.UTF8.GetString(raw) : "";
        SdkLogger.LogInfo("net: message " + Text(map, "source") + " " + Text(map, "channel") + " " + data);
        int player = SdkEvents.EventPlayer(ctx);
        if (player > 0 && SdkProxyNetwork.MessageSend(player, "basalt:demo", Utf8("pong"), true) == 0)
            SdkLogger.LogInfo("net: reply sent");
    }

    static void OnDecider(int ctx)
    {
        byte[] body = SdkProxyNetwork.PacketBody(ctx);
        if (!Open(SdkProxyNetwork.PacketGet(ctx), out Value map) || body == null)
        {
            SdkLogger.LogError("packet read failed");
            return;
        }
        int at = Find(body, rewriteMarker);
        if (Find(body, cancelMarker) >= 0)
        {
            if (SdkProxyNetwork.PacketCancel(ctx) == 0) SdkLogger.LogInfo("net: cancel for " + Text(map, "player_name"));
        }
        else if (at >= 0)
        {
            for (int k = 0; k < Utf8(rewriteMarker).Length; k++)
                if (body[at + k] >= 'a' && body[at + k] <= 'z') body[at + k] -= 32;
            if (SdkProxyNetwork.PacketSetBody(ctx, body) == 0) SdkLogger.LogInfo("net: rewrite for " + Text(map, "player_name"));
        }
    }

    static void OnWatch(int ctx)
    {
        if (!Open(SdkProxyNetwork.PacketGet(ctx), out Value map)) return;
        if (!refusedOnce && SdkProxyNetwork.PacketCancel(ctx) < 0)
        {
            refusedOnce = true;
            SdkLogger.LogInfo("net: observer cancel refused");
        }
        if (map.TryFind("cancelled", out Value c) && c.TryBool(out bool seen) && seen)
            SdkLogger.LogInfo("net: watch saw a cancelled packet");
    }

    static void OnAntibot(int ctx)
    {
        if (Open(SdkProxyNetwork.AntibotEventGet(ctx), out Value map))
            SdkLogger.LogInfo("net: antibot " + Text(map, "kind") + " " + Text(map, "action"));
    }

    static void Idle(int ctx) { }

    static void OnJoin(int ctx)
    {
        int player = SdkEvents.EventPlayer(ctx);
        if (player <= 0) return;
        byte[] body = ChatBody("net welcome");
        if (SdkProxyNetwork.PacketSend(player, "clientbound.system_chat", body) == 0) SdkLogger.LogInfo("net: sent welcome");
        if (SdkProxyNetwork.PacketSend(player, "serverbound.play.chat_command", body) < 0)
            SdkLogger.LogInfo("net: wrong direction refused");
        if (SdkProxyNetwork.MessageSend(player, channel, Utf8("hello"), true) == 0) SdkLogger.LogInfo("net: sent hello message");
        if (SdkProxyNetwork.PacketReceive(player, "serverbound.play.custom_payload", StringPrefixed(channel, "ping")) == 0)
            SdkLogger.LogInfo("net: injected custom payload");
        if (SdkProxyNetwork.PacketReceive(player, "serverbound.play.chat_command", StringPrefixed("list", "")) == 0)
            SdkLogger.LogInfo("net: injected command");
    }

    static void OnEnable(int ctx)
    {
        if (Open(SdkProxyNetwork.PacketNames(), out Value names) && names.Count > 100) SdkLogger.LogInfo("net: packets known");
        if (Open(SdkProxyNetwork.PacketResolve("clientbound.system_chat"), out Value map) &&
            map.TryFind("serverbound", out Value sb) && sb.TryBool(out bool toServer) && !toServer)
            SdkLogger.LogInfo("net: resolved a clientbound play packet");
        if (SdkProxyNetwork.PacketResolve("nope.nothing") == null) SdkLogger.LogInfo("net: unknown packet refused");
        if (SdkProxyNetwork.ChannelRegister(channel) == 0) SdkLogger.LogInfo("net: channel registered");
        if (SdkProxyNetwork.ChannelRegister("BungeeCord") < 0) SdkLogger.LogInfo("net: bungee channel refused");
        if (SdkProxyNetwork.ChannelRegister("basalt:temp") == 0 && SdkProxyNetwork.ChannelUnregister("basalt:temp") == 0 &&
            SdkProxyNetwork.ChannelUnregister("basalt:temp") < 0)
            SdkLogger.LogInfo("net: temp channel unregistered");
        SdkProxyNetwork.MessageOn("basalt:demo", OnMessage);
        byte[] opts = new ValueWriter().BeginMap().Key("priority").String("late").Key("fail").String("closed").End().Finish();
        if (SdkProxyNetwork.PacketOn("clientbound.system_chat", opts, OnDecider) > 0) SdkLogger.LogInfo("net: decider listening");
        SdkProxyNetwork.PacketWatch("clientbound.system_chat", new byte[0], OnWatch);
        int listener = SdkProxyNetwork.PacketOn("serverbound.play.*", new byte[0], Idle);
        if (listener > 0 && SdkProxyNetwork.PacketRemove(listener) == 0 && SdkProxyNetwork.PacketRemove(listener) < 0)
            SdkLogger.LogInfo("net: listener removed");
        if (SdkProxyNetwork.PacketOn("nope.nothing", new byte[0], Idle) < 0) SdkLogger.LogInfo("net: bad listener name refused");
        if (SdkProxyNetwork.PacketGet(9999) == null) SdkLogger.LogInfo("net: bad context refused");
        SdkProxyNetwork.AntibotOn(OnAntibot);
        SdkProxyNetwork.AntibotVerify("NetBot");
        SdkLogger.LogInfo(SdkProxyNetwork.AntibotIsVerified("NetBot") == 1 ? "net: verified yes" : "net: verified no");
        SdkProxyNetwork.AntibotUnverify("NetBot");
        SdkLogger.LogInfo(SdkProxyNetwork.AntibotIsVerified("NetBot") == 0 ? "net: unverified" : "net: still verified");
        if (SdkProxyNetwork.AntibotBan("203.0.113.9") == 0) SdkLogger.LogInfo("net: ban ok");
        if (SdkProxyNetwork.AntibotUnban("203.0.113.9") == 0) SdkLogger.LogInfo("net: unban ok");
        if (SdkProxyNetwork.AntibotUnban("203.0.113.9") < 0) SdkLogger.LogInfo("net: unban absent");
        if (SdkProxyNetwork.AntibotBan("not an address") < 0) SdkLogger.LogInfo("net: bad address refused");
        SdkLogger.LogInfo(SdkProxyNetwork.AntibotUnderAttack() == 0 ? "net: no attack" : "net: under attack");
    }

    public static void Init()
    {
        rewriteMarker = SdkConfig.ConfigStr("rewrite-marker") ?? "";
        cancelMarker = SdkConfig.ConfigStr("cancel-marker") ?? "";
        channel = SdkConfig.ConfigStr("channel") ?? "";
        SdkEvents.On("plugin.enable", Priority.Normal, OnEnable);
        SdkEvents.On("player.join", Priority.Normal, OnJoin);
    }
}
