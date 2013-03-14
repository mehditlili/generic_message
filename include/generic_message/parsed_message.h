
#pragma once

#include <vector>

#include <boost/any.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/none.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>

namespace generic_message {

struct BaseType {
  typedef enum {
    UNKNOWN,
    BOOL, INT8, UINT8,
    INT16, UINT16,
    INT32, UINT32,
    INT64, UINT64,
    FLOAT32, FLOAT64,
    STRING, TIME, DURATION
  } base_type;

  base_type type;
  BaseType() : type(UNKNOWN) {}
  BaseType(BaseType::base_type type) : type(type) {}
};

struct MessageType {
  std::string package;
  std::string name;

  MessageType() {}
  MessageType(const std::string &name) : name(name) {}
  MessageType(const std::string &package, const std::string &name)
      : package(package), name(name) {}
};

template<typename T>
struct ArrayType {
  T type;
  boost::optional<size_t> size;

  ArrayType() {}
  ArrayType(
      const T &type,
      const boost::optional<size_t> size = boost::none)
      : type(type), size(size) {}
};

typedef ArrayType<BaseType> BaseTypeArray;
typedef ArrayType<MessageType> MessageTypeArray;

typedef boost::variant<BaseType, MessageType, BaseTypeArray, MessageTypeArray> Type;
typedef boost::variant<bool, int, double, std::vector<char> > BaseTypes;

struct Constant {
  Type type;
  std::string name;
  boost::any value;

  Constant() {}

  template<typename T>
  Constant(const Type &type, const std::string &name, T &value)
      : type(type), name(name), value(value) {};
};
  
struct Field {
  Type type;
  std::string name;

  Field() {}
  Field(const Type &type, const std::string &name)
      : type(type), name(name) {}
};

struct ParsedMessage {
  std::vector<Constant> constants;
  std::vector<Field> fields;

  ParsedMessage() {}
  ParsedMessage(
      const std::vector<Constant> &constants)
      : constants(constants) {}
  ParsedMessage(
      const std::vector<Field> &fields)
      : fields(fields) {}
  ParsedMessage(
      const std::vector<Constant> &constants,
      const std::vector<Field> &fields)
      : constants(constants), fields(fields) {}
};

}  // namespace generic_message

BOOST_FUSION_ADAPT_STRUCT(
    generic_message::ParsedMessage,
    (std::vector<generic_message::Constant>, constants)
    (std::vector<generic_message::Field>, fields))
