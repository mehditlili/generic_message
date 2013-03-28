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

#include <map>
#include <stdexcept>
#include <string>

#include <boost/function.hpp>

#include <generic_message/parsed_message.h>

namespace generic_message {

class MessagePool;

class CompilationFailed : public std::runtime_error {
 public:
  CompilationFailed(const std::string &message)
      : std::runtime_error(message) {}
};

class CompiledMessage {
 public:
  class AccessPath {
   public:
    AccessPath();
    AccessPath(size_t offset, boost::function<size_t (const void *)> dynamic_offset);
    size_t offset(const void *data) const {
      return offset_ + dynamic_offset_(data);
    }
    bool isDynamic();

    static AccessPath advance(const AccessPath &path, size_t offset);
    static AccessPath advance(
        const AccessPath &path,
        boost::function<size_t (const void *)> dynamic_offset);
    
   private:
    size_t offset_;
    boost::function<size_t (const void *)> dynamic_offset_;
  };

  class CompiledField {
   public:
    CompiledField();
    CompiledField(const CompiledField &src);
    CompiledField(const Field &field, const AccessPath &path);
    const Field &field() const { return *field_; }
    const AccessPath &path() const { return path_; }

   private:
    const Field *field_;
    const AccessPath path_;
  };

  CompiledMessage() {}
  CompiledMessage(const MessagePool &pool, const ParsedMessage &message);
  size_t size(const void *data) const { path_to_next_.offset(data); }
  const ParsedMessage &message() const { return message_; }

 private:
  AccessPath path_to_next_;
  std::map<std::string, AccessPath> field_access_paths_;
  ParsedMessage message_;
};

}  // namespace generic_message
