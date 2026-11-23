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

namespace Basalt.Proxy.Admin;

// Proxy admin area (abi/areas/proxy_admin.h): backends, console and chat, permission providers, groups, component JSON,
// the plugin store, data by holder and waiting. Calls that give a value answer a Value through TryValue-style outs.

public static unsafe class Management
{
    const int Invalid = -3;

    static bool Open(OutResult result, out Value root) => result.TryValue(out root);

    static string Translations(OutResult result) => result.Error == 0 ? Encoding.UTF8.GetString(result.Bytes) : null;

    /// <summary>Status of ServersRegister (0 when it worked) and the reason the proxy gave otherwise.</summary>
    public static int ServersRegister(string name, string host, int port, string like, out string reason)
    {
        byte[] n = Encoding.UTF8.GetBytes(name), h = Encoding.UTF8.GetBytes(host), l = Encoding.UTF8.GetBytes(like);
        byte[] err = new byte[256];
        int status;
        fixed (byte* np = n) fixed (byte* hp = h) fixed (byte* lp = l) fixed (byte* ep = err)
            status = RawProxyAdmin.ServersRegister(np, n.Length, hp, h.Length, port, lp, l.Length, ep, err.Length);
        int end = 0;
        while (status < 0 && end < err.Length && err[end] != 0) end++;
        reason = Encoding.UTF8.GetString(err, 0, end);
        return status;
    }

    public static int ServersUnregister(string name)
    {
        byte[] n = Encoding.UTF8.GetBytes(name);
        fixed (byte* p = n) return RawProxyAdmin.ServersUnregister(p, n.Length);
    }

    /// <summary>Pings a backend; the handler reads the answer with CompletionValue. Returns the request handle.</summary>
    public static int ServersPing(string name, Handler handler)
    {
        byte[] n = Encoding.UTF8.GetBytes(name);
        int token = Native.TokenFor(handler);
        fixed (byte* p = n) return RawProxyAdmin.ServersPing(p, n.Length, token);
    }

    /// <summary>Runs the handler after ticks ticks of 50 ms. Returns the request handle.</summary>
    public static int Wait(int ticks, Handler handler) => RawProxyAdmin.Wait(ticks, Native.TokenFor(handler));

    public static int RequestCancel(int request) => RawCoreValues.RequestCancel(request);

    public static bool CompletionValue(int ctx, out Value root) =>
        Open(ValueCall.Out(256, (o, c) => RawCoreValues.ValueResult(ctx, o, c)), out root);

    /// <summary>1 when the line named a command, 0 when not, -4 without the capability and the operator's grant.</summary>
    public static int ConsoleRun(string line)
    {
        byte[] l = Encoding.UTF8.GetBytes(line);
        fixed (byte* p = l) return RawProxyAdmin.ConsoleRun(p, l.Length);
    }

    public static int PlayerChat(int player, string text)
    {
        byte[] t = Encoding.UTF8.GetBytes(text);
        fixed (byte* p = t) return RawProxyAdmin.PlayerChat(player, p, t.Length);
    }

    public static int PlayerSendJson(int player, string json)
    {
        byte[] j = Encoding.UTF8.GetBytes(json);
        fixed (byte* p = j) return RawProxyAdmin.PlayerSendJson(player, p, j.Length);
    }

    public static int PlayerKickJson(int player, string json)
    {
        byte[] j = Encoding.UTF8.GetBytes(json);
        fixed (byte* p = j) return RawProxyAdmin.PlayerKickJson(player, p, j.Length);
    }

    /// <summary>1 granted, 0 denied, -1 undecided. who "" is the console; server may be empty.</summary>
    public static int PermissionCheck(string who, string node, string server = "")
    {
        byte[] w = Encoding.UTF8.GetBytes(who), n = Encoding.UTF8.GetBytes(node);
        byte[] context = new byte[0];
        if (server.Length > 0)
        {
            context = new ValueWriter().BeginMap().Key("server").String(server).End().Finish();
            if (context == null) return Invalid;
        }
        fixed (byte* wp = w) fixed (byte* np = n) fixed (byte* cp = context)
            return RawProxyAdmin.PermissionCheck(wp, w.Length, np, n.Length, cp, context.Length);
    }

    public static string PermissionMeta(string who, string key)
    {
        byte[] w = Encoding.UTF8.GetBytes(who), k = Encoding.UTF8.GetBytes(key);
        return Translations(ValueCall.Out(256, (o, c) =>
        {
            fixed (byte* wp = w) fixed (byte* kp = k)
                return RawProxyAdmin.PermissionMeta(wp, w.Length, kp, k.Length, kp, 0, o, c);
        }));
    }

    /// <summary>Registers provider callbacks (null for none); returns the provider handle.</summary>
    public static int PermissionProvide(Handler check, Handler meta, int priority = 0) =>
        RawProxyAdmin.PermissionProvide(check == null ? 0 : Native.TokenFor(check), meta == null ? 0 : Native.TokenFor(meta), priority);

    public static int PermissionUnprovide(int provider) => RawProxyAdmin.PermissionUnprovide(provider);

    /// <summary>The question of a provider callback: a map with kind, name, uuid, console, text and context.</summary>
    public static bool PermissionRequest(int ctx, out Value root) =>
        Open(ValueCall.Out(256, (o, c) => RawProxyAdmin.PermissionRequest(ctx, o, c)), out root);

    /// <summary>1 grants, 0 denies, negative leaves it to the next provider.</summary>
    public static int PermissionAnswer(int ctx, int answer) => RawProxyAdmin.PermissionAnswer(ctx, answer);

    public static int PermissionAnswerText(int ctx, string text)
    {
        byte[] t = Encoding.UTF8.GetBytes(text);
        fixed (byte* p = t) return RawProxyAdmin.PermissionAnswerText(ctx, p, t.Length);
    }

    public static int PermissionChanged() => RawProxyAdmin.PermissionChanged();

    /// <summary>1 grants, 0 denies, -1 removes.</summary>
    public static int PermissionOverride(string name, string node, int value)
    {
        byte[] n = Encoding.UTF8.GetBytes(name), o = Encoding.UTF8.GetBytes(node);
        fixed (byte* np = n) fixed (byte* op = o) return RawProxyAdmin.PermissionOverride(np, n.Length, op, o.Length, value);
    }

    public static bool PermissionList(string name, out Value root)
    {
        byte[] n = Encoding.UTF8.GetBytes(name);
        return Open(ValueCall.Out(256, (o, c) => { fixed (byte* p = n) return RawProxyAdmin.PermissionList(p, n.Length, o, c); }), out root);
    }

    public static string GroupGet(string name)
    {
        byte[] n = Encoding.UTF8.GetBytes(name);
        return Translations(ValueCall.Out(64, (o, c) => { fixed (byte* p = n) return RawProxyAdmin.GroupGet(p, n.Length, o, c); }));
    }

    public static int GroupSet(string name, string group)
    {
        byte[] n = Encoding.UTF8.GetBytes(name), g = Encoding.UTF8.GetBytes(group);
        fixed (byte* np = n) fixed (byte* gp = g) return RawProxyAdmin.GroupSet(np, n.Length, gp, g.Length);
    }

    /// <summary>Reads a store key (holder null) or a player data key of holder; false when unset.</summary>
    public static bool DataValue(string holder, string key, out Value root)
    {
        byte[] k = Encoding.UTF8.GetBytes(key);
        byte[] h = holder == null ? null : Encoding.UTF8.GetBytes(holder);
        return Open(ValueCall.Out(256, (o, c) =>
        {
            fixed (byte* kp = k)
            {
                if (h == null) return RawProxyAdmin.StoreGet(kp, k.Length, o, c);
                fixed (byte* hp = h) return RawProxyAdmin.PlayerDataGet(hp, h.Length, kp, k.Length, o, c);
            }
        }), out root);
    }

    public static int DataSetValue(string holder, string key, byte[] doc)
    {
        if (doc == null || doc.Length == 0) return Invalid;
        byte[] k = Encoding.UTF8.GetBytes(key);
        fixed (byte* kp = k) fixed (byte* dp = doc)
        {
            if (holder == null) return RawProxyAdmin.StoreSet(kp, k.Length, dp, doc.Length);
            byte[] h = Encoding.UTF8.GetBytes(holder);
            fixed (byte* hp = h) return RawProxyAdmin.PlayerDataSet(hp, h.Length, kp, k.Length, dp, doc.Length);
        }
    }

    public static int DataSetInt(string holder, string key, long number) =>
        DataSetValue(holder, key, new ValueWriter().Int(number).Finish());

    public static int DataSetFloat(string holder, string key, double number) =>
        DataSetValue(holder, key, new ValueWriter().Float(number).Finish());

    public static int DataSetBool(string holder, string key, bool flag) =>
        DataSetValue(holder, key, new ValueWriter().Bool(flag).Finish());

    public static int DataSetStr(string holder, string key, string text) =>
        DataSetValue(holder, key, new ValueWriter().String(text).Finish());

    public static int DataRemove(string holder, string key) =>
        DataSetValue(holder, key, new ValueWriter().Null().Finish());

    /// <summary>The best limit numeric values of a store key: a list of maps with uuid, name and value.</summary>
    public static bool StoreTop(string key, int limit, out Value root)
    {
        byte[] k = Encoding.UTF8.GetBytes(key);
        return Open(ValueCall.Out(256, (o, c) => { fixed (byte* p = k) return RawProxyAdmin.StoreTop(p, k.Length, limit, o, c); }), out root);
    }
}
