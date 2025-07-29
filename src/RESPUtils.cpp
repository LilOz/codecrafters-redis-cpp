#include "RESPUtils.hpp"

void RESPParser::append(const char* data, size_t len) {
  buffer.append(data, len);
}

// parses a single RESP command from the data in buf
std::optional<RESPCmd> RESPParser::parseCommand() {
  if (buffer.empty() || buffer[0] != '*')
    return {};

  buffer.erase(0, 1);

  std::string line;
  if (!readLine(line)) {
    return {};
  }

  int cnt;
  try {
    cnt = std::stoi(line);
  } catch (...) {
    return {};
  }

  std::cout << "test\n";
  RESPCmd cmd;
  for (int i = 0; i < cnt; i++) {
    std::string arg;
    if (!readBulkString(arg))
      return {};

    cmd.args.push_back(arg);
  }

  std::cout<<"Command args: \n\n";
  cmd.print();
  return cmd;
}

bool RESPParser::readLine(std::string& line) {
  auto n = buffer.find("\r\n");
  if (n == std::string::npos)
    return false;

  line = buffer.substr(0, n);
  buffer.erase(0, n + 2);

  return true;
}

bool RESPParser::readBulkString(std::string& out) {
  if (buffer.empty() || buffer[0] != '$')
    return false;

  buffer.erase(0, 1); // remove $
  std::string line;
  if (!readLine(line))
    return {};

  int cnt = std::stoi(line);

  if (buffer.size() < cnt + 2)
    return false;
  out = buffer.substr(0, cnt);
  buffer.erase(0, cnt + 2); // remove all plus the \r\n

  return true;
}
