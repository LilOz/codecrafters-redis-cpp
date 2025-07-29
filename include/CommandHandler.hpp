#pragma once

#include "RESPUtils.hpp"
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>

class CommandHandler {
public:
  CommandHandler();
  std::string handleCommand(const RESPCmd& command);

private:
  std::mutex mutex;
  std::unordered_map<std::string, std::function<std::string(const RESPCmd&)>>
      handlers;
  std::unordered_map<std::string, std::string> store;

  std::string handlePing(const RESPCmd& command);
  std::string handleEcho(const RESPCmd& command);
  std::string handleSet(const RESPCmd& command);
  std::string handleGet(const RESPCmd& command);
};

void setTimeout(std::function<void()> callback, int delay_ms);
