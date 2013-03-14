
#include <iostream>
#include <iterator>
#include <fstream>

#include <generic_message/message_parser.h>

using namespace generic_message;

struct type_visitor : public boost::static_visitor<> {
  void operator()(const BaseType &type) const {
    std::cout << "<base type " << type.type << ">";
  }
  void operator()(const MessageType &type) const {
    std::cout << type.package << "/" << type.name;
  }
  void operator()(const BaseTypeArray &type) const {
    std::cout << "<base type " << type.type.type << ">["
              << type.size.get_value_or(0) << "]";
  }
  void operator()(const MessageTypeArray &type) const {
    std::cout << type.type.package << "/"
              << type.type.name << "["
              << type.size.get_value_or(999) << "]";
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
  std::cout << "Values:" << std::endl;
  for (size_t i = 0; i < parsed_message.fields.size(); i++) {
    boost::apply_visitor(type_visitor(), parsed_message.fields[i].type);
    std::cout << " " << parsed_message.fields[i].name << std::endl;
  }
  std::cout << std::endl << "Constants:" << std::endl;
  for (size_t i = 0; i < parsed_message.constants.size(); i++) {
    std::cout << parsed_message.constants[i].name << " ";
    boost::apply_visitor(type_visitor(), parsed_message.constants[i].type);
    std::cout << " = '" << parsed_message.constants[i].value << "'" << std::endl;
  }
  return 0;
}
