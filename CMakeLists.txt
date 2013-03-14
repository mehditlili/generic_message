cmake_minimum_required(VERSION 2.8)

project(generic_message)

include_directories(include)

add_executable(test_generic_message
  src/test_generic_message.cc
  src/message_parser.cc)
