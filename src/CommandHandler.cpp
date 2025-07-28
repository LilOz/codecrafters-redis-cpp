#include "CommandHandler.hpp"
#include "RESPUtils.hpp"
#include "ResponseBuilder.hpp"
#include <cctype>
#include <string>
#include <unordered_map>

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

  if (auto it = handlers.find(cmd); it!=handlers.end()){
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
  if (command.args.size() < 3)
    return ResponseBuilder::buildError("Not enough args for SET");
  store.insert_or_assign(command.args[1], command.args[2]);
  return ResponseBuilder::buildSimpleString("OK");
}
std::string CommandHandler::handleGet(const RESPCmd& command) {
  if (command.args.size() < 2)
    return ResponseBuilder::buildError("Not enough args for SET");
  if (auto it = store.find(command.args[1]); it != store.end())
    return ResponseBuilder::buildSimpleString(it->second);
  return ResponseBuilder::buildBulkString("-1");
}
