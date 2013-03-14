#pragma once

#include <string>

#include <generic_message/parsed_message.h>

namespace generic_message {

bool parse_message(const std::string &message, ParsedMessage *result);

}  // namespace generic_message
