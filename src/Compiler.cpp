#include "Compiler.hpp"

extern "C" {
#include <lauxlib.h>
#include <lua.h>
}

#include <memory>

namespace basalt::kiln {
namespace {

struct StateCloser {
  void operator()(lua_State *state) const { lua_close(state); }
};

int append(lua_State *, const void *chunk, std::size_t size, void *out) {
  static_cast<std::string *>(out)->append(static_cast<const char *>(chunk),
                                          size);
  return 0;
}

} // namespace

std::expected<std::string, std::string> compile(std::string_view source,
                                                std::string_view chunkName,
                                                const Options &options) {
  const std::unique_ptr<lua_State, StateCloser> state{luaL_newstate()};
  if (state == nullptr) {
    return std::unexpected("out of memory");
  }
  const std::string named = "@" + std::string{chunkName};
  if (luaL_loadbufferx(state.get(), source.data(), source.size(), named.c_str(),
                       "t") != LUA_OK) {
    const char *message = lua_tostring(state.get(), -1);
    return std::unexpected(message == nullptr ? "syntax error" : message);
  }
  std::string bytecode;
  if (lua_dump(state.get(), &append, &bytecode, options.strip ? 1 : 0) != 0) {
    return std::unexpected("could not dump the bytecode");
  }
  return bytecode;
}

} // namespace basalt::kiln
