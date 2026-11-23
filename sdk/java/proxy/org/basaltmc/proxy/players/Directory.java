package org.basaltmc.proxy.players;

import org.basaltmc.proxy.core.*;
import org.basaltmc.proxy.internal.Proxy;

public final class Directory extends SharedDirectory {

    public static String playerUuid(int player) { return Proxy.playerUuid(player); }
    public static String playerServer(int player) { return Proxy.playerServer(player); }
    public static int playerPing(int player) { return Proxy.playerPing(player); }
    public static String playerSettingStr(int player, String field) { return Proxy.playerSettingStr(player, field); }
    public static Long playerSettingI64(int player, String field) { return Proxy.playerSettingI64(player, field); }
    public static boolean playerHasPermission(int player, String node) { return Proxy.playerHasPermission(player, node); }
    public static int playerTitle(int player, String title, String subtitle, int fadeIn, int stay, int fadeOut) { return Proxy.playerTitle(player, title, subtitle, fadeIn, stay, fadeOut); }
    public static int playerConnect(int player, String server) { return Proxy.playerConnect(player, server); }
    public static int playerKick(int player, String reason, Args args) { return Proxy.playerKick(player, reason, args); }
    public static int playerTitle(int player, String title, String subtitle) { return playerTitle(player, title, subtitle, -1, -1, -1); }
}
