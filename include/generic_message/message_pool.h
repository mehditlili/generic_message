
#pragma once

#include <stdexcept>
#include <map>
#include <string>

#include <generic_message/parsed_message.h>
#include <generic_message/compiled_message.h>

namespace generic_message {

class ParsingFailed : public std::runtime_error {
 public:
  ParsingFailed(const std::string &message)
      : std::runtime_error(message) {}
};

class MessageNotFound : public std::runtime_error {
 public:
  MessageNotFound(const std::string &message)
      : std::runtime_error(message) {}
};

class MessagePool {
 public:
  void add(
      const std::string &package, const std::string &name,
      const ParsedMessage &message);
  void add(
      const std::string &package, const std::string &name,
      const std::string &description);
  const CompiledMessage &get(
      const std::string &package, const std::string &name) const;
 private:
  std::map<std::string, CompiledMessage> known_messages_;

  std::string make_key(
      const std::string &package, const std::string &name) const;
};

}  // namespace generic_message
