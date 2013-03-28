/**
 * Copyright (c) 2013, Lorenz Moesenlechner <moesenle@gmail.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * The names of the contributors may not be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

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
