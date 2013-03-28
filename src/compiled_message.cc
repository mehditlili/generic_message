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

#include <generic_message/compiled_message.h>

#include <stdint.h>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include <generic_message/message_pool.h>
#include <generic_message/message_type_traits.h>

namespace generic_message {

static size_t zero_offset(const void *) {
  return 0;
}

static size_t sizeOf(const BaseType &type) {
  using boost::lexical_cast;

  switch (type.type) {
    case BaseType::BOOL:
    case BaseType::INT8:
    case BaseType::UINT8: return 1;
    case BaseType::INT16:
    case BaseType::UINT16: return 2;
    case BaseType::INT32:
    case BaseType::UINT32:
    case BaseType::FLOAT32: return 4;
    case BaseType::INT64:
    case BaseType::UINT64:
    case BaseType::FLOAT64: return 8;
    case BaseType::TIME:
    case BaseType::DURATION: return 8;
    default:
      throw CompilationFailed(
          "Not a fixed size type type: " + lexical_cast<std::string>(type.type));
  }
}

static size_t sizeOf(
    const MessagePool &pool, const BaseType &type, const void *data) {
  if (type.type == BaseType::STRING) {
    return *reinterpret_cast<const uint32_t *>(data) + 4;
  } else {
    return sizeOf(type);
  }
}

static size_t sizeOf(
    const MessagePool &pool, const MessageType &type, const void *data) {
  return pool.get(type.package, type.name).size(data);
}

template<typename T>
static size_t sizeOf(
    const MessagePool &pool, const ArrayType<T> &type, const void *data) {
  size_t size;
  const char *current;
  if (type.size) {
    size = *type.size;
    current = reinterpret_cast<const char *>(data);
  } else {
    size = *reinterpret_cast<const uint32_t *>(data);
    current = reinterpret_cast<const char *>(data) + 4;
  }
  if (isDynamic(pool, type.type)) {
    for (size_t i = 0; i < size; i++) {
      current += sizeOf(pool, *reinterpret_cast<const T *>(current), current);
    }
    return current - reinterpret_cast<const char *>(data);
  } else {
    return size * sizeOf(pool, type.type, current);
  }
}
                     
struct CombineDynamicOffsets {
  boost::function<size_t (const void *)> lhs;
  boost::function<size_t (const void *)> rhs;
  CombineDynamicOffsets(
      boost::function<size_t (const void *)> lhs,
      boost::function<size_t (const void *)> rhs)
      : lhs(lhs), rhs(rhs) {}
  size_t operator()(const void *data) {
    return lhs(data) + rhs(data);
  }
};

// Dynamic array over fixed size elements
template<typename T>
struct DynamicOffset {
  const MessagePool &pool;
  const T &type;
  CompiledMessage::AccessPath path;

  DynamicOffset(
      const MessagePool &pool, const T &type,
      const CompiledMessage::AccessPath &path)
      : pool(pool), type(type), path(path) {}
  size_t operator()(const void *data) {
    size_t offset = path.offset(data);
    const void *data_pointer = reinterpret_cast<const void *>(
        reinterpret_cast<const uint8_t *>(data) + offset);
    return sizeOf(pool, type, data_pointer) + offset;
  }
};

struct MakeAccessPathVisitor
    : public boost::static_visitor<CompiledMessage::AccessPath> {
  MakeAccessPathVisitor(
      const CompiledMessage::AccessPath &path, const MessagePool &pool)
      : path(path), pool(pool) {}
  CompiledMessage::AccessPath operator()(const BaseType &type) {
    if (type.type == BaseType::STRING) {
      return CompiledMessage::AccessPath::advance(
          path, DynamicOffset<BaseType>(pool, type, path));
    } else {
      return CompiledMessage::AccessPath::advance(path, sizeOf(type));
    }
  }
  CompiledMessage::AccessPath operator()(const MessageType &type) {
    const CompiledMessage &sub_message = pool.get(type.package, type.name);
    if (isDynamic(pool, sub_message.message())) {
      return CompiledMessage::AccessPath::advance(path, DynamicOffset<MessageType>(
          pool, type, path));
    } else {
      // Since the message is not dynamic, computing its size will not
      // require a data pointer and we can safely pass null.
      return CompiledMessage::AccessPath::advance(
          path, sub_message.size(0));
    }
  }
  template<typename T>
  CompiledMessage::AccessPath operator()(const ArrayType<T> &type) {
    if (isDynamic(pool, type)) {
      return CompiledMessage::AccessPath::advance(path, DynamicOffset<ArrayType<T> >(
          pool, type, path));
    } else {
      return CompiledMessage::AccessPath::advance(path, sizeOf(pool, type, 0));
    }
  }

  const CompiledMessage::AccessPath &path;
  const MessagePool &pool;
};

CompiledMessage::AccessPath::AccessPath()
    : offset_(0), dynamic_offset_(&zero_offset) {
};

CompiledMessage::AccessPath::AccessPath(
    size_t offset, boost::function<size_t (const void *)> dynamic_offset)
    : offset_(offset), dynamic_offset_(dynamic_offset) {
}

bool CompiledMessage::AccessPath::isDynamic() {
  return dynamic_offset_ != &zero_offset;
}

CompiledMessage::AccessPath CompiledMessage::AccessPath::advance(
    const AccessPath &path, size_t offset){
  return AccessPath(path.offset_ + offset, path.dynamic_offset_);
}

CompiledMessage::AccessPath CompiledMessage::AccessPath::advance(
    const AccessPath &path,
    boost::function<size_t (const void *)> dynamic_offset) {
  return AccessPath(
      path.offset_, CombineDynamicOffsets(
          path.dynamic_offset_, dynamic_offset));
}

CompiledMessage::CompiledField::CompiledField()
    : field_(0) {
}

CompiledMessage::CompiledField::CompiledField(const CompiledField &src)
    : field_(src.field_), path_(src.path_) {
}

CompiledMessage::CompiledField::CompiledField(
    const Field &field, const AccessPath &path)
    : field_(&field), path_(path) {
}

CompiledMessage::CompiledMessage(
    const MessagePool &pool, const ParsedMessage &message)
    : message_(message) {
  AccessPath current_path;
  BOOST_FOREACH(const Field &field, message.fields) {
    field_access_paths_[field.name] = current_path;
    MakeAccessPathVisitor visitor(current_path, pool);
    boost::apply_visitor(visitor, field.type);
    current_path = visitor.path;
  }
  path_to_next_ = current_path;
}

}  // namespace generic_message
