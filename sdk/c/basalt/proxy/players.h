/* Player handles, messages and the proxy player calls: uuid, server, ping, settings, title, connect, kick. */
#ifndef BASALT_PROXY_PLAYERS_H
#define BASALT_PROXY_PLAYERS_H

#include "basalt/internal/players.h"
#include "basalt_proxy_abi.h"

#ifdef __cplusplus
extern "C" {
#endif

int bs_proxy_player_uuid(bs_player player, char *out, int cap);
int bs_proxy_player_server(bs_player player, char *out, int cap);
int bs_proxy_player_ping(bs_player player);
int bs_proxy_player_setting_str(bs_player player, const char *field, char *out, int cap);
int bs_proxy_player_setting_i64(bs_player player, const char *field, int64_t *out);
int bs_proxy_player_has_permission(bs_player player, const char *node);
int bs_proxy_player_title(bs_player player, const char *title, const char *subtitle, int fade_in, int stay,
                          int fade_out);
int bs_proxy_player_connect(bs_player player, const char *server);
int bs_proxy_player_kick(bs_player player, const char *reason, const bs_args *args);
int bs_proxy_players_on(const char *server, bs_player *out, int cap);

#ifdef __cplusplus
}
#endif

#endif
