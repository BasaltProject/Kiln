/* Player handles and messages, shared by both profiles. */
#ifndef BASALT_INTERNAL_PLAYERS_H
#define BASALT_INTERNAL_PLAYERS_H

#include "basalt/internal/core.h"

#ifdef __cplusplus
extern "C" {
#endif

bs_player bs_player_by_name(const char *name);
int bs_players_online(bs_player *out, int cap);
int bs_player_name(bs_player player, char *out, int cap);
int bs_send(bs_player player, const char *text, const bs_args *args);
int bs_action_bar(bs_player player, const char *text, const bs_args *args);

#ifdef __cplusplus
}
#endif

#endif
