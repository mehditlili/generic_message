
#include "generic_message/message_pool.h"

#include <boost/foreach.hpp>

#include <generic_message/message_parser.h>

namespace generic_message {

struct FixMessageTypeVisitor : public boost::static_visitor<> {
  FixMessageTypeVisitor(const std::string &current_package)
      : current_package(current_package) {}
  void operator()(BaseType &) {}
  void operator()(BaseTypeArray &) {}
  void operator()(MessageType &type) {
    if (type.package.size() == 0) {
      type.package = current_package;
    }
  }
  void operator()(MessageTypeArray &type) {
    if (type.type.package.size() == 0) {
      type.type.package = current_package;
    }
  }
  const std::string &current_package;
};

void MessagePool::add(
    const std::string &package, const std::string &name,
    const ParsedMessage &message) {
  
  known_messages_[make_key(package, name)] =
      CompiledMessage(*this, message);
}

void MessagePool::add(
    const std::string &package, const std::string &name,
    const std::string &description) {
  ParsedMessage parsed_message;
  if (!parse_message(description, &parsed_message)) {
    throw ParsingFailed("Unable to parse message:\n" + description);
  }
  FixMessageTypeVisitor visitor(package);
  BOOST_FOREACH(Field &field, parsed_message.fields) {
    boost::apply_visitor(visitor, field.type);
  }
  known_messages_[make_key(package, name)] =
      CompiledMessage(*this, parsed_message);
}

const CompiledMessage &MessagePool::get(
    const std::string &package, const std::string &name) const {
  std::map<std::string, CompiledMessage>::const_iterator it =
      known_messages_.find(make_key(package, name));
  if (it == known_messages_.end()) {
    throw MessageNotFound(package + "/" + name);
  }
  return it->second;
}

std::string MessagePool::make_key(
    const std::string &package, const std::string &name) const {
  return package + "/" + name;
}

}  // namespace generic_message
