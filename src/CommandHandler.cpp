#include "CommandHandler.hpp"
#include "RESPUtils.hpp"
#include "ResponseBuilder.hpp"
#include <cctype>
#include <chrono>
#include <string>
#include <thread>
#include <unordered_map>

void setTimeout(std::function<void()> callback, int delay_ms) {
  std::thread([callback, delay_ms]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    callback();
  }).detach();
}

CommandHandler::CommandHandler() {
  handlers["PING"] = [this](const RESPCmd& cmd) { return handlePing(cmd); };
  handlers["ECHO"] = [this](const RESPCmd& cmd) { return handleEcho(cmd); };
  handlers["SET"] = [this](const RESPCmd& cmd) { return handleSet(cmd); };
  handlers["GET"] = [this](const RESPCmd& cmd) { return handleGet(cmd); };
}

std::string CommandHandler::handleCommand(const RESPCmd& command) {
  if (command.args.empty())
    return ResponseBuilder::buildError("Empty command");

  std::string cmd = command.args[0];
  for (auto& x : cmd) {
    x = toupper(static_cast<unsigned char>(x));
  }

  if (auto it = handlers.find(cmd); it != handlers.end()) {
    return it->second(command);
  }
  return ResponseBuilder::buildError("Command not found" + cmd);
}

std::string CommandHandler::handlePing(const RESPCmd& command) {
  return ResponseBuilder::buildSimpleString("PONG");
}
std::string CommandHandler::handleEcho(const RESPCmd& command) {
  if (command.args.size() < 2)
    return ResponseBuilder::buildError("Not enough args for ECHO");
  return ResponseBuilder::buildSimpleString(command.args[1]);
}
std::string CommandHandler::handleSet(const RESPCmd& command) {
  if (command.args.size() == 3) {
    store.insert_or_assign(command.args[1], command.args[2]);
    return ResponseBuilder::buildSimpleString("OK");
  }

  if (command.args.size() == 5) {
    // assume is in correct format for now, arg[3] will be PX and arg[4] is num
    // of ms
    int delay_ms = std::stoi(command.args[4]);
    setTimeout([this, command]() { store.erase(command.args[1]); }, delay_ms);
  }

  return ResponseBuilder::buildError("Incorrect format for SET");
}
std::string CommandHandler::handleGet(const RESPCmd& command) {
  if (command.args.size() < 2)
    return ResponseBuilder::buildError("Not enough args for GET");
  if (auto it = store.find(command.args[1]); it != store.end())
    return ResponseBuilder::buildSimpleString(it->second);
  return ResponseBuilder::buildBulkString("-1");
}
