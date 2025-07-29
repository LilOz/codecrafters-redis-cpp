#include "CommandHandler.hpp"
#include "RESPUtils.hpp"
#include "ResponseBuilder.hpp"
#include <cctype>
#include <chrono>
#include <mutex>
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
  std::lock_guard<std::mutex> lock(mutex);
  if (command.args.size() < 3)
    return ResponseBuilder::buildError("Not enough args for SET");
  const std::string& key = command.args[1];
  const std::string& val = command.args[2];
  store.insert_or_assign(key, val);

  if (command.args.size() == 5) {
    std::string opt = command.args[3];
    for (auto& x : opt) {
      x = toupper(static_cast<unsigned char>(x));
    }
    if (opt != "PX")
      return ResponseBuilder::buildError("Invalid flag: " + opt);

    const std::string& ttl = command.args[4];

    try {
      int delay_ms = std::stoi(ttl);
      setTimeout([this, key]() { store.erase(key); }, delay_ms);
    } catch (...) {
      return ResponseBuilder::buildError("Incorrect ttl: " + ttl);
    }
  }
  return ResponseBuilder::buildSimpleString("OK");
}

std::string CommandHandler::handleGet(const RESPCmd& command) {
  if (command.args.size() < 2)
    return ResponseBuilder::buildError("Not enough args for GET");

  if (auto it = store.find(command.args[1]); it != store.end())
    return ResponseBuilder::buildSimpleString(it->second);

  return ResponseBuilder::buildNullBulkString();
}
