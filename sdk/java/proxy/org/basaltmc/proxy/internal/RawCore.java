package org.basaltmc.proxy.internal;

import org.teavm.interop.Address;
import org.teavm.interop.Import;

public final class RawCore {
    @Import(name = "basalt_log") public static native void rawLog(int level, Address msg, int len);
    @Import(name = "basalt_config_i64") public static native long rawConfigI64(Address key, int klen, long def);
    @Import(name = "basalt_config_f64") public static native double rawConfigF64(Address key, int klen, double def);
    @Import(name = "basalt_config_bool") public static native int rawConfigBool(Address key, int klen, int def);
    @Import(name = "basalt_config_str") public static native int rawConfigStr(Address key, int klen, Address out, int cap);
    @Import(name = "basalt_translate")
    public static native int rawTranslate(Address key, int klen, Address args, int alen, Address out, int cap);
    @Import(name = "basalt_player_by_name") public static native int rawPlayerByName(Address name, int len);
    @Import(name = "basalt_players_online") public static native int rawPlayersOnline(Address out, int cap);
    @Import(name = "basalt_player_name") public static native int rawPlayerName(int player, Address out, int cap);
    @Import(name = "basalt_player_send")
    public static native int rawPlayerSend(int player, Address text, int tlen, Address args, int alen);
    @Import(name = "basalt_player_action_bar")
    public static native int rawPlayerActionBar(int player, Address text, int tlen, Address args, int alen);
    @Import(name = "basalt_data_get_i64")
    public static native long rawDataGetI64(int player, Address key, int klen, long def);
    @Import(name = "basalt_data_set_i64")
    public static native int rawDataSetI64(int player, Address key, int klen, long value);
    @Import(name = "basalt_data_get_bool")
    public static native int rawDataGetBool(int player, Address key, int klen, int def);
    @Import(name = "basalt_data_set_bool")
    public static native int rawDataSetBool(int player, Address key, int klen, int value);
    @Import(name = "basalt_data_get_str")
    public static native int rawDataGetStr(int player, Address key, int klen, Address out, int cap);
    @Import(name = "basalt_data_set_str")
    public static native int rawDataSetStr(int player, Address key, int klen, Address value, int vlen);
    @Import(name = "basalt_data_remove") public static native int rawDataRemove(int player, Address key, int klen);
    @Import(name = "basalt_event_on")
    public static native int rawEventOn(Address name, int nlen, int priority, int token);
    @Import(name = "basalt_event_player") public static native int rawEventPlayer(int ctx);
    @Import(name = "basalt_event_field_str")
    public static native int rawEventFieldStr(int ctx, Address field, int flen, Address out, int cap);
    @Import(name = "basalt_event_field_i64")
    public static native int rawEventFieldI64(int ctx, Address field, int flen, Address out);
    @Import(name = "basalt_event_cancel") public static native int rawEventCancel(int ctx, int cancelled);
    @Import(name = "basalt_command_register")
    public static native int rawCommandRegister(Address spec, int slen, Address permission, int plen, int token);
    @Import(name = "basalt_command_source_name")
    public static native int rawCommandSourceName(int ctx, Address out, int cap);
    @Import(name = "basalt_command_is_console") public static native int rawCommandIsConsole(int ctx);
    @Import(name = "basalt_command_reply")
    public static native int rawCommandReply(int ctx, Address text, int tlen, Address args, int alen);
    @Import(name = "basalt_command_arg_str")
    public static native int rawCommandArgStr(int ctx, Address name, int nlen, Address out, int cap);
    @Import(name = "basalt_command_arg_i64")
    public static native int rawCommandArgI64(int ctx, Address name, int nlen, Address out);
    @Import(name = "basalt_task_after") public static native int rawTaskAfter(int delay, int token);
    @Import(name = "basalt_task_every") public static native int rawTaskEvery(int delay, int period, int token);
    @Import(name = "basalt_task_cancel") public static native int rawTaskCancel(int task);
}
