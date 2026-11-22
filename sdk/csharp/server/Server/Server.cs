using System;
using System.Text;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Basalt.Server.Core;
using Basalt.Server.Internal;
using Basalt.Server.Values;
using static Basalt.Server.Internal.Native;
using static Basalt.Server.Core.Level;
using static Basalt.Server.Core.Priority;
namespace Basalt.Server.Server;

// Server area for C#: permissions, dynamic commands, broadcast, plugin files and storage, services and the scheduler
// completions. Calls that answer a document return the import status (0 or negative) and fill a Value.

public static unsafe class ServerArea
{
    public const int BroadcastPlayers = 1, BroadcastConsole = 2;
    public const int ProvideCheck = 1, ProvideMeta = 2;
    public const int ErrInvalid = -1;
    public const int Buffer = 1024;

    static string Text(int capacity, ValueFill call)
    {
        OutResult result = ValueCall.Out(capacity, call);
        return result.Error == 0 ? Encoding.UTF8.GetString(result.Bytes) : null;
    }

    static int Doc(ValueFill call, out Value root)
    {
        root = default;
        OutResult result = ValueCall.Out(Buffer, call);
        if (result.Error != 0) return result.Error;
        return Value.TryOpen(result.Bytes, out root) ? 0 : ErrInvalid;
    }

    public static int PermissionCheck(string subject, string node)
    {
        byte[] s = Native.Utf8(subject), n = Native.Utf8(node);
        fixed (byte* sp = s, np = n) return RawServer.PermissionCheck(sp, s.Length, np, n.Length);
    }

    public static int PermissionHas(int player, string node)
    {
        byte[] n = Native.Utf8(node);
        fixed (byte* np = n) return RawServer.PermissionHas(player, np, n.Length);
    }

    static int PermissionSet(int player, string node, int mode)
    {
        byte[] n = Native.Utf8(node);
        fixed (byte* np = n) return RawServer.PermissionSet(player, np, n.Length, mode);
    }

    public static int PermissionGrant(int player, string node) => PermissionSet(player, node, 1);

    public static int PermissionDeny(int player, string node) => PermissionSet(player, node, 0);

    public static int PermissionDrop(int player, string node) => PermissionSet(player, node, -1);

    /// <summary>The meta text of the subject, or null when there is none.</summary>
    public static string PermissionMeta(string subject, string key)
    {
        byte[] s = Native.Utf8(subject), k = Native.Utf8(key);
        return Text(128, (o, c) => { fixed (byte* sp = s, kp = k) return RawServer.PermissionMeta(sp, s.Length, kp, k.Length, o, c); });
    }

    public static int PermissionProvide(int priority, int kinds, Handler handler) =>
        RawServer.PermissionProvide(priority, kinds, Native.TokenFor(handler));

    public static int PermissionProviderRemove(int provider) => RawServer.PermissionProviderRemove(provider);

    public static int PermissionChanged() => RawServer.PermissionChanged();

    public static int PermissionAnswer(int ctx, int answer) => RawServer.PermissionAnswer(ctx, answer);

    public static int PermissionAnswerMeta(int ctx, string text)
    {
        byte[] t = Native.Utf8(text);
        fixed (byte* tp = t) return RawServer.PermissionAnswerMeta(ctx, tp, t.Length);
    }

    /// <summary>A text field ("node", "kind", ...) of the query a permission provider runs for, or null.</summary>
    public static string PermissionQuery(int ctx, string field)
    {
        if (!ValueResult(ctx, out Value root) || !root.Find(field).TryString(out string text)) return null;
        return text;
    }

    /// <summary>Opens the value a completion callback (wait, lookup, service call) carries.</summary>
    public static bool ValueResult(int ctx, out Value root) =>
        ValueCall.Out(Buffer, (o, c) => RawCoreValues.ValueResult(ctx, o, c)).TryValue(out root);

    public static int Command(string spec, string permission, Handler handler, Handler complete = null)
    {
        byte[] s = Native.Utf8(spec), p = Native.Utf8(permission);
        int token = Native.TokenFor(handler);
        int completeToken = complete == null ? -1 : Native.TokenFor(complete);
        fixed (byte* sp = s, pp = p) return RawServer.CommandRegisterEx(sp, s.Length, pp, p.Length, token, completeToken);
    }

    public static int CommandAlias(string name, string alias)
    {
        byte[] n = Native.Utf8(name), a = Native.Utf8(alias);
        fixed (byte* np = n, ap = a) return RawServer.CommandAlias(np, n.Length, ap, a.Length);
    }

    public static int CommandUnregister(string name)
    {
        byte[] n = Native.Utf8(name);
        fixed (byte* np = n) return RawServer.CommandUnregister(np, n.Length);
    }

    public static int CommandRun(string line, int player = 0)
    {
        byte[] l = Native.Utf8(line);
        fixed (byte* lp = l) return RawServer.CommandRun(lp, l.Length, player);
    }

    public static string CompleteInput(int ctx) =>
        Text(256, (o, c) => RawServer.CommandCompleteInput(ctx, o, c));

    public static int CompleteAdd(int ctx, string candidate)
    {
        byte[] t = Native.Utf8(candidate);
        fixed (byte* tp = t) return RawServer.CommandCompleteAdd(ctx, tp, t.Length);
    }

    public static int CommandArgF64(int ctx, string name, out double value)
    {
        byte[] n = Native.Utf8(name);
        double v = 0;
        int rc;
        fixed (byte* np = n) rc = RawServer.CommandArgF64(ctx, np, n.Length, &v);
        value = v;
        return rc;
    }

    public static int CommandArgBool(int ctx, string name, out bool value)
    {
        byte[] n = Native.Utf8(name);
        int v = 0;
        int rc;
        fixed (byte* np = n) rc = RawServer.CommandArgBool(ctx, np, n.Length, &v);
        value = v != 0;
        return rc;
    }

    public static int Broadcast(string text, Args args, int flags)
    {
        byte[] t = Native.Utf8(text);
        byte[] a = args == null ? null : args.Packed();
        fixed (byte* tp = t, ap = a) return RawServer.ServerBroadcast(tp, t.Length, ap, a == null ? 0 : a.Length, flags);
    }

    public static string PlaceholderFill(string text, int player = 0)
    {
        byte[] t = Native.Utf8(text);
        return Text(256, (o, c) => { fixed (byte* tp = t) return RawServer.PlaceholderFill(tp, t.Length, player, o, c); });
    }

    public static int Info(out Value root) => Doc((o, c) => RawServer.ServerInfo(o, c), out root);

    /// <summary>Runs an admin operation; <paramref name="args"/> is a finished document or null.</summary>
    public static int Call(string op, byte[] args, out Value root)
    {
        byte[] n = Native.Utf8(op);
        return Doc((o, c) =>
        {
            fixed (byte* np = n, ap = args) return RawServer.ServerCall(np, n.Length, ap, args == null ? 0 : args.Length, o, c);
        }, out root);
    }

    public static int ProfileLookup(string name, Handler handler)
    {
        byte[] n = Native.Utf8(name);
        int token = Native.TokenFor(handler);
        fixed (byte* np = n) return RawServer.ProfileLookup(np, n.Length, token);
    }

    /// <summary>Reads up to <paramref name="output"/>.Length bytes; the result is the length the file needs.</summary>
    public static int FileRead(string path, byte[] output)
    {
        byte[] p = Native.Utf8(path);
        fixed (byte* pp = p, op = output) return RawServer.FilesRead(pp, p.Length, op, output.Length);
    }

    public static int FileWrite(string path, string data)
    {
        byte[] p = Native.Utf8(path), d = Native.Utf8(data);
        fixed (byte* pp = p, dp = d) return RawServer.FilesWrite(pp, p.Length, dp, d.Length);
    }

    public static bool FileExists(string path)
    {
        byte[] p = Native.Utf8(path);
        fixed (byte* pp = p) return RawServer.FilesExists(pp, p.Length) == 1;
    }

    public static int FileDelete(string path, bool recursive = false)
    {
        byte[] p = Native.Utf8(path);
        fixed (byte* pp = p) return RawServer.FilesDelete(pp, p.Length, recursive ? 1 : 0);
    }

    public static int FileList(string prefix, out Value root)
    {
        byte[] p = Native.Utf8(prefix);
        return Doc((o, c) => { fixed (byte* pp = p) return RawServer.FilesList(pp, p.Length, o, c); }, out root);
    }

    public static int FileStat(string path, out Value root)
    {
        byte[] p = Native.Utf8(path);
        return Doc((o, c) => { fixed (byte* pp = p) return RawServer.FilesStat(pp, p.Length, o, c); }, out root);
    }

    public static int FileWalk(string path, out Value root)
    {
        byte[] p = Native.Utf8(path);
        return Doc((o, c) => { fixed (byte* pp = p) return RawServer.FilesWalk(pp, p.Length, o, c); }, out root);
    }

    public static int FileCapabilities(string path, out Value root)
    {
        byte[] p = Native.Utf8(path);
        return Doc((o, c) => { fixed (byte* pp = p) return RawServer.FilesCapabilities(pp, p.Length, o, c); }, out root);
    }

    static int StoreSetDoc(string key, byte[] doc)
    {
        if (doc == null) return ErrInvalid;
        byte[] k = Native.Utf8(key);
        fixed (byte* kp = k, dp = doc) return RawServer.StoreSet(kp, k.Length, dp, doc.Length);
    }

    public static int StoreSet(string key, long value) => StoreSetDoc(key, new ValueWriter(16).Int(value).Finish());

    public static int StoreSet(string key, string value) => StoreSetDoc(key, new ValueWriter(256).String(value).Finish());

    public static int StoreGet(string key, out Value root)
    {
        byte[] k = Native.Utf8(key);
        return Doc((o, c) => { fixed (byte* kp = k) return RawServer.StoreGet(kp, k.Length, o, c); }, out root);
    }

    /// <summary>Writes a finished document under <paramref name="name"/>.</summary>
    public static int DataWrite(string name, byte[] doc)
    {
        if (doc == null) return ErrInvalid;
        byte[] n = Native.Utf8(name);
        fixed (byte* np = n, dp = doc) return RawServer.DataWrite(np, n.Length, dp, doc.Length);
    }

    public static int DataRead(string name, out Value root)
    {
        byte[] n = Native.Utf8(name);
        return Doc((o, c) => { fixed (byte* np = n) return RawServer.DataRead(np, n.Length, o, c); }, out root);
    }

    public static int ConfigValue(string key, out Value root)
    {
        byte[] k = Native.Utf8(key);
        return Doc((o, c) => { fixed (byte* kp = k) return RawServer.ConfigValue(kp, k.Length, o, c); }, out root);
    }

    public static int ServiceRegister(string name, Handler handler)
    {
        byte[] n = Native.Utf8(name);
        int token = Native.TokenFor(handler);
        fixed (byte* np = n) return RawServer.ServiceRegister(np, n.Length, token);
    }

    public static int ServiceReply(int ctx, byte[] doc)
    {
        fixed (byte* dp = doc) return RawServer.ServiceReply(ctx, dp, doc == null ? 0 : doc.Length);
    }

    public static int ServiceReplyNull(int ctx) => ServiceReply(ctx, null);

    public static int ServiceRemove(int service) => RawServer.ServiceRemove(service);

    public static bool ServiceHas(string name)
    {
        byte[] n = Native.Utf8(name);
        fixed (byte* np = n) return RawServer.ServiceHas(np, n.Length) == 1;
    }

    public static int ServiceList(out Value root) => Doc((o, c) => RawServer.ServiceList(o, c), out root);

    public static int ServiceCall(string name, byte[] args, int timeout, Handler handler)
    {
        byte[] n = Native.Utf8(name);
        int token = Native.TokenFor(handler);
        fixed (byte* np = n, ap = args) return RawServer.ServiceCall(np, n.Length, ap, args == null ? 0 : args.Length, timeout, token);
    }

    public static int Wait(int ticks, Handler handler) => RawServer.TaskWait(ticks, Native.TokenFor(handler));

    public static int At(string world, double x, double z, Handler handler)
    {
        byte[] w = Native.Utf8(world);
        int token = Native.TokenFor(handler);
        fixed (byte* wp = w) return RawServer.TaskAt(wp, w.Length, x, z, token);
    }
}
