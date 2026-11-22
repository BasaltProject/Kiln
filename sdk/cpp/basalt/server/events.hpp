#include <basalt/server.h>
#pragma once
#include <basalt/server/core.hpp>
#include <basalt/server/events.h>

namespace basalt::server::events {
using namespace basalt::server::core;

inline int on(const char* event, Priority priority, Handler handler) { return bs_on(event, priority, handler); }

inline Player event_player(Ctx ctx) { return bs_event_player(ctx); }

inline Text<256> event_field_str(Ctx ctx, const char* field) {
    return core::detail::fill<256>([&](char* out, int cap) { return bs_event_field_str(ctx, field, out, cap); });
}

inline int event_field_i64(Ctx ctx, const char* field, long long* out) { return bs_event_field_i64(ctx, field, out); }

inline int event_cancel(Ctx ctx, bool cancelled) { return bs_event_cancel(ctx, cancelled); }
} // namespace basalt::server::events

#include <basalt/server/core.hpp>
#include <basalt/server/events.h>
#include <basalt/server/values.hpp>
// Events area for C++: the C layer of wrappers/events.h with Value answers and bool results.
#pragma once

namespace basalt::server::events {
using namespace basalt::server::core;
using namespace basalt::server::values;

enum EventFlag { kEventPost = BASALT_EVENT_POST, kEventCancelled = BASALT_EVENT_CANCELLED };

namespace detail {
template <typename Call>
int value_of(Call call, char* buffer, int capacity, Value& out) {
    bs_val raw;
    const int rc = call(raw);
    out = Value{raw};
    (void)buffer;
    (void)capacity;
    return rc;
}
} // namespace detail

/// Listens to `name`; `filters` is a finished map document or null.
inline int event_on_ex(const char* name, Priority priority, int flags, const void* filters, int filters_length,
                       Handler handler) {
    return bs_event_on_ex(name, priority, flags, filters, filters_length, handler);
}

inline Text<64> event_name(Ctx ctx) {
    return core::detail::fill<64>([&](char* out, int cap) { return bs_event_name(ctx, out, cap); });
}

inline int event_get(Ctx ctx, const char* field, char* buffer, int capacity, Value& out) {
    return detail::value_of([&](bs_val& raw) { return bs_event_get(ctx, field, buffer, capacity, &raw); }, buffer,
                            capacity, out);
}

inline int event_set(Ctx ctx, const char* field, const void* value, int length) {
    return bs_event_set(ctx, field, value, length);
}

inline int event_fields(Ctx ctx, char* buffer, int capacity, Value& out) {
    return detail::value_of([&](bs_val& raw) { return bs_event_fields(ctx, buffer, capacity, &raw); }, buffer, capacity,
                            out);
}

inline int event_payload(Ctx ctx, char* buffer, int capacity, Value& out) {
    return detail::value_of([&](bs_val& raw) { return bs_event_payload(ctx, buffer, capacity, &raw); }, buffer,
                            capacity, out);
}

inline int event_describe(const char* name, char* buffer, int capacity, Value& out) {
    return detail::value_of([&](bs_val& raw) { return bs_event_describe(name, buffer, capacity, &raw); }, buffer,
                            capacity, out);
}

inline int event_names(char* buffer, int capacity, Value& out) {
    return detail::value_of([&](bs_val& raw) { return bs_event_names(buffer, capacity, &raw); }, buffer, capacity, out);
}

inline int event_cancelled(Ctx ctx) { return bs_event_cancelled(ctx); }

inline int event_cancel_with(Ctx ctx, const char* text, const Args* args = nullptr) {
    return bs_event_cancel_with(ctx, text, core::detail::raw(args));
}

inline Text<256> event_cancel_message(Ctx ctx) {
    return core::detail::fill<256>([&](char* out, int cap) { return bs_event_cancel_message(ctx, out, cap); });
}

inline int event_is_post(Ctx ctx) { return bs_event_is_post(ctx); }

inline int event_emit(const char* name, const void* value, int length) { return bs_event_emit(name, value, length); }

inline Player player_by_uuid(const char* uuid) { return bs_player_by_uuid(uuid); }

inline Text<64> player_uuid(Player player) {
    return core::detail::fill<64>([&](char* out, int cap) { return bs_player_uuid(player, out, cap); });
}

inline int player_get(Player player, const char* name, char* buffer, int capacity, Value& out) {
    return detail::value_of([&](bs_val& raw) { return bs_player_get(player, name, buffer, capacity, &raw); }, buffer,
                            capacity, out);
}

inline int player_set(Player player, const char* name, const void* value, int length) {
    return bs_player_set(player, name, value, length);
}

inline int player_call(Player player, const char* op, const void* args, int args_length, char* buffer, int capacity,
                       Value& out) {
    return detail::value_of(
        [&](bs_val& raw) { return bs_player_call(player, op, args, args_length, buffer, capacity, &raw); }, buffer,
        capacity, out);
}

inline int player_describe(char* buffer, int capacity, Value& out) {
    return detail::value_of([&](bs_val& raw) { return bs_player_describe(buffer, capacity, &raw); }, buffer, capacity,
                            out);
}

inline int player_health(Player player, double* out) { return bs_player_health(player, out); }

inline int player_set_health(Player player, double health) { return bs_player_set_health(player, health); }

inline Text<16> player_gamemode(Player player) {
    return core::detail::fill<16>([&](char* out, int cap) { return bs_player_gamemode(player, out, cap); });
}

inline int player_set_gamemode(Player player, const char* mode) { return bs_player_set_gamemode(player, mode); }

inline int player_teleport(Player player, const char* world, double x, double y, double z, float yaw, float pitch) {
    return bs_player_teleport(player, world, x, y, z, yaw, pitch);
}

inline int player_title(Player player, const char* title, const char* subtitle, int fade_in, int stay, int fade_out,
                        const Args* args = nullptr) {
    return bs_player_title(player, title, subtitle, fade_in, stay, fade_out, core::detail::raw(args));
}

inline int player_kick(Player player, const char* text, const Args* args = nullptr) {
    return bs_player_kick(player, text, core::detail::raw(args));
}

inline int player_effect(Player player, const char* effect, int ticks, int amplifier) {
    return bs_player_effect(player, effect, ticks, amplifier);
}

inline int player_open_inventory(Player player, const char* kind, const void* spec, int length) {
    return bs_player_open_inventory(player, kind, spec, length);
}

inline int player_close_window(Player player) { return bs_player_close_window(player); }

inline int form_open(Player player, const void* form, int length, Handler handler) {
    return bs_form_open(player, form, length, handler);
}

inline int form_close(Player player) { return bs_form_close(player); }

inline int dialog_show(Player player, const char* dialog, Handler handler) {
    return bs_dialog_show(player, dialog, handler);
}

inline int dialog_clear(Player player) { return bs_dialog_clear(player); }

inline int completion_value(Ctx ctx, char* buffer, int capacity, Value& out) {
    return detail::value_of([&](bs_val& raw) { return bs_completion_value(ctx, buffer, capacity, &raw); }, buffer,
                            capacity, out);
}

} // namespace basalt::server::events
