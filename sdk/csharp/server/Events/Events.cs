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
namespace Basalt.Server.Events;

// Events area for C#: strings and byte arrays over the raw imports, documents as Value (false when a call failed).

public static unsafe class EventsArea
{
    public const int Post = 1, Cancelled = 2;

    static readonly byte[] None = new byte[0];

    static byte[] Bytes(byte[] data) => data ?? None;

    static string Text(int capacity, ValueFill call)
    {
        OutResult result = ValueCall.Out(capacity, call);
        return result.Error == 0 ? Encoding.UTF8.GetString(result.Bytes) : null;
    }

    static bool Doc(int initial, ValueFill call, out Value root)
    {
        root = default;
        return ValueCall.Out(initial, call).TryValue(out root);
    }

    /// <summary>Listens to <paramref name="name"/>; <paramref name="filters"/> is a finished map document or null.</summary>
    public static int OnEx(string name, int priority, int flags, byte[] filters, Handler handler)
    {
        byte[] n = Native.Utf8(name);
        byte[] f = Bytes(filters);
        int token = Native.TokenFor(handler);
        fixed (byte* np = n, fp = f) return RawEvents.EventOnEx(np, n.Length, priority, flags, fp, f.Length, token);
    }

    public static string EventName(int ctx) => Text(64, (o, c) => RawEvents.EventName(ctx, o, c));

    public static bool EventGet(int ctx, string field, int initial, out Value value)
    {
        byte[] f = Native.Utf8(field);
        return Doc(initial, (o, c) => { fixed (byte* fp = f) return RawEvents.EventGet(ctx, fp, f.Length, o, c); }, out value);
    }

    public static int EventSet(int ctx, string field, byte[] value)
    {
        byte[] f = Native.Utf8(field);
        byte[] v = Bytes(value);
        fixed (byte* fp = f, vp = v) return RawEvents.EventSet(ctx, fp, f.Length, vp, v.Length);
    }

    public static bool EventFields(int ctx, int initial, out Value value) =>
        Doc(initial, (o, c) => RawEvents.EventFields(ctx, o, c), out value);

    public static bool EventPayload(int ctx, int initial, out Value value) =>
        Doc(initial, (o, c) => RawEvents.EventPayload(ctx, o, c), out value);

    public static bool EventDescribe(string name, int initial, out Value value)
    {
        byte[] n = Native.Utf8(name);
        return Doc(initial, (o, c) => { fixed (byte* np = n) return RawEvents.EventDescribe(np, n.Length, o, c); }, out value);
    }

    public static bool EventNames(int initial, out Value value) =>
        Doc(initial, (o, c) => RawEvents.EventNames(o, c), out value);

    public static int EventCancelled(int ctx) => RawEvents.EventCancelled(ctx);

    public static int EventCancelWith(int ctx, string text, Args args = null)
    {
        byte[] t = Native.Utf8(text);
        byte[] a = Bytes(args?.Packed());
        fixed (byte* tp = t, ap = a) return RawEvents.EventCancelWith(ctx, tp, t.Length, ap, a.Length);
    }

    public static string EventCancelMessage(int ctx) => Text(256, (o, c) => RawEvents.EventCancelMessage(ctx, o, c));

    public static int EventIsPost(int ctx) => RawEvents.EventIsPost(ctx);

    public static int EventEmit(string name, byte[] value)
    {
        byte[] n = Native.Utf8(name);
        byte[] v = Bytes(value);
        fixed (byte* np = n, vp = v) return RawEvents.EventEmit(np, n.Length, vp, v.Length);
    }

    public static int PlayerByUuid(string uuid)
    {
        byte[] u = Native.Utf8(uuid);
        fixed (byte* up = u) return RawEvents.PlayerByUuid(up, u.Length);
    }

    public static string PlayerUuid(int player) => Text(64, (o, c) => RawEvents.PlayerUuid(player, o, c));

    public static bool PlayerGet(int player, string name, int initial, out Value value)
    {
        byte[] n = Native.Utf8(name);
        return Doc(initial, (o, c) => { fixed (byte* np = n) return RawEvents.PlayerGet(player, np, n.Length, o, c); }, out value);
    }

    public static int PlayerSet(int player, string name, byte[] value)
    {
        byte[] n = Native.Utf8(name);
        byte[] v = Bytes(value);
        fixed (byte* np = n, vp = v) return RawEvents.PlayerSet(player, np, n.Length, vp, v.Length);
    }

    /// <summary>Runs a named operation; <paramref name="args"/> is a finished list document or null.</summary>
    public static bool PlayerCall(int player, string op, byte[] args, int initial, out Value value)
    {
        byte[] o = Native.Utf8(op);
        byte[] a = Bytes(args);
        return Doc(initial, (x, c) =>
        {
            fixed (byte* op_ = o, ap = a) return RawEvents.PlayerCall(player, op_, o.Length, ap, a.Length, x, c);
        }, out value);
    }

    public static bool PlayerDescribe(int initial, out Value value) =>
        Doc(initial, (o, c) => RawEvents.PlayerDescribe(o, c), out value);

    /// <summary>The health in half hearts; false when the call failed.</summary>
    public static bool PlayerHealth(int player, out double health)
    {
        double result = 0;
        int rc = RawEvents.PlayerHealth(player, &result);
        health = result;
        return rc >= 0;
    }

    public static int PlayerSetHealth(int player, double health) => RawEvents.PlayerSetHealth(player, health);

    public static string PlayerGamemode(int player) => Text(16, (o, c) => RawEvents.PlayerGamemode(player, o, c));

    public static int PlayerSetGamemode(int player, string mode)
    {
        byte[] m = Native.Utf8(mode);
        fixed (byte* mp = m) return RawEvents.PlayerSetGamemode(player, mp, m.Length);
    }

    public static int PlayerTeleport(int player, string world, double x, double y, double z, float yaw, float pitch)
    {
        byte[] w = Native.Utf8(world);
        fixed (byte* wp = w) return RawEvents.PlayerTeleport(player, wp, w.Length, x, y, z, yaw, pitch);
    }

    public static int PlayerTitle(int player, string title, string subtitle, int fadeIn, int stay, int fadeOut,
        Args args = null)
    {
        byte[] t = Native.Utf8(title);
        byte[] s = Native.Utf8(subtitle);
        byte[] a = Bytes(args?.Packed());
        fixed (byte* tp = t, sp = s, ap = a)
            return RawEvents.PlayerTitle(player, tp, t.Length, sp, s.Length, fadeIn, stay, fadeOut, ap, a.Length);
    }

    public static int PlayerKick(int player, string text, Args args = null)
    {
        byte[] t = Native.Utf8(text);
        byte[] a = Bytes(args?.Packed());
        fixed (byte* tp = t, ap = a) return RawEvents.PlayerKick(player, tp, t.Length, ap, a.Length);
    }

    public static int PlayerEffect(int player, string effect, int ticks, int amplifier)
    {
        byte[] e = Native.Utf8(effect);
        fixed (byte* ep = e) return RawEvents.PlayerEffect(player, ep, e.Length, ticks, amplifier);
    }

    public static int PlayerOpenInventory(int player, string kind, byte[] spec)
    {
        byte[] k = Native.Utf8(kind);
        byte[] s = Bytes(spec);
        fixed (byte* kp = k, sp = s) return RawEvents.PlayerOpenInventory(player, kp, k.Length, sp, s.Length);
    }

    public static int PlayerCloseWindow(int player) => RawEvents.PlayerCloseWindow(player);

    /// <summary>Shows a form (a finished document); the handler runs once with the answer, read with CompletionValue.</summary>
    public static int FormOpen(int player, byte[] form, Handler handler)
    {
        byte[] f = Bytes(form);
        int token = Native.TokenFor(handler);
        fixed (byte* fp = f) return RawEvents.FormOpen(player, fp, f.Length, token);
    }

    public static int FormClose(int player) => RawEvents.FormClose(player);

    public static int DialogShow(int player, string dialog, Handler handler)
    {
        byte[] d = Native.Utf8(dialog);
        int token = Native.TokenFor(handler);
        fixed (byte* dp = d) return RawEvents.DialogShow(player, dp, d.Length, token);
    }

    public static int DialogClear(int player) => RawEvents.DialogClear(player);

    public static bool CompletionValue(int ctx, int initial, out Value value) =>
        Doc(initial, (o, c) => RawCoreValues.ValueResult(ctx, o, c), out value);
}
