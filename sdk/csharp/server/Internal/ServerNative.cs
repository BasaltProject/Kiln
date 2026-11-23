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

namespace Basalt.Server.Internal;

internal static unsafe partial class Native {
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "player_particle")]
    internal static extern int RawPlayerParticle(int player, byte* id, int idlen, int count, double spread);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "player_sound")]
    internal static extern int RawPlayerSound(int player, byte* id, int idlen, float volume, float pitch);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "store_top")]
    internal static extern int RawStoreTop(byte* key, int klen, int limit, byte* output, int cap);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "placeholder_register")]
    internal static extern int RawPlaceholderRegister(byte* name, int nlen, int token);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "placeholder_player")]
    internal static extern int RawPlaceholderPlayer(int ctx);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "placeholder_result")]
    internal static extern int RawPlaceholderResult(int ctx, byte* text, int len);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "menu_create")]
    internal static extern int RawMenuCreate(byte* title, int tlen, int rows);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "menu_set")]
    internal static extern int RawMenuSet(int menu, int slot, byte* item, int ilen, byte* name, int nlen, byte* lore, int llen, int token);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "menu_open")]
    internal static extern int RawMenuOpen(int menu, int player);
    [WasmImportLinkage, DllImport("basalt", EntryPoint = "menu_click_player")]
    internal static extern int RawMenuClickPlayer(int ctx);
}
