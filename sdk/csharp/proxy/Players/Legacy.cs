using Basalt.Proxy.Core;
using Basalt.Proxy.Internal;
namespace Basalt.Proxy.Players;
public static partial class Players {
    public static string PlayerUuid(int player) => ProxyNative.PlayerUuid(player);
    public static string PlayerServer(int player) => ProxyNative.PlayerServer(player);
    public static int PlayerPing(int player) => ProxyNative.PlayerPing(player);
    public static string PlayerSettingStr(int player, string field) => ProxyNative.PlayerSettingStr(player, field);
    public static long? PlayerSettingI64(int player, string field) => ProxyNative.PlayerSettingI64(player, field);
    public static bool PlayerHasPermission(int player, string node) => ProxyNative.PlayerHasPermission(player, node);
    public static int PlayerTitle(int player, string title, string subtitle, int fadeIn = -1, int stay = -1, int fadeOut = -1) => ProxyNative.PlayerTitle(player, title, subtitle, fadeIn, stay, fadeOut);
    public static int PlayerConnect(int player, string server) => ProxyNative.PlayerConnect(player, server);
    public static int PlayerKick(int player, string reason, Args args = null) => ProxyNative.PlayerKick(player, reason, args);
}
