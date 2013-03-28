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
