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

#include <generic_message/parsed_message.h>

#include <vector>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_symbols.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>

namespace generic_message {

namespace spirit = boost::spirit;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

template<typename Iterator>
struct SkipGrammar 
    : qi::grammar<Iterator> {
  qi::rule<Iterator> comment;
  qi::rule<Iterator> skip;

  SkipGrammar() : SkipGrammar::base_type(skip) {
    using qi::lit;
    using qi::eol;
    using ascii::char_;
    using ascii::blank;

    comment = lit('#') >> *(char_ - eol) >> &eol;
    skip = blank | comment;
  }
};

template<typename Iterator>
struct MessageGrammar
    : qi::grammar<Iterator, ParsedMessage(), SkipGrammar<Iterator> > {
  
  qi::rule<Iterator, std::string()> basic_id;
  qi::rule<Iterator, std::string(), SkipGrammar<Iterator> > identifier;
  qi::rule<Iterator, boost::optional<size_t>(), SkipGrammar<Iterator> > array_size;
  qi::rule<Iterator, BaseType(), SkipGrammar<Iterator> > base_type;
  qi::rule<Iterator, BaseType(), SkipGrammar<Iterator> > constant_type;
  qi::rule<Iterator, MessageType(), SkipGrammar<Iterator> > message_type;
  qi::rule<Iterator, BaseTypeArray(), SkipGrammar<Iterator> > base_type_array;
  qi::rule<Iterator, MessageTypeArray(), SkipGrammar<Iterator> > message_type_array;  
  qi::rule<Iterator, Type(), SkipGrammar<Iterator> > type;
  qi::rule<Iterator, Field(), SkipGrammar<Iterator> > field;
  qi::rule<Iterator, bool(), SkipGrammar<Iterator> > bool_constant;
  qi::rule<Iterator, long long(), SkipGrammar<Iterator> > fixed_number_constant;
  qi::rule<Iterator, double(), SkipGrammar<Iterator> > floating_point_constant;
  qi::rule<Iterator, std::string(), SkipGrammar<Iterator> > string_constant;
  qi::rule<Iterator, Constant(), SkipGrammar<Iterator> > constant;
  qi::rule<Iterator, ParsedMessage(), SkipGrammar<Iterator> > message;

  qi::symbols<char, BaseType> bool_type;
  qi::symbols<char, BaseType> fixed_number_types;
  qi::symbols<char, BaseType> floating_point_types;
  qi::symbols<char, BaseType> string_type;
  qi::symbols<char, BaseType> time_types;

  MessageGrammar() : MessageGrammar::base_type(message) {
    using qi::lit;
    using qi::lexeme;
    using qi::eol;
    using spirit::ulong_;
    using spirit::bool_;
    using spirit::long_long;
    using spirit::double_;
    using ascii::char_;
    using phoenix::construct;
    using phoenix::at_c;
    using phoenix::push_back;
    using phoenix::bind;
    using namespace qi::labels;

    bool_type.add("bool", BaseType(BaseType::BOOL));
    fixed_number_types.add
        ("int8", BaseType(BaseType::INT8))
        ("uint8", BaseType(BaseType::UINT8))
        ("int16", BaseType(BaseType::INT16))
        ("uint16", BaseType(BaseType::UINT16))
        ("int32", BaseType(BaseType::INT32))
        ("uint32", BaseType(BaseType::UINT32))
        ("int64", BaseType(BaseType::INT64))
        ("uint64", BaseType(BaseType::UINT64));
    floating_point_types.add
        ("float32", BaseType(BaseType::FLOAT32))
        ("float64", BaseType(BaseType::FLOAT64));
    string_type.add
        ("string", BaseType(BaseType::STRING));
    time_types.add
        ("time", BaseType(BaseType::TIME))
        ("duration", BaseType(BaseType::DURATION));

    basic_id %= +char_("a-zA-Z_0-9_");
    identifier %= lexeme[basic_id];
    
    array_size %= lit('[') >> -ulong_ >> lit(']');
    
    base_type = bool_type | fixed_number_types | floating_point_types
                | string_type | time_types;

    message_type =
        lexeme[ (basic_id >> lit('/') >> basic_id) [ _val = construct<MessageType>(_1, _2) ]]
        | identifier [ _val = construct<MessageType>(_1) ];

    base_type_array = (base_type >> array_size) [
        _val = construct<BaseTypeArray>(_1, _2) ];
    message_type_array = (message_type >> array_size) [
        _val = construct<MessageTypeArray>(_1, _2) ];

    type %= base_type_array | message_type_array | base_type | message_type;

    field = (type >> identifier >> eol) [ _val = construct<Field>(_1, _2) ];

    bool_constant %= bool_;
    fixed_number_constant %= long_long;
    floating_point_constant %= double_;
    string_constant %= lexeme[*(char_ - eol) >> &eol];
    
    constant =
        (bool_type >> identifier >> lit('=') >> bool_constant) [
            _val = construct<Constant>(_1, _2, _3) ]
        | (fixed_number_types >> identifier >> lit('=') >> fixed_number_constant) [
            _val = construct<Constant>(_1, _2, _3) ]
        | (floating_point_types >> identifier >> lit('=') >> floating_point_constant) [
            _val = construct<Constant>(_1, _2, _3) ]
        | (string_type >> identifier >> lit('=') >> string_constant) [
            _val = construct<Constant>(_1, _2, _3) ];

    message = *(
        field [ push_back(at_c<1>(_val), _1) ]
        | constant [ push_back(at_c<0>(_val), _1) ]
        | eol);
  }
};

bool parse_message(const std::string &message, ParsedMessage *result) {
  using qi::phrase_parse;

  std::string::const_iterator iter = message.begin();
  std::string::const_iterator end = message.end();
  
  bool r = phrase_parse(
      iter, end,
      MessageGrammar<std::string::const_iterator>(),
      SkipGrammar<std::string::const_iterator>(),
      *result);
  return r && iter == end;
}

}  // namespace message_parser
