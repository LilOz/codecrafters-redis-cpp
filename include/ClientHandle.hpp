#pragma once
#include "CommandHandler.hpp"
#include <memory>

int handleClient(int client_fd, std::shared_ptr<CommandHandler> cmdHandler);
