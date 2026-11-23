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

namespace Basalt.Proxy.Values;

// Value builder and reader over the byte format of basalt_values.h. The writer grows a heap buffer (an optional limit
// acts as a fixed capacity); the reader works in place on a validated document and never throws on malformed input.

public enum ValueKind { Absent = -1, Null, Bool, Int, Float, String, Bytes, List, Map }

public enum ValueError { None, Invalid, Overflow }

public static class ValueLimits
{
    public const int MaxBytes = 1 << 20, MaxDepth = 16, MaxNodes = 65536, MaxKeyBytes = 256;
}

/// <summary>Builds one document. A failure is sticky: later calls do nothing and Finish returns null.</summary>
public sealed class ValueWriter
{
    sealed class Frame
    {
        public int Start;
        public int Count;
        public bool IsMap;
        public bool PendingKey;
    }

    byte[] buf = new byte[64];
    int len;
    readonly int limit;
    int nodes;
    readonly List<Frame> frames = new List<Frame>();

    public ValueError Error { get; private set; }

    public bool Ok => Error == ValueError.None;

    public ValueWriter() : this(ValueLimits.MaxBytes) { }

    /// <summary>Fails with Overflow once the document would pass <paramref name="limit"/> bytes.</summary>
    public ValueWriter(int limit)
    {
        this.limit = Math.Min(limit, ValueLimits.MaxBytes);
    }

    void Fail(ValueError reason)
    {
        if (Error == ValueError.None) Error = reason;
    }

    bool Room(int extra)
    {
        if (Error != ValueError.None) return false;
        if (extra < 0 || extra > limit - len)
        {
            Fail(ValueError.Overflow);
            return false;
        }
        int need = len + extra;
        if (need > buf.Length)
        {
            int size = buf.Length * 2;
            while (size < need) size *= 2;
            Array.Resize(ref buf, size);
        }
        return true;
    }

    void Put(int b)
    {
        if (Room(1)) buf[len++] = (byte)b;
    }

    void Leb(ulong value)
    {
        while (value >= 0x80)
        {
            Put((int)(value & 0x7F) | 0x80);
            value >>= 7;
        }
        Put((int)value);
    }

    void Raw(byte[] data)
    {
        if (Room(data.Length))
        {
            Array.Copy(data, 0, buf, len, data.Length);
            len += data.Length;
        }
    }

    bool Begin()
    {
        if (Error != ValueError.None) return false;
        int n = frames.Count;
        if (n == 0)
        {
            if (nodes > 0)
            {
                Fail(ValueError.Invalid);
                return false;
            }
        }
        else
        {
            Frame top = frames[n - 1];
            if (top.IsMap)
            {
                if (!top.PendingKey)
                {
                    Fail(ValueError.Invalid);
                    return false;
                }
                top.PendingKey = false;
            }
            else
            {
                top.Count++;
            }
        }
        nodes++;
        if (n > ValueLimits.MaxDepth || nodes > ValueLimits.MaxNodes)
        {
            Fail(ValueError.Invalid);
            return false;
        }
        return true;
    }

    public ValueWriter Null()
    {
        if (Begin()) Put(0);
        return this;
    }

    public ValueWriter Bool(bool value)
    {
        if (Begin()) Put(value ? 2 : 1);
        return this;
    }

    public ValueWriter Int(long value)
    {
        if (Begin())
        {
            Put(3);
            Leb(((ulong)value << 1) ^ (ulong)(value >> 63));
        }
        return this;
    }

    public ValueWriter Float(double value)
    {
        if (Begin())
        {
            Put(4);
            long bits = BitConverter.DoubleToInt64Bits(value);
            for (int shift = 0; shift < 64; shift += 8) Put((int)((bits >> shift) & 0xFF));
        }
        return this;
    }

    void Blob(int tag, byte[] data)
    {
        if (Begin())
        {
            Put(tag);
            Leb((ulong)data.Length);
            Raw(data);
        }
    }

    public ValueWriter String(string text)
    {
        Blob(5, Encoding.UTF8.GetBytes(text));
        return this;
    }

    public ValueWriter Bytes(byte[] data)
    {
        Blob(6, data);
        return this;
    }

    void Open(bool isMap)
    {
        if (!Begin()) return;
        Put(isMap ? 8 : 7);
        if (Error == ValueError.None) frames.Add(new Frame { Start = len, IsMap = isMap });
    }

    public ValueWriter BeginList()
    {
        Open(false);
        return this;
    }

    public ValueWriter BeginMap()
    {
        Open(true);
        return this;
    }

    /// <summary>Closes the innermost list or map, inserting its count in front of the items.</summary>
    public ValueWriter End()
    {
        if (Error != ValueError.None) return this;
        int n = frames.Count;
        if (n == 0 || (frames[n - 1].IsMap && frames[n - 1].PendingKey))
        {
            Fail(ValueError.Invalid);
            return this;
        }
        Frame top = frames[n - 1];
        frames.RemoveAt(n - 1);
        int size = 1;
        for (ulong v = (ulong)top.Count; v >= 0x80; v >>= 7) size++;
        if (!Room(size)) return this;
        Array.Copy(buf, top.Start, buf, top.Start + size, len - top.Start);
        ulong c = (ulong)top.Count;
        int at = top.Start;
        while (c >= 0x80)
        {
            buf[at++] = (byte)((c & 0x7F) | 0x80);
            c >>= 7;
        }
        buf[at] = (byte)c;
        len += size;
        return this;
    }

    /// <summary>The next key of the open map; the value follows.</summary>
    public ValueWriter Key(string name)
    {
        if (Error != ValueError.None) return this;
        int n = frames.Count;
        byte[] encoded = Encoding.UTF8.GetBytes(name);
        if (n == 0 || !frames[n - 1].IsMap || frames[n - 1].PendingKey || encoded.Length > ValueLimits.MaxKeyBytes)
        {
            Fail(ValueError.Invalid);
            return this;
        }
        Leb((ulong)encoded.Length);
        Raw(encoded);
        frames[n - 1].Count++;
        frames[n - 1].PendingKey = true;
        return this;
    }

    /// <summary>A copy of the finished document, or null when the writer failed or a container is still open.</summary>
    public byte[] Finish()
    {
        if (Error != ValueError.None) return null;
        if (frames.Count != 0 || nodes == 0)
        {
            Error = ValueError.Invalid;
            return null;
        }
        byte[] outBytes = new byte[len];
        Array.Copy(buf, outBytes, len);
        return outBytes;
    }
}

/// <summary>Iterates the items of a list or the entries of a map: call Next, then read Item (and Key for a map).</summary>
public sealed class ValueIter
{
    readonly byte[] bytes;
    int pos;
    long left;
    readonly bool isMap;

    /// <summary>The current item after a true Next.</summary>
    public Value Item { get; private set; }

    /// <summary>The key of the current entry, empty for a list.</summary>
    public string Key { get; private set; } = "";

    /// <summary>The raw UTF-8 bytes of the key.</summary>
    public byte[] KeyBytes { get; private set; } = new byte[0];

    internal ValueIter(byte[] bytes, int pos, long left, bool isMap)
    {
        this.bytes = bytes;
        this.pos = pos;
        this.left = left;
        this.isMap = isMap;
    }

    public bool Next()
    {
        int end = bytes.Length;
        if (left <= 0 || pos >= end) return false;
        int cursor = pos;
        if (isMap)
        {
            if (!Value.Leb(bytes, end, ref cursor, out ulong klen) || klen > (ulong)(end - cursor))
            {
                left = 0;
                return false;
            }
            KeyBytes = new byte[(int)klen];
            Array.Copy(bytes, cursor, KeyBytes, 0, (int)klen);
            Key = Encoding.UTF8.GetString(KeyBytes);
            cursor += (int)klen;
        }
        Item = new Value(bytes, cursor);
        int after = Value.Skip(bytes, end, cursor);
        pos = after < 0 ? end : after;
        left--;
        return true;
    }
}

/// <summary>A position in a validated document. The default Value is absent (Kind is Absent, every getter fails).</summary>
public readonly struct Value
{
    readonly byte[] bytes;
    readonly int off;

    internal Value(byte[] bytes, int off)
    {
        this.bytes = bytes;
        this.off = off;
    }

    /// <summary>Validates the whole document (tags, limits, duplicate keys, trailing bytes); false when malformed.</summary>
    public static bool TryOpen(byte[] document, out Value root)
    {
        root = default;
        if (document == null || document.Length == 0 || document.Length > ValueLimits.MaxBytes) return false;
        int nodes = 0;
        if (Walk(document, document.Length, 0, 0, ref nodes) != document.Length) return false;
        root = new Value(document, 0);
        return true;
    }

    internal static bool Leb(byte[] b, int end, ref int pos, out ulong value)
    {
        value = 0;
        int at = pos;
        for (int shift = 0; shift < 70; shift += 7)
        {
            if (at >= end) return false;
            int c = b[at++];
            if (shift == 63 && c > 1) return false;
            value |= (ulong)(c & 0x7F) << shift;
            if ((c & 0x80) == 0)
            {
                pos = at;
                return true;
            }
        }
        return false;
    }

    internal static int Skip(byte[] b, int end, int start)
    {
        if (start < 0 || start >= end) return -1;
        int tag = b[start];
        int pos = start + 1;
        ulong n;
        switch (tag)
        {
            case 3:
                return Leb(b, end, ref pos, out n) ? pos : -1;
            case 4:
                return end - pos >= 8 ? pos + 8 : -1;
            case 5:
            case 6:
                return Leb(b, end, ref pos, out n) && n <= (ulong)(end - pos) ? pos + (int)n : -1;
            case 7:
            case 8:
                if (!Leb(b, end, ref pos, out n)) return -1;
                for (; n > 0; n--)
                {
                    if (tag == 8)
                    {
                        if (!Leb(b, end, ref pos, out ulong k) || k > (ulong)(end - pos)) return -1;
                        pos += (int)k;
                    }
                    pos = Skip(b, end, pos);
                    if (pos < 0) return -1;
                }
                return pos;
            default:
                return pos;
        }
    }

    static bool Same(byte[] b, int x, int y, int n)
    {
        for (int i = 0; i < n; i++) if (b[x + i] != b[y + i]) return false;
        return true;
    }

    static int Walk(byte[] b, int end, int start, int depth, ref int nodes)
    {
        if (depth > ValueLimits.MaxDepth) return -1;
        nodes++;
        if (nodes > ValueLimits.MaxNodes || start >= end) return -1;
        int tag = b[start];
        int pos = start + 1;
        ulong n;
        switch (tag)
        {
            case 0:
            case 1:
            case 2:
                return pos;
            case 3:
                return Leb(b, end, ref pos, out n) ? pos : -1;
            case 4:
                return end - pos >= 8 ? pos + 8 : -1;
            case 5:
            case 6:
                return Leb(b, end, ref pos, out n) && n <= (ulong)(end - pos) ? pos + (int)n : -1;
            case 7:
                if (!Leb(b, end, ref pos, out n) || n > (ulong)(end - pos)) return -1;
                for (ulong i = 0; i < n; i++)
                {
                    pos = Walk(b, end, pos, depth + 1, ref nodes);
                    if (pos < 0) return -1;
                }
                return pos;
            case 8:
            {
                if (!Leb(b, end, ref pos, out n) || n > (ulong)(end - pos)) return -1;
                int first = pos;
                for (ulong i = 0; i < n; i++)
                {
                    if (!Leb(b, end, ref pos, out ulong klen) || klen > (ulong)ValueLimits.MaxKeyBytes ||
                        klen > (ulong)(end - pos)) return -1;
                    int name = pos;
                    int scan = first;
                    for (ulong j = 0; j < i; j++)
                    {
                        Leb(b, end, ref scan, out ulong olen);
                        if (olen == klen && Same(b, scan, name, (int)klen)) return -1;
                        scan = Skip(b, end, scan + (int)olen);
                        if (scan < 0) return -1;
                    }
                    pos = Walk(b, end, name + (int)klen, depth + 1, ref nodes);
                    if (pos < 0) return -1;
                }
                return pos;
            }
            default:
                return -1;
        }
    }

    public ValueKind Kind
    {
        get
        {
            if (bytes == null || off >= bytes.Length) return ValueKind.Absent;
            switch (bytes[off])
            {
                case 1:
                case 2: return ValueKind.Bool;
                case 3: return ValueKind.Int;
                case 4: return ValueKind.Float;
                case 5: return ValueKind.String;
                case 6: return ValueKind.Bytes;
                case 7: return ValueKind.List;
                case 8: return ValueKind.Map;
                default: return ValueKind.Null;
            }
        }
    }

    public bool Present => Kind != ValueKind.Absent;

    public bool IsNull => Kind == ValueKind.Null;

    public bool TryBool(out bool value)
    {
        value = false;
        if (Kind != ValueKind.Bool) return false;
        value = bytes[off] == 2;
        return true;
    }

    public bool TryInt(out long value)
    {
        value = 0;
        if (Kind != ValueKind.Int) return false;
        int pos = off + 1;
        if (!Leb(bytes, bytes.Length, ref pos, out ulong raw)) return false;
        value = (long)(raw >> 1) ^ -(long)(raw & 1);
        return true;
    }

    public bool TryFloat(out double value)
    {
        value = 0;
        if (Kind != ValueKind.Float || bytes.Length - off < 9) return false;
        long bits = 0;
        for (int i = 0; i < 8; i++) bits |= (long)bytes[off + 1 + i] << (8 * i);
        value = BitConverter.Int64BitsToDouble(bits);
        return true;
    }

    bool Payload(ValueKind kind, out byte[] data)
    {
        data = null;
        if (Kind != kind) return false;
        int pos = off + 1;
        if (!Leb(bytes, bytes.Length, ref pos, out ulong n) || n > (ulong)(bytes.Length - pos)) return false;
        data = new byte[(int)n];
        Array.Copy(bytes, pos, data, 0, (int)n);
        return true;
    }

    /// <summary>The text of a string value (invalid UTF-8 becomes U+FFFD).</summary>
    public bool TryString(out string text)
    {
        text = null;
        if (!Payload(ValueKind.String, out byte[] data)) return false;
        text = Encoding.UTF8.GetString(data);
        return true;
    }

    /// <summary>A copy of the raw UTF-8 payload of a string value.</summary>
    public bool TryStringBytes(out byte[] data) => Payload(ValueKind.String, out data);

    /// <summary>A copy of the payload of a bytes value.</summary>
    public bool TryBytes(out byte[] data) => Payload(ValueKind.Bytes, out data);

    /// <summary>Items of a list or entries of a map, -1 for any other value.</summary>
    public int Count
    {
        get
        {
            ValueKind k = Kind;
            if (k != ValueKind.List && k != ValueKind.Map) return -1;
            int pos = off + 1;
            return Leb(bytes, bytes.Length, ref pos, out ulong n) ? (int)n : -1;
        }
    }

    public ValueIter Iter()
    {
        ValueKind k = Kind;
        int pos = off + 1;
        long left = 0;
        if ((k == ValueKind.List || k == ValueKind.Map) && Leb(bytes, bytes.Length, ref pos, out ulong n)) left = (long)n;
        return new ValueIter(bytes, pos, left, k == ValueKind.Map);
    }

    public bool TryAt(int index, out Value item)
    {
        item = default;
        if (Kind != ValueKind.List || index < 0) return false;
        ValueIter it = Iter();
        for (int i = 0; it.Next(); i++)
        {
            if (i == index)
            {
                item = it.Item;
                return true;
            }
        }
        return false;
    }

    public bool TryFind(string key, out Value item)
    {
        item = default;
        if (Kind != ValueKind.Map) return false;
        byte[] wanted = Encoding.UTF8.GetBytes(key);
        ValueIter it = Iter();
        while (it.Next())
        {
            if (it.KeyBytes.Length != wanted.Length) continue;
            bool same = true;
            for (int i = 0; i < wanted.Length; i++)
            {
                if (it.KeyBytes[i] != wanted[i])
                {
                    same = false;
                    break;
                }
            }
            if (same)
            {
                item = it.Item;
                return true;
            }
        }
        return false;
    }

    /// <summary>The value under <paramref name="key"/>, or an absent Value.</summary>
    public Value Find(string key)
    {
        TryFind(key, out Value item);
        return item;
    }

    /// <summary>Item <paramref name="index"/> of a list, or an absent Value.</summary>
    public Value At(int index)
    {
        TryAt(index, out Value item);
        return item;
    }
}

public unsafe delegate int ValueFill(byte* output, int cap);

/// <summary>The outcome of ValueCall.Out: Error is 0 or a negative import result (TooLarge when nothing fit).</summary>
public sealed class OutResult
{
    public int Error;
    public byte[] Bytes = new byte[0];

    /// <summary>The answer validated as a document.</summary>
    public bool TryValue(out Value root)
    {
        root = default;
        return Error == 0 && Value.TryOpen(Bytes, out root);
    }
}

public static unsafe class ValueCall
{
    public const int TooLarge = -100;

    /// <summary>Calls an import that fills (output, cap) and answers with the length it needs: with
    /// <paramref name="initial"/> bytes, then once more with exactly the needed size when the answer did not fit.</summary>
    public static OutResult Out(int initial, ValueFill call)
    {
        OutResult result = new OutResult();
        byte[] buf = new byte[Math.Max(initial, 1)];
        int n;
        fixed (byte* p = buf) n = call(p, buf.Length);
        if (n < 0)
        {
            result.Error = n;
            return result;
        }
        if (n > buf.Length)
        {
            if (n > ValueLimits.MaxBytes)
            {
                result.Error = TooLarge;
                return result;
            }
            buf = new byte[n];
            fixed (byte* p = buf) n = call(p, buf.Length);
            if (n < 0)
            {
                result.Error = n;
                return result;
            }
            if (n > buf.Length)
            {
                result.Error = TooLarge;
                return result;
            }
        }
        result.Bytes = new byte[n];
        Array.Copy(buf, result.Bytes, n);
        return result;
    }
}
