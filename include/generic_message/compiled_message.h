
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
    const Field &field() { return *field_; }
    const AccessPath &path() { return path_; }

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
