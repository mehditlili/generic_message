
#include <iostream>
#include <iterator>
#include <fstream>

#include <generic_message/message_parser.h>

using namespace generic_message;

struct type_visitor : public boost::static_visitor<> {
  void operator()(const BaseType &type) const {
    std::cout << "base type: " << type.type << std::endl;
  }
  void operator()(const MessageType &type) const {
    std::cout << "message type: " << type.package << "-" << type.name << std::endl;
  }
  void operator()(const BaseTypeArray &type) const {
    std::cout << "base type array " << type.type.type << " "
              << (type.size ? *type.size : -1 ) << std::endl;
  }
  void operator()(const MessageTypeArray &type) const {
    std::cout << "message type array " << type.type.package << " "
              << type.type.name << " "
              << (type.size ? *type.size : -1 ) << std::endl;
  }
};

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <message>" << std::endl;
    return 1;
  }

  std::ifstream message_file(argv[1]);
  if (!message_file.is_open()) {
    std::cerr << "Unable to open " << argv[1] << std::endl;
    return 1;
  }

  std::string message;
  message_file.unsetf(std::ios::skipws);
  std::copy(std::istream_iterator<char>(message_file),
            std::istream_iterator<char>(),
            std::back_inserter(message));

  ParsedMessage parsed_message;
  if (parse_message(message, &parsed_message)) {
    std::cout << "Parsing ok" << std::endl;
  }
  for (size_t i = 0; i < parsed_message.fields.size(); i++) {
    std::cout << parsed_message.fields[i].name << ": ";
    boost::apply_visitor(type_visitor(), parsed_message.fields[i].type);
  }
}
