using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using Basalt.Proxy.Core;
using Basalt.Proxy.Internal;
using Basalt.Proxy.Values;
using static Basalt.Proxy.Internal.Native;
using static Basalt.Proxy.Core.Level;
using static Basalt.Proxy.Core.Priority;
namespace Basalt.Proxy.Internal;

    internal static unsafe class ProxyNative
    {
        [WasmImportLinkage, DllImport("basalt_proxy", EntryPoint = "event_set_str")]
        static extern int RawEventSetStr(int ctx, byte* field, int flen, byte* value, int vlen);
        [WasmImportLinkage, DllImport("basalt_proxy", EntryPoint = "event_set_i64")]
        static extern int RawEventSetI64(int ctx, byte* field, int flen, long value);
        [WasmImportLinkage, DllImport("basalt_proxy", EntryPoint = "player_uuid")]
        static extern int RawPlayerUuid(int player, byte* output, int cap);
        [WasmImportLinkage, DllImport("basalt_proxy", EntryPoint = "player_server")]
        static extern int RawPlayerServer(int player, byte* output, int cap);
        [WasmImportLinkage, DllImport("basalt_proxy", EntryPoint = "player_ping")]
        static extern int RawPlayerPing(int player);
        [WasmImportLinkage, DllImport("basalt_proxy", EntryPoint = "player_setting_str")]
        static extern int RawPlayerSettingStr(int player, byte* field, int flen, byte* output, int cap);
        [WasmImportLinkage, DllImport("basalt_proxy", EntryPoint = "player_setting_i64")]
        static extern int RawPlayerSettingI64(int player, byte* field, int flen, long* output);
        [WasmImportLinkage, DllImport("basalt_proxy", EntryPoint = "player_has_permission")]
        static extern int RawPlayerHasPermission(int player, byte* node, int nlen);
        [WasmImportLinkage, DllImport("basalt_proxy", EntryPoint = "player_title")]
        static extern int RawPlayerTitle(int player, byte* title, int tlen, byte* subtitle, int slen, int fadeIn, int stay, int fadeOut);
        [WasmImportLinkage, DllImport("basalt_proxy", EntryPoint = "player_connect")]
        static extern int RawPlayerConnect(int player, byte* server, int slen);
        [WasmImportLinkage, DllImport("basalt_proxy", EntryPoint = "player_kick")]
        static extern int RawPlayerKick(int player, byte* reason, int rlen, byte* args, int alen);
        [WasmImportLinkage, DllImport("basalt_proxy", EntryPoint = "servers_list")]
        static extern int RawServersList(byte* output, int cap);
        [WasmImportLinkage, DllImport("basalt_proxy", EntryPoint = "server_address")]
        static extern int RawServerAddress(byte* name, int nlen, byte* output, int cap);
        [WasmImportLinkage, DllImport("basalt_proxy", EntryPoint = "players_on")]
        static extern int RawPlayersOn(byte* server, int slen, int* output, int cap);

        public static int EventSetStr(int ctx, string field, string value)
        {
            byte[] f = Utf8(field);
            byte[] v = Utf8(value);
            fixed (byte* fp = f)
            fixed (byte* vp = v)
                return RawEventSetStr(ctx, fp, f.Length, vp, v.Length);
        }
        public static int EventSetI64(int ctx, string field, long value)
        {
            byte[] f = Utf8(field);
            fixed (byte* fp = f)
                return RawEventSetI64(ctx, fp, f.Length, value);
        }

        public static string PlayerUuid(int player) => Fetch((o, c) => RawPlayerUuid(player, o, c));
        public static string PlayerServer(int player) => Fetch((o, c) => RawPlayerServer(player, o, c));
        public static int PlayerPing(int player) => RawPlayerPing(player);
        public static string PlayerSettingStr(int player, string field)
        {
            byte[] f = Utf8(field);
            return Fetch((o, c) => { fixed (byte* fp = f) return RawPlayerSettingStr(player, fp, f.Length, o, c); });
        }
        public static long? PlayerSettingI64(int player, string field)
        {
            byte[] f = Utf8(field);
            long value = 0;
            fixed (byte* fp = f)
                return RawPlayerSettingI64(player, fp, f.Length, &value) >= 0 ? value : null;
        }
        public static bool PlayerHasPermission(int player, string node)
        {
            byte[] n = Utf8(node);
            fixed (byte* np = n)
                return RawPlayerHasPermission(player, np, n.Length) == 1;
        }
        /// <summary>Times are in ticks; a negative one means the client default.</summary>
        public static int PlayerTitle(int player, string title, string subtitle, int fadeIn = -1, int stay = -1, int fadeOut = -1)
        {
            byte[] t = Utf8(title);
            byte[] s = Utf8(subtitle);
            fixed (byte* tp = t)
            fixed (byte* sp = s)
                return RawPlayerTitle(player, tp, t.Length, sp, s.Length, fadeIn, stay, fadeOut);
        }
        public static int PlayerConnect(int player, string server)
        {
            byte[] s = Utf8(server);
            fixed (byte* sp = s)
                return RawPlayerConnect(player, sp, s.Length);
        }
        public static int PlayerKick(int player, string reason, Args args = null)
        {
            byte[] r = Utf8(reason);
            byte[] a = args?.Packed();
            fixed (byte* rp = r)
            fixed (byte* ap = a)
                return RawPlayerKick(player, rp, r.Length, ap, a == null ? 0 : a.Length);
        }

        public static string[] ServersList()
        {
            string names = Fetch((o, c) => RawServersList(o, c));
            return string.IsNullOrEmpty(names) ? new string[0] : names.Split('\n');
        }
        public static string ServerAddress(string name)
        {
            byte[] n = Utf8(name);
            return Fetch((o, c) => { fixed (byte* np = n) return RawServerAddress(np, n.Length, o, c); });
        }
        public static int[] PlayersOn(string server)
        {
            byte[] s = Utf8(server);
            int cap = 64;
            while (true)
            {
                int[] buffer = new int[cap];
                int n;
                fixed (byte* sp = s)
                fixed (int* p = buffer)
                    n = RawPlayersOn(sp, s.Length, p, cap);
                if (n < 0)
                {
                    return new int[0];
                }
                if (n > cap)
                {
                    cap = n;
                    continue;
                }
                int[] result = new int[n];
                System.Array.Copy(buffer, result, n);
                return result;
            }
        }
    }
