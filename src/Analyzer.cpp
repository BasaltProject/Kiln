#include "Analyzer.hpp"

#include <map>

extern "C" {
#include <lauxlib.h>
#include <ldebug.h>
#include <lobject.h>
#include <lopcodes.h>
#include <lua.h>
}

#include <memory>

namespace basalt::kiln {
namespace {

struct StateCloser {
    void operator()(lua_State* state) const { lua_close(state); }
};

const char* stringConstant(const Proto* proto, int index) {
    const TValue* constant = &proto->k[index];
    return ttisstring(constant) ? getstr(tsvalue(constant)) : nullptr;
}

bool writesEnvironment(const Proto* proto, int upvalue) {
    const TString* name = proto->upvalues[upvalue].name;
    return name != nullptr && std::string_view{getstr(name)} == "_ENV";
}

using Registers = std::map<int, std::string>;

void forget(Registers& registers, int from) { registers.erase(registers.lower_bound(from), registers.end()); }

void follow(Registers& registers, int target, const std::string* base, const char* key) {
    if (base == nullptr || key == nullptr) {
        registers.erase(target);
        return;
    }
    registers[target] = *base + "." + key;
}

void walk(const Proto* proto, Usage& usage) {
    Registers registers;
    for (int pc = 0; pc < proto->sizecode; ++pc) {
        const Instruction instruction = proto->code[pc];
        const OpCode op = GET_OPCODE(instruction);
        const int a = GETARG_A(instruction);
        const int line = luaG_getfuncline(proto, pc);
        switch (op) {
        case OP_GETTABUP: {
            const char* key = stringConstant(proto, GETARG_C(instruction));
            if (writesEnvironment(proto, GETARG_B(instruction)) && key != nullptr) {
                usage.globalReads.push_back({key, line});
                registers[a] = key;
            } else {
                registers.erase(a);
            }
            break;
        }
        case OP_GETFIELD:
        case OP_SELF: {
            const auto base = registers.find(GETARG_B(instruction));
            const std::string* path = base == registers.end() || base->second.front() == '"' ? nullptr : &base->second;
            const char* key = stringConstant(proto, GETARG_C(instruction));
            if (path != nullptr && key != nullptr) {
                usage.fieldReads.push_back({*path + "." + key, line});
            }
            const std::string copy = path == nullptr ? std::string{} : *path;
            if (op == OP_SELF) {
                forget(registers, a);
                if (path != nullptr) {
                    registers[a + 1] = copy;
                }
            }
            follow(registers, a, path == nullptr ? nullptr : &copy, key);
            break;
        }
        case OP_MOVE: {
            const auto from = registers.find(GETARG_B(instruction));
            if (from == registers.end()) {
                registers.erase(a);
            } else {
                const std::string copy = from->second;
                registers[a] = copy;
            }
            break;
        }
        case OP_LOADK: {
            const char* text = stringConstant(proto, GETARG_Bx(instruction));
            if (text != nullptr) {
                registers[a] = std::string{"\""} + text;
            } else {
                registers.erase(a);
            }
            break;
        }
        case OP_CALL:
        case OP_TAILCALL: {
            const auto callee = registers.find(a);
            const auto argument = registers.find(a + 1);
            if (callee != registers.end() && callee->second == "require" && argument != registers.end() &&
                argument->second.front() == '"') {
                usage.requires_.push_back({argument->second.substr(1), line});
            }
            forget(registers, a);
            break;
        }
        case OP_SETTABUP: {
            const char* key = stringConstant(proto, GETARG_B(instruction));
            if (writesEnvironment(proto, a) && key != nullptr) {
                usage.globalWrites.emplace_back(key);
            }
            break;
        }
        case OP_JMP:
        case OP_FORLOOP:
        case OP_FORPREP:
        case OP_TFORPREP:
        case OP_TFORCALL:
        case OP_TFORLOOP:
        case OP_VARARG:
        case OP_LOADNIL:
            registers.clear();
            break;
        default:
            if (testTMode(op)) {
                registers.clear();
            } else if (testAMode(op)) {
                registers.erase(a);
            }
            break;
        }
    }
    for (int i = 0; i < proto->sizep; ++i) {
        walk(proto->p[i], usage);
    }
}

} // namespace

std::expected<Usage, std::string> analyze(std::string_view source, std::string_view chunkName) {
    const std::unique_ptr<lua_State, StateCloser> state{luaL_newstate()};
    if (state == nullptr) {
        return std::unexpected("out of memory");
    }
    const std::string named = "@" + std::string{chunkName};
    if (luaL_loadbufferx(state.get(), source.data(), source.size(), named.c_str(), "t") != LUA_OK) {
        const char* message = lua_tostring(state.get(), -1);
        return std::unexpected(message == nullptr ? "syntax error" : message);
    }
    const auto* closure = static_cast<const LClosure*>(lua_topointer(state.get(), -1));
    Usage usage;
    walk(closure->p, usage);
    return usage;
}

} // namespace basalt::kiln
