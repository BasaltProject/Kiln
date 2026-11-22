#include "Process.hpp"

#include <array>
#include <cstdlib>
#include <sstream>

#if defined(_WIN32)
#include <windows.h>
#define environ _environ
#else
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
extern char** environ;
#endif

namespace fs = std::filesystem;

namespace basalt::kiln {
namespace {

#if defined(_WIN32)
constexpr char kPathSeparator = ';';
#else
constexpr char kPathSeparator = ':';
#endif

std::string currentPath() {
    const char* value = std::getenv("PATH");
    return value == nullptr ? std::string{} : std::string{value};
}

std::map<std::string, std::string> effectiveEnvironment(const std::map<std::string, std::string>& overrides) {
    std::map<std::string, std::string> merged;
    for (char** entry = environ; entry != nullptr && *entry != nullptr; ++entry) {
        const std::string text{*entry};
        if (const std::size_t equals = text.find('='); equals != std::string::npos && equals > 0) {
            merged[text.substr(0, equals)] = text.substr(equals + 1);
        }
    }
    for (const auto& [key, value] : overrides) {
        if (key == "PATH" && merged.contains("PATH")) {
            merged[key] = value + std::string{kPathSeparator} + merged[key];
        } else {
            merged[key] = value;
        }
    }
    return merged;
}

} // namespace

std::optional<fs::path> findOnPath(const std::string& name) {
    std::stringstream paths{currentPath()};
    std::string folder;
    while (std::getline(paths, folder, kPathSeparator)) {
        if (folder.empty()) {
            continue;
        }
#if defined(_WIN32)
        for (const char* suffix : {".exe", ".cmd", ".bat"}) {
            const fs::path candidate = fs::path{folder} / (name + suffix);
            if (fs::is_regular_file(candidate)) {
                return candidate;
            }
        }
#else
        const fs::path candidate = fs::path{folder} / name;
        std::error_code error;
        if (fs::is_regular_file(candidate, error) && access(candidate.c_str(), X_OK) == 0) {
            return candidate;
        }
#endif
    }
    return std::nullopt;
}

#if defined(_WIN32)

ProcessResult runProcess(const std::vector<std::string>& command, const fs::path& workingDirectory,
                         const std::map<std::string, std::string>& environment) {
    ProcessResult result;
    std::string line;
    for (const std::string& part : command) {
        line += line.empty() ? "" : " ";
        line += '"';
        for (const char c : part) {
            line += c == '"' ? "\\\"" : std::string{c};
        }
        line += '"';
    }
    std::string block;
    for (const auto& [key, value] : effectiveEnvironment(environment)) {
        block += key + "=" + value;
        block.push_back('\0');
    }
    block.push_back('\0');
    SECURITY_ATTRIBUTES inherit{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    HANDLE read = nullptr;
    HANDLE write = nullptr;
    if (!CreatePipe(&read, &write, &inherit, 0)) {
        return result;
    }
    SetHandleInformation(read, HANDLE_FLAG_INHERIT, 0);
    STARTUPINFOA startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdOutput = write;
    startup.hStdError = write;
    startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    PROCESS_INFORMATION info{};
    const std::string directory = workingDirectory.string();
    if (!CreateProcessA(nullptr, line.data(), nullptr, nullptr, TRUE, 0, block.data(),
                        directory.empty() ? nullptr : directory.c_str(), &startup, &info)) {
        CloseHandle(read);
        CloseHandle(write);
        return result;
    }
    CloseHandle(write);
    result.started = true;
    std::array<char, 4096> buffer{};
    DWORD count = 0;
    while (ReadFile(read, buffer.data(), static_cast<DWORD>(buffer.size()), &count, nullptr) && count > 0) {
        result.output.append(buffer.data(), count);
    }
    WaitForSingleObject(info.hProcess, INFINITE);
    DWORD code = 0;
    GetExitCodeProcess(info.hProcess, &code);
    result.exitCode = static_cast<int>(code);
    CloseHandle(info.hProcess);
    CloseHandle(info.hThread);
    CloseHandle(read);
    return result;
}

#else

ProcessResult runProcess(const std::vector<std::string>& command, const fs::path& workingDirectory,
                         const std::map<std::string, std::string>& environment) {
    ProcessResult result;
    if (command.empty()) {
        return result;
    }
    const std::map<std::string, std::string> merged = effectiveEnvironment(environment);
    std::vector<std::string> variables;
    for (const auto& [key, value] : merged) {
        variables.push_back(key + "=" + value);
    }
    std::vector<char*> envp;
    for (std::string& variable : variables) {
        envp.push_back(variable.data());
    }
    envp.push_back(nullptr);
    std::vector<std::string> arguments = command;
    std::vector<char*> argv;
    for (std::string& argument : arguments) {
        argv.push_back(argument.data());
    }
    argv.push_back(nullptr);
    std::array<int, 2> channel{};
    if (pipe(channel.data()) != 0) {
        return result;
    }
    const pid_t child = fork();
    if (child < 0) {
        close(channel[0]);
        close(channel[1]);
        return result;
    }
    if (child == 0) {
        dup2(channel[1], STDOUT_FILENO);
        dup2(channel[1], STDERR_FILENO);
        close(channel[0]);
        close(channel[1]);
        if (!workingDirectory.empty() && chdir(workingDirectory.c_str()) != 0) {
            _exit(126);
        }
        environ = envp.data();
        execvp(argv[0], argv.data());
        _exit(127);
    }
    close(channel[1]);
    std::array<char, 4096> buffer{};
    ssize_t count = 0;
    while ((count = read(channel[0], buffer.data(), buffer.size())) > 0) {
        result.output.append(buffer.data(), static_cast<std::size_t>(count));
    }
    close(channel[0]);
    int status = 0;
    waitpid(child, &status, 0);
    result.started = !(WIFEXITED(status) && WEXITSTATUS(status) == 127 && result.output.empty());
    result.exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    return result;
}

#endif

} // namespace basalt::kiln
