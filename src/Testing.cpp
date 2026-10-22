#include "Testing.hpp"

#include <basalt/wire/io/FileIO.hpp>
#include <basalt/wire/lua/LuaSandbox.hpp>
#include <basalt/wire/plugins/PluginPackage.hpp>

#include <algorithm>
#include <iostream>
#include <vector>

namespace fs = std::filesystem;

namespace basalt::kiln {
namespace {

// What a test file sees: test(name, fn), a stand-in server (on, cmd, mock) and a small mock world.
constexpr std::string_view kPrelude = R"lua(
local tests = {}
function test(name, fn) tests[#tests + 1] = { name = name, fn = fn } end
local handlers, commands = {}, {}
function on(event, fn)
    handlers[event] = handlers[event] or {}
    table.insert(handlers[event], fn)
end
watch = on
function cmd(spec, fn, options)
    commands[spec:match("^(%S+)")] = { fn = fn, options = options or {} }
end
local clock, tasks = 0, {}
local function schedule(delay, period, fn)
    local task = { at = clock + delay, period = period, fn = fn, live = true }
    tasks[#tasks + 1] = task
    return { cancel = function() task.live = false end }
end
function later(ticks, fn) return schedule(ticks, nil, fn) end
function every(period, fn) return schedule(period, period, fn) end
local known = {}
local function world(name)
    known[name] = known[name] or { name = name, time = 0, raining = false, thundering = false,
                                     difficulty = "normal", rules = {}, blocks = {}, entities = {} }
    local w = known[name]
    local function key(x, y, z) if type(x) == "table" then return x.x .. "," .. x.y .. "," .. x.z end return x .. "," .. y .. "," .. z end
    function w:block(x, y, z) return self.blocks[key(x, y, z)] or "minecraft:air" end
    function w:set_block(x, y, z, block)
        if type(x) == "table" then block = y end
        self.blocks[key(x, y, z)] = block
        return true
    end
    function w:spawn(kind, x, y, z)
        local at = type(x) == "table" and x or { x = x, y = y, z = z }
        local entity = { type = kind, x = at.x, y = at.y, z = at.z, alive = true }
        function entity:remove() self.alive = false end
        self.entities[#self.entities + 1] = entity
        return entity
    end
    return w
end
worlds = { by_name = world, default = function() return world("world") end,
          all = function() local all = {} for _, w in pairs(known) do all[#all + 1] = w end return all end }
mock = {}
function mock.tick(count)
    for _ = 1, count or 1 do
        clock = clock + 1
        for _, task in ipairs(tasks) do
            if task.live and clock >= task.at then
                task.fn()
                if task.period then task.at = task.at + task.period else task.live = false end
            end
        end
    end
end
function mock.world(name) return world(name or "world") end
function mock.player(name, fields)
    local player = { name = name, health = 20, max_health = 20, sent = {} }
    function player:send(text, args) self.sent[#self.sent + 1] = { text = text, args = args or {} } end
    for key, value in pairs(fields or {}) do player[key] = value end
    return player
end
function mock.emit(event, data)
    for _, fn in ipairs(handlers[event] or {}) do fn(data) end
end
function mock.command(name, source, ...)
    assert(commands[name], "no command called " .. name)
    return commands[name].fn(source, ...)
end
function mock.reset() handlers, commands, tasks, clock = {}, {}, {}, 0 end
function __basalt_run()
    local failed = 0
    for _, t in ipairs(tests) do
        local ok, err = pcall(t.fn)
        if ok then
            print("ok   " .. t.name)
        else
            failed = failed + 1
            print("FAIL " .. t.name .. ": " .. tostring(err))
        end
    end
    print("# " .. #tests .. " test(s), " .. failed .. " failed")
end
)lua";

struct Counts {
    int total = 0;
    int failed = 0;
};

void runFile(const Project& project, const fs::path& file, Counts& counts) {
    namespace io = wire::io::fileIO;
    const auto sandbox = wire::lua::LuaSandbox::create(
        project.manifest.id, {}, 0,
        [&project](std::string_view module) -> std::optional<std::vector<std::uint8_t>> {
            const std::optional<std::string> path = wire::plugins::packageLayout::moduleFile(module);
            return path.has_value() ? io::readBytes((project.root / *path).string()) : std::nullopt;
        },
        [&counts](std::string_view line) {
            std::cout << "  " << line << '\n';
            const bool failed = line.starts_with("FAIL ");
            counts.failed += failed ? 1 : 0;
            counts.total += failed || line.starts_with("ok   ") ? 1 : 0;
        });
    std::cout << file.filename().string() << '\n';
    const std::string name = file.filename().string();
    const std::string source = io::readText(file.string()).value_or(std::string{});
    const std::pair<std::string_view, std::string> steps[] = {
        {kPrelude, "prelude"}, {source, name}, {"__basalt_run()", "run"}};
    for (const auto& [text, chunkName] : steps) {
        sandbox->refill();
        const auto chunk = sandbox->compile(text, chunkName);
        const auto result = chunk.has_value() ? sandbox->run(*chunk) : std::unexpected(chunk.error());
        if (!result.has_value()) {
            std::cout << "  FAIL " << chunkName << ": " << result.error().message << '\n';
            ++counts.failed;
            return;
        }
    }
}

} // namespace

bool runTests(const Project& project) {
    const fs::path folder = project.root / "tests";
    std::vector<fs::path> files;
    if (fs::is_directory(folder)) {
        for (const fs::directory_entry& entry : fs::directory_iterator(folder)) {
            if (entry.is_regular_file() && entry.path().extension() == ".lua") {
                files.push_back(entry.path());
            }
        }
    }
    std::ranges::sort(files);
    Counts counts;
    for (const fs::path& file : files) {
        runFile(project, file, counts);
    }
    std::cout << counts.total << " test(s) run, " << counts.failed << " failed\n";
    return counts.failed == 0;
}

} // namespace basalt::kiln
