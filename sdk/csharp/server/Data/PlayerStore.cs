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

namespace Basalt.Server.Data;

public static unsafe partial class PlayerStore {
    public static List<TopEntry> StoreTop(string key, int limit)
    {
        byte[] k = Utf8(key);
        int cap = 1024;
        while (true)
        {
            byte[] buffer = new byte[cap];
            int n;
            fixed (byte* kp = k)
            fixed (byte* bp = buffer)
                n = RawStoreTop(kp, k.Length, limit, bp, cap);
            if (n > cap)
            {
                cap = n;
                continue;
            }
            List<TopEntry> rows = new();
            int at = 0;
            while (n > 0 && at + 4 <= n)
            {
                int length = buffer[at] | (buffer[at + 1] << 8) | (buffer[at + 2] << 16) | (buffer[at + 3] << 24);
                at += 4;
                if (at + length + 8 > n)
                {
                    break;
                }
                string name = Encoding.UTF8.GetString(buffer, at, length);
                at += length;
                long value = 0;
                for (int i = 7; i >= 0; i--)
                {
                    value = (value << 8) | buffer[at + i];
                }
                at += 8;
                rows.Add(new TopEntry(name, value));
            }
            return rows;
        }
    }
}
