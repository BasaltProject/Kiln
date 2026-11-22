package org.basaltmc.proxy.players;
import org.basaltmc.proxy.core.*;

public final class Players extends SharedPlayers {

    public static String playerUuid(int player) { return org.basaltmc.proxy.internal.Proxy.playerUuid(player); }
    public static String playerServer(int player) { return org.basaltmc.proxy.internal.Proxy.playerServer(player); }
    public static int playerPing(int player) { return org.basaltmc.proxy.internal.Proxy.playerPing(player); }
    public static String playerSettingStr(int player, String field) { return org.basaltmc.proxy.internal.Proxy.playerSettingStr(player, field); }
    public static Long playerSettingI64(int player, String field) { return org.basaltmc.proxy.internal.Proxy.playerSettingI64(player, field); }
    public static boolean playerHasPermission(int player, String node) { return org.basaltmc.proxy.internal.Proxy.playerHasPermission(player, node); }
    public static int playerTitle(int player, String title, String subtitle, int fadeIn, int stay, int fadeOut) { return org.basaltmc.proxy.internal.Proxy.playerTitle(player, title, subtitle, fadeIn, stay, fadeOut); }
    public static int playerConnect(int player, String server) { return org.basaltmc.proxy.internal.Proxy.playerConnect(player, server); }
    public static int playerKick(int player, String reason, Args args) { return org.basaltmc.proxy.internal.Proxy.playerKick(player, reason, args); }
    public static int playerTitle(int player, String title, String subtitle) { return playerTitle(player, title, subtitle, -1, -1, -1); }
}
