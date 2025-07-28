#pragma once

#include <string>

class ResponseBuilder {
private:
  inline static const std::string deliminator = "\r\n";
public:
  inline static std::string buildSimpleString(const std::string& word) {
    return "+" + word + deliminator;
  }
  inline static std::string buildBulkString(const std::string& word) {
    return "$" + std::to_string(word.size()) + "\r\n" + word + "\r\n";
  }
  inline static std::string buildError(const std::string& word) {
    return "-ERR" + word + "\r\n";
  }
};
