#include <basalt/server.h>
#pragma once
#include <basalt/server/core.h>

namespace basalt::server::core {
using Player = bs_player;
using Ctx = bs_ctx;
using Handler = bs_handler;

enum Level { kDebug = BASALT_LOG_DEBUG, kInfo = BASALT_LOG_INFO, kWarn = BASALT_LOG_WARN, kError = BASALT_LOG_ERROR };

enum Priority {
    kLowest = BASALT_PRIORITY_LOWEST,
    kLow = BASALT_PRIORITY_LOW,
    kNormal = BASALT_PRIORITY_NORMAL,
    kHigh = BASALT_PRIORITY_HIGH,
    kHighest = BASALT_PRIORITY_HIGHEST,
};

struct Args {
    bs_args raw;

    Args() { bs_args_init(&raw); }

    Args& add(const char* key, const char* value) {
        bs_args_add(&raw, key, value);
        return *this;
    }
};

/// A fixed buffer a call fills in; `length` is negative when the call failed and may exceed N when it was cut.
template <int N>
struct Text {
    char data[N + 1] = {};
    int length = 0;

    bool ok() const { return length >= 0; }

    const char* c_str() const { return data; }
};

namespace detail {
inline const bs_args* raw(const Args* args) { return args == nullptr ? nullptr : &args->raw; }

template <int N, typename Call>
Text<N> fill(Call call) {
    Text<N> text;
    text.length = call(text.data, N);
    if (text.length >= 0 && text.length <= N) {
        text.data[text.length] = 0;
    } else if (text.length > N) {
        text.data[N] = 0;
    }
    return text;
}
} // namespace detail

inline int format_i64(long long value, char* out) { return bs_format_i64(value, out); }

/// Drops a pending request (a wait, a lookup, a service call, a world request) so its callback never runs.
inline int request_cancel(int request) { return bs_request_cancel(request); }
} // namespace basalt::server::core
