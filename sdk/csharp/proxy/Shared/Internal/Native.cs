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

namespace Basalt.Proxy.Internal;

internal static unsafe partial class Native {
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "log")]
    internal static extern void RawLog(int level, byte* msg, int len);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "config_i64")]
    internal static extern long RawConfigI64(byte* key, int klen, long def);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "config_f64")]
    internal static extern double RawConfigF64(byte* key, int klen, double def);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "config_bool")]
    internal static extern int RawConfigBool(byte* key, int klen, int def);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "config_str")]
    internal static extern int RawConfigStr(byte* key, int klen, byte* output, int cap);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "translate")]
    internal static extern int RawTranslate(byte* key, int klen, byte* args, int alen, byte* output, int cap);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "player_by_name")]
    internal static extern int RawPlayerByName(byte* name, int len);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "players_online")]
    internal static extern int RawPlayersOnline(int* output, int cap);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "player_name")]
    internal static extern int RawPlayerName(int player, byte* output, int cap);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "player_send")]
    internal static extern int RawPlayerSend(int player, byte* text, int tlen, byte* args, int alen);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "player_action_bar")]
    internal static extern int RawPlayerActionBar(int player, byte* text, int tlen, byte* args, int alen);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "data_get_i64")]
    internal static extern long RawDataGetI64(int player, byte* key, int klen, long def);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "data_set_i64")]
    internal static extern int RawDataSetI64(int player, byte* key, int klen, long value);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "data_get_bool")]
    internal static extern int RawDataGetBool(int player, byte* key, int klen, int def);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "data_set_bool")]
    internal static extern int RawDataSetBool(int player, byte* key, int klen, int value);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "data_get_str")]
    internal static extern int RawDataGetStr(int player, byte* key, int klen, byte* output, int cap);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "data_set_str")]
    internal static extern int RawDataSetStr(int player, byte* key, int klen, byte* value, int vlen);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "data_remove")]
    internal static extern int RawDataRemove(int player, byte* key, int klen);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "event_on")]
    internal static extern int RawEventOn(byte* name, int nlen, int priority, int token);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "event_player")]
    internal static extern int RawEventPlayer(int ctx);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "event_field_str")]
    internal static extern int RawEventFieldStr(int ctx, byte* field, int flen, byte* output, int cap);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "event_field_i64")]
    internal static extern int RawEventFieldI64(int ctx, byte* field, int flen, long* output);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "event_cancel")]
    internal static extern int RawEventCancel(int ctx, int cancelled);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "command_register")]
    internal static extern int RawCommandRegister(byte* spec, int slen, byte* permission, int plen, int token);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "command_source_name")]
    internal static extern int RawCommandSourceName(int ctx, byte* output, int cap);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "command_is_console")]
    internal static extern int RawCommandIsConsole(int ctx);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "command_reply")]
    internal static extern int RawCommandReply(int ctx, byte* text, int tlen, byte* args, int alen);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "command_arg_str")]
    internal static extern int RawCommandArgStr(int ctx, byte* name, int nlen, byte* output, int cap);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "command_arg_i64")]
    internal static extern int RawCommandArgI64(int ctx, byte* name, int nlen, long* output);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "task_after")]
    internal static extern int RawTaskAfter(int delay, int token);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "task_every")]
    internal static extern int RawTaskEvery(int delay, int period, int token);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "task_cancel")]
    internal static extern int RawTaskCancel(int task);
    internal static readonly List<Handler> handlers = new();
    internal static int TokenFor(Handler handler)
    {
        handlers.Add(handler);
        return handlers.Count;
    }
    internal static void Dispatch(int token, int ctx)
    {
        if (token > 0 && token <= handlers.Count)
        {
            handlers[token - 1](ctx);
        }
    }
    internal static byte[] Utf8(string text) => Encoding.UTF8.GetBytes(text);
    internal delegate int Fill(byte* output, int cap);
    internal static string Fetch(Fill call)
    {
        int cap = 256;
        while (true)
        {
            byte[] buffer = new byte[cap];
            int n;
            fixed (byte* p = buffer)
            {
                n = call(p, cap);
            }
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
}
