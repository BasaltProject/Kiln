#include "runtime.h"

#include "basalt/proxy/admin.h"
#include "basalt/proxy/events.h"
#include "basalt/proxy/players.h"

int bs_proxy_event_set_str(bs_ctx ctx, const char *field, const char *value) {
    return basalt_proxy_event_set_str(ctx, field, bs_length(field), value, bs_length(value));
}

int bs_proxy_event_set_i64(bs_ctx ctx, const char *field, int64_t value) {
    return basalt_proxy_event_set_i64(ctx, field, bs_length(field), value);
}

int bs_proxy_player_uuid(bs_player player, char *out, int cap) { return basalt_proxy_player_uuid(player, out, cap); }

int bs_proxy_player_server(bs_player player, char *out, int cap) {
    return basalt_proxy_player_server(player, out, cap);
}

int bs_proxy_player_ping(bs_player player) { return basalt_proxy_player_ping(player); }

int bs_proxy_player_setting_str(bs_player player, const char *field, char *out, int cap) {
    return basalt_proxy_player_setting_str(player, field, bs_length(field), out, cap);
}

int bs_proxy_player_setting_i64(bs_player player, const char *field, int64_t *out) {
    return basalt_proxy_player_setting_i64(player, field, bs_length(field), out);
}

int bs_proxy_player_has_permission(bs_player player, const char *node) {
    return basalt_proxy_player_has_permission(player, node, bs_length(node));
}

int bs_proxy_player_title(bs_player player, const char *title, const char *subtitle, int fade_in, int stay,
                          int fade_out) {
    return basalt_proxy_player_title(player, title, bs_length(title), subtitle, bs_length(subtitle), fade_in, stay,
                                     fade_out);
}

int bs_proxy_player_connect(bs_player player, const char *server) {
    return basalt_proxy_player_connect(player, server, bs_length(server));
}

int bs_proxy_player_kick(bs_player player, const char *reason, const bs_args *args) {
    return basalt_proxy_player_kick(player, reason, bs_length(reason), ARGS_PTR(args), ARGS_LEN(args));
}

int bs_proxy_servers_list(char *out, int cap) { return basalt_proxy_servers_list(out, cap); }

int bs_proxy_server_address(const char *name, char *out, int cap) {
    return basalt_proxy_server_address(name, bs_length(name), out, cap);
}

int bs_proxy_players_on(const char *server, bs_player *out, int cap) {
    return basalt_proxy_players_on(server, bs_length(server), out, cap);
}
