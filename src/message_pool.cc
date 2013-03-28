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
