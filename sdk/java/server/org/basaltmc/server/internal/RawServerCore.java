package org.basaltmc.server.internal;

import org.teavm.interop.Address;
import org.teavm.interop.Import;

public final class RawServerCore {
    @Import(name = "basalt_player_particle")
    public static native int rawPlayerParticle(int player, Address id, int idlen, int count, double spread);
    @Import(name = "basalt_player_sound")
    public static native int rawPlayerSound(int player, Address id, int idlen, float volume, float pitch);
    @Import(name = "basalt_store_top")
    public static native int rawStoreTop(Address key, int klen, int limit, Address out, int cap);
    @Import(name = "basalt_placeholder_register")
    public static native int rawPlaceholderRegister(Address name, int nlen, int token);
    @Import(name = "basalt_placeholder_player") public static native int rawPlaceholderPlayer(int ctx);
    @Import(name = "basalt_placeholder_result")
    public static native int rawPlaceholderResult(int ctx, Address text, int len);
    @Import(name = "basalt_menu_create") public static native int rawMenuCreate(Address title, int tlen, int rows);
    @Import(name = "basalt_menu_set")
    public static native int rawMenuSet(int menu, int slot, Address item, int ilen, Address name, int nlen,
                                         Address lore, int llen, int token);
    @Import(name = "basalt_menu_open") public static native int rawMenuOpen(int menu, int player);
    @Import(name = "basalt_menu_click_player") public static native int rawMenuClickPlayer(int ctx);
}
