/* Players: handles, messages, effects and the player calls of the events area; values come back as bs_val. */
#ifndef BASALT_SERVER_PLAYERS_H
#define BASALT_SERVER_PLAYERS_H

#include "basalt/internal/players.h"
#include "basalt/server/events.h"

#ifdef __cplusplus
extern "C" {
#endif

int bs_particle(bs_player player, const char *id, int count, double spread);
int bs_sound(bs_player player, const char *id, float volume, float pitch);

#ifdef __cplusplus
}
#endif

static inline int32_t bs_ev_player_get_fn(void *user, char *out, int32_t cap) {
    const bs_ev_call *c = (const bs_ev_call *)user;
    return basalt_player_get(c->a, c->name, c->name_len, out, cap);
}
static inline int32_t bs_ev_player_describe_fn(void *user, char *out, int32_t cap) {
    (void)user;
    return basalt_player_describe(out, cap);
}

typedef struct {
    int32_t a;
    const char *name;
    int32_t name_len;
    const char *args;
    int32_t args_len;
} bs_ev_op;

static inline int32_t bs_ev_player_call_fn(void *user, char *out, int32_t cap) {
    const bs_ev_op *c = (const bs_ev_op *)user;
    return basalt_player_call(c->a, c->name, c->name_len, c->args, c->args_len, out, cap);
}

static inline bs_player bs_player_by_uuid(const char *uuid) { return basalt_player_by_uuid(uuid, bs_ev_len(uuid)); }

static inline int bs_player_uuid(bs_player player, char *out, int cap) {
    return bs_ev_text(basalt_player_uuid(player, out, cap), out, cap);
}

static inline int bs_player_get(bs_player player, const char *name, char *buf, int cap, bs_val *root) {
    bs_ev_call c = {player, 0, name, bs_ev_len(name), 0};
    return bs_out_value(bs_ev_player_get_fn, &c, buf, cap, 0, 0, root);
}

static inline int bs_player_set(bs_player player, const char *name, const void *value, int len) {
    return basalt_player_set(player, name, bs_ev_len(name), (const char *)value, len);
}

/* `args` is a finished list document or 0 for none. */
static inline int bs_player_call(bs_player player, const char *op, const void *args, int args_len, char *buf, int cap,
                                 bs_val *root) {
    bs_ev_op c = {player, op, bs_ev_len(op), (const char *)args, args_len};
    return bs_out_value(bs_ev_player_call_fn, &c, buf, cap, 0, 0, root);
}

static inline int bs_player_describe(char *buf, int cap, bs_val *root) {
    return bs_out_value(bs_ev_player_describe_fn, 0, buf, cap, 0, 0, root);
}

static inline int bs_player_health(bs_player player, double *out) { return basalt_player_health(player, out); }

static inline int bs_player_set_health(bs_player player, double health) {
    return basalt_player_set_health(player, health);
}

static inline int bs_player_gamemode(bs_player player, char *out, int cap) {
    return bs_ev_text(basalt_player_gamemode(player, out, cap), out, cap);
}

static inline int bs_player_set_gamemode(bs_player player, const char *mode) {
    return basalt_player_set_gamemode(player, mode, bs_ev_len(mode));
}

static inline int bs_player_teleport(bs_player player, const char *world, double x, double y, double z, float yaw,
                                     float pitch) {
    return basalt_player_teleport(player, world, bs_ev_len(world), x, y, z, yaw, pitch);
}

static inline int bs_player_title(bs_player player, const char *title, const char *subtitle, int fade_in, int stay,
                                  int fade_out, const bs_args *args) {
    return basalt_player_title(player, title, bs_ev_len(title), subtitle, bs_ev_len(subtitle), fade_in, stay, fade_out,
                               bs_ev_args(args), bs_ev_args_len(args));
}

static inline int bs_player_kick(bs_player player, const char *text, const bs_args *args) {
    return basalt_player_kick(player, text, bs_ev_len(text), bs_ev_args(args), bs_ev_args_len(args));
}

static inline int bs_player_effect(bs_player player, const char *effect, int ticks, int amplifier) {
    return basalt_player_effect(player, effect, bs_ev_len(effect), ticks, amplifier);
}

static inline int bs_player_open_inventory(bs_player player, const char *kind, const void *spec, int spec_len) {
    return basalt_player_open_inventory(player, kind, bs_ev_len(kind), (const char *)spec, spec_len);
}

static inline int bs_player_close_window(bs_player player) { return basalt_player_close_window(player); }

#endif
