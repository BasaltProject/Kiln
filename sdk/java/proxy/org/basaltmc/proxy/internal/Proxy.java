package org.basaltmc.proxy.internal;
import org.teavm.interop.Address;
import org.teavm.interop.Import;
import org.basaltmc.proxy.core.*;
import static org.basaltmc.proxy.internal.Native.*;
public final class Proxy {

        private Proxy() {}

        @Import(name = "basalt_proxy_event_set_str")
        public static native int rawEventSetStr(int ctx, Address field, int flen, Address value, int vlen);
        @Import(name = "basalt_proxy_event_set_i64")
        public static native int rawEventSetI64(int ctx, Address field, int flen, long value);
        @Import(name = "basalt_proxy_player_uuid") public static native int rawPlayerUuid(int player, Address out, int cap);
        @Import(name = "basalt_proxy_player_server") public static native int rawPlayerServer(int player, Address out, int cap);
        @Import(name = "basalt_proxy_player_ping") public static native int rawPlayerPing(int player);
        @Import(name = "basalt_proxy_player_setting_str")
        public static native int rawPlayerSettingStr(int player, Address field, int flen, Address out, int cap);
        @Import(name = "basalt_proxy_player_setting_i64")
        public static native int rawPlayerSettingI64(int player, Address field, int flen, Address out);
        @Import(name = "basalt_proxy_player_has_permission")
        public static native int rawPlayerHasPermission(int player, Address node, int nlen);
        @Import(name = "basalt_proxy_player_title")
        public static native int rawPlayerTitle(int player, Address title, int tlen, Address subtitle, int slen,
                int fadeIn, int stay, int fadeOut);
        @Import(name = "basalt_proxy_player_connect")
        public static native int rawPlayerConnect(int player, Address server, int slen);
        @Import(name = "basalt_proxy_player_kick")
        public static native int rawPlayerKick(int player, Address reason, int rlen, Address args, int alen);
        @Import(name = "basalt_proxy_servers_list") public static native int rawServersList(Address out, int cap);
        @Import(name = "basalt_proxy_server_address")
        public static native int rawServerAddress(Address name, int nlen, Address out, int cap);
        @Import(name = "basalt_proxy_players_on")
        public static native int rawPlayersOn(Address server, int slen, Address out, int cap);

        public static int eventSetStr(int ctx, String field, String value) {
            byte[] f = utf8(field);
            byte[] v = utf8(value);
            return rawEventSetStr(ctx, at(f), f.length, at(v), v.length);
        }

        public static int eventSetI64(int ctx, String field, long value) {
            byte[] f = utf8(field);
            return rawEventSetI64(ctx, at(f), f.length, value);
        }

        public static String playerUuid(int player) { return fetch((out, cap) -> rawPlayerUuid(player, out, cap)); }

        public static String playerServer(int player) { return fetch((out, cap) -> rawPlayerServer(player, out, cap)); }

        public static int playerPing(int player) { return rawPlayerPing(player); }

        public static String playerSettingStr(int player, String field) {
            byte[] f = utf8(field);
            return fetch((out, cap) -> rawPlayerSettingStr(player, at(f), f.length, out, cap));
        }

        public static Long playerSettingI64(int player, String field) {
            byte[] f = utf8(field);
            long[] out = new long[1];
            return rawPlayerSettingI64(player, at(f), f.length, Address.ofData(out)) >= 0 ? Long.valueOf(out[0]) : null;
        }

        public static boolean playerHasPermission(int player, String node) {
            byte[] n = utf8(node);
            return rawPlayerHasPermission(player, at(n), n.length) == 1;
        }

        /** Times are in ticks; a negative one means the client default. */
        public static int playerTitle(int player, String title, String subtitle, int fadeIn, int stay, int fadeOut) {
            byte[] t = utf8(title);
            byte[] s = utf8(subtitle);
            return rawPlayerTitle(player, at(t), t.length, at(s), s.length, fadeIn, stay, fadeOut);
        }

        public static int playerTitle(int player, String title, String subtitle) {
            return playerTitle(player, title, subtitle, -1, -1, -1);
        }

        public static int playerConnect(int player, String server) {
            byte[] s = utf8(server);
            return rawPlayerConnect(player, at(s), s.length);
        }

        public static int playerKick(int player, String reason, Args args) {
            byte[] r = utf8(reason);
            byte[] a = packed(args);
            return rawPlayerKick(player, at(r), r.length, at(a), a.length);
        }

        public static String[] serversList() {
            String names = fetch((out, cap) -> rawServersList(out, cap));
            if (names == null || names.isEmpty()) {
                return new String[0];
            }
            return names.split("\n");
        }

        public static String serverAddress(String name) {
            byte[] n = utf8(name);
            return fetch((out, cap) -> rawServerAddress(at(n), n.length, out, cap));
        }

        public static int[] playersOn(String server) {
            byte[] s = utf8(server);
            int cap = 64;
            while (true) {
                int[] out = new int[cap];
                int n = rawPlayersOn(at(s), s.length, Address.ofData(out), cap);
                if (n < 0) {
                    return new int[0];
                }
                if (n <= cap) {
                    int[] exact = new int[n];
                    for (int i = 0; i < n; i++) {
                        exact[i] = out[i];
                    }
                    return exact;
                }
                cap = n;
            }
        }

}
