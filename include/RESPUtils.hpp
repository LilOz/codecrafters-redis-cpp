#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <vector>

inline std::string buildResponse(const std::string& word) {
  return "+" + word + "\r\n";
}

struct RESPCmd {
  std::vector<std::string> args;

  void print() {
    for (const auto& x : args) {
      std::cout << x << "\n";
    }
  }
};

class RESPParser {
public:
  void append(const char* data, size_t len);
  std::optional<RESPCmd> parseCommand();

private:
  std::string buffer;
  bool readLine(std::string& line);
  bool readBulkString(std::string& out);
};
