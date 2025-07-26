#include <arpa/inet.h>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <optional>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

//                                $3\r\nhey\r\n

std::string buildResponse(const std::string& word) {
  std::string res;
  res += '$';
  res += std::to_string(word.size());
  res += "\r\n";
  res += word;
  res += "\r\n";

  return res;
}

struct RESPCmd {
  std::vector<std::string> args;

  void print() {
    for (auto& x : args) {
      std::cout << x << "\n";
    }
  }
};

class RESPParser {
public:
  void append(const char* data, size_t len) { buffer.append(data, len); }

  // parses a single RESP command from the data in buf
  std::optional<RESPCmd> parseCommand() {
    if (buffer.empty() || buffer[0] != '*')
      return {};

    buffer.erase(0, 1);

    std::string line;
    if (!readLine(line)) {
      return {};
    }
    int cnt = std::stoi(line);

    RESPCmd cmd;
    for (int i = 0; i < cnt; i++) {
      std::string arg;
      if (!readBulkString(arg))
        return {};

      for (auto& c : arg)
        c = toupper(c);
      cmd.args.push_back(arg);
    }

    return cmd;
  }

private:
  std::string buffer;

  bool readLine(std::string& line) {
    auto n = buffer.find("\r\n");
    if (n == std::string::npos)
      return false;

    line = buffer.substr(0, n);
    buffer.erase(0, n + 2);

    return true;
  }

  bool readBulkString(std::string& out) {
    // ECHO\r\n$3\r\nhey\r\n

    if (buffer.empty() || buffer[0] != '$')
      return false;

    buffer.erase(0, 1); // remove $
    std::string line;
    if (!readLine(line))
      return {};

    int cnt = std::stoi(line);

    out = buffer.substr(0, cnt);
    buffer.erase(0, cnt + 2); // remove all plus the \r\n

    return true;
  }
};

int main(int argc, char** argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // creates a ipv4 (inet), tcp (sock_stream) socket handler
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    std::cerr << "Failed to create server socket\n";
    return 1;
  }

  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
      0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(6379);

  // binds socket to address and port
  if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) !=
      0) {
    std::cerr << "Failed to bind to port 6379\n";
    return 1;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }

  while (true) {
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);
    std::cout << "Waiting for a client to connect...\n";
    std::cout << "Logs from your program will appear here!\n";

    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr,
                           (socklen_t*)&client_addr_len);
    std::thread(
        [](int client_fd) {
          RESPParser parser;
          char buf[1024] = {};
          while (true) {
            if (recv(client_fd, buf, sizeof(buf), 0) < 0) {
              std::cerr << "recieve failed\n";
              return -1;
            }
            std::cout << "Message from client: " << buf << "\n";

            parser.append(buf, strlen(buf));
            auto cmdOpt = parser.parseCommand();
            std::string response;

            if (cmdOpt) {
              auto& cmd = cmdOpt.value();

              const std::string& command = cmd.args[0];

              if (command == "PING") {
                response = buildResponse("PONG");
              }

              else if (command == "ECHO") {
                response = buildResponse(cmd.args[1]);
              }
            }
            if (send(client_fd, response.c_str(), response.size(), 0) < 0) {
              std::cerr << "send failed\n";
              return -1;
            }
          }
          close(client_fd);
          return 0;
        },
        client_fd)
        .detach();
    std::cout << "Client connected\n";
  }

  close(server_fd);

  return 0;
}
