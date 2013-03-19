
#pragma once

#include <boost/foreach.hpp>

#include <generic_message/parsed_message.h>
#include <generic_message/message_pool.h>

namespace generic_message {

template<typename T>
struct MessageTypeTraits {
};

struct IsDynamicVisitor : public boost::static_visitor<bool> {
  const MessagePool &pool;
  IsDynamicVisitor(const MessagePool &pool) : pool(pool) {}
  template<typename T> bool operator()(const T &type) const {
    return MessageTypeTraits<T>::isDynamic(pool, type);
  }
};

template<>
struct MessageTypeTraits<BaseType> {
  static bool isDynamic(const MessagePool &, const BaseType &type) {
    if (type.type == BaseType::STRING) {
      return true;
    } else {
      return false;
    }
  }
};

template<typename T>
struct MessageTypeTraits<ArrayType<T> > {
  static bool isDynamic(const MessagePool &pool, const ArrayType<T> &type) {
    return !type.size || MessageTypeTraits<T>::isDynamic(pool, type.type);
  }
};

template<>
struct MessageTypeTraits<Type> {
  static bool isDynamic(const MessagePool &pool, const Type type) {
    return boost::apply_visitor(IsDynamicVisitor(pool), type);
  }
};

template<>
struct MessageTypeTraits<ParsedMessage> {
  static bool isDynamic(const MessagePool &pool, const ParsedMessage &type) {
    BOOST_FOREACH(const Field &field, type.fields) {
      if (MessageTypeTraits<Type>::isDynamic(pool, field.type)) {
        return true;
      }
    }
    return false;
  }
};

template<>
struct MessageTypeTraits<MessageType> {
  static bool isDynamic(const MessagePool &pool, const MessageType &type) {
    return MessageTypeTraits<ParsedMessage>::isDynamic(
        pool, pool.get(type.package, type.name).message());
  }
};

template<typename T>
bool isDynamic(const MessagePool &pool, const T &type) {
  return MessageTypeTraits<T>::isDynamic(pool, type);
}

}  // namespace generic_message
