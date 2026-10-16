#include "Compiler.hpp"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

constexpr std::string_view kUsage =
    "usage: kiln compile <file.lua|dir> [-o <out>] [--strip]\n"
    "       kiln check <file.lua|dir>\n";

std::vector<fs::path> luaFilesIn(const fs::path &input) {
  std::vector<fs::path> files;
  if (fs::is_directory(input)) {
    for (const fs::directory_entry &entry :
         fs::recursive_directory_iterator(input)) {
      if (entry.is_regular_file() && entry.path().extension() == ".lua") {
        files.push_back(entry.path());
      }
    }
    std::ranges::sort(files);
  } else {
    files.push_back(input);
  }
  return files;
}

bool readFile(const fs::path &path, std::string &out) {
  std::ifstream in{path, std::ios::binary};
  if (!in) {
    return false;
  }
  out.assign(std::istreambuf_iterator<char>{in},
             std::istreambuf_iterator<char>{});
  return true;
}

} // namespace

int main(int argc, char **argv) {
  std::vector<std::string> args(argv + 1, argv + argc);
  if (args.size() < 2 || (args[0] != "compile" && args[0] != "check")) {
    std::cerr << kUsage;
    return 2;
  }
  const bool check = args[0] == "check";
  basalt::kiln::Options options;
  fs::path input{args[1]};
  std::optional<fs::path> output;
  for (std::size_t i = 2; i < args.size(); ++i) {
    if (args[i] == "--strip") {
      options.strip = true;
    } else if (args[i] == "-o" && i + 1 < args.size()) {
      output = args[++i];
    } else {
      std::cerr << kUsage;
      return 2;
    }
  }
  const bool directory = fs::is_directory(input);
  int failed = 0;
  for (const fs::path &file : luaFilesIn(input)) {
    std::string source;
    if (!readFile(file, source)) {
      std::cerr << file.string() << ": cannot read\n";
      ++failed;
      continue;
    }
    const fs::path relative =
        directory ? fs::relative(file, input) : file.filename();
    const auto bytecode =
        basalt::kiln::compile(source, relative.generic_string(), options);
    if (!bytecode.has_value()) {
      std::cerr << bytecode.error() << '\n';
      ++failed;
      continue;
    }
    if (check) {
      continue;
    }
    fs::path target =
        output.value_or(fs::path{file}.replace_extension(".luac"));
    if (directory) {
      target = output.value_or(input) / relative;
      target.replace_extension(".luac");
    }
    if (target.has_parent_path()) {
      fs::create_directories(target.parent_path());
    }
    std::ofstream out{target, std::ios::binary};
    out.write(bytecode->data(), static_cast<std::streamsize>(bytecode->size()));
    if (!out) {
      std::cerr << target.string() << ": cannot write\n";
      ++failed;
    }
  }
  return failed == 0 ? 0 : 1;
}
