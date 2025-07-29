#include "ClientHandle.hpp"
#include "CommandHandler.hpp"
#include "RESPUtils.hpp"
#include <memory>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

int handleClient(int client_fd, std::shared_ptr<CommandHandler> cmdHandler) {
  RESPParser parser;
  char buf[1024] = {};

  while (true) {
    size_t bytes = recv(client_fd, buf, sizeof(buf), 0);
    if (bytes < 0) {
      std::cerr << "recieve failed\n";
      return -1;
    } else if (bytes == 0) {
      std::cout << "Client disconnected\n";
      break;
    }
    std::cout << "Message from client: " << std::string(buf, bytes) << "\n";

    parser.append(buf, bytes);

    while (true) {
      auto cmdOpt = parser.parseCommand();
      if (!cmdOpt)
        break;
      auto& cmd = cmdOpt.value();

      std::string response = cmdHandler->handleCommand(cmd);

      if (send(client_fd, response.c_str(), response.size(), 0) < 0) {
        std::cerr << "send failed\n";
        return -1;
      }
    }
  }
  close(client_fd);
  return 0;
}
