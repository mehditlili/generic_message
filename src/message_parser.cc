
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
  qi::rule<Iterator, Constant(), SkipGrammar<Iterator> > constant;
  qi::rule<Iterator, ParsedMessage(), SkipGrammar<Iterator> > message;

  qi::symbols<char, BaseType> base_types;
  qi::symbols<char, BaseType> time_types;

  // Hack to allow to return the right parser from parseConstant.
  qi::rule<Iterator, BaseTypes(), SkipGrammar<Iterator> > current_constant_parser;

  MessageGrammar() : MessageGrammar::base_type(message) {
    using qi::lit;
    using qi::lexeme;
    using qi::eol;
    using spirit::ulong_;
    using spirit::lazy;
    using ascii::char_;
    using phoenix::construct;
    using phoenix::at_c;
    using phoenix::push_back;
    using phoenix::bind;
    using namespace qi::labels;

    base_types.add
        ("bool", BaseType(BaseType::BOOL))
        ("int8", BaseType(BaseType::INT8))
        ("uint8", BaseType(BaseType::UINT8))
        ("int16", BaseType(BaseType::INT16))
        ("uint16", BaseType(BaseType::UINT16))
        ("int32", BaseType(BaseType::INT32))
        ("uint32", BaseType(BaseType::UINT32))
        ("int64", BaseType(BaseType::INT64))
        ("uint64", BaseType(BaseType::UINT64))
        ("float32", BaseType(BaseType::FLOAT32))
        ("float64", BaseType(BaseType::FLOAT64))
        ("string", BaseType(BaseType::STRING));

    time_types.add
        ("time", BaseType(BaseType::TIME))
        ("duration", BaseType(BaseType::DURATION));

    basic_id %= +char_("a-zA-Z_0-9_");
    identifier %= lexeme[basic_id];
    
    array_size %= lit('[') >> -ulong_ >> lit(']');
    
    base_type = base_types | time_types;
    constant_type = base_types;

    message_type =
        lexeme[ (basic_id >> lit('/') >> basic_id) [ _val = construct<MessageType>(_1, _2) ]]
        | identifier [ _val = construct<MessageType>(_1) ];

    base_type_array = (base_type >> array_size) [
        _val = construct<BaseTypeArray>(_1, _2) ];
    message_type_array = (message_type >> array_size) [
        _val = construct<MessageTypeArray>(_1, _2) ];

    type %= base_type_array | message_type_array | base_type | message_type;

    field = (type >> identifier >> eol) [ _val = construct<Field>(_1, _2) ];

    constant = (constant_type [ phoenix::ref(current_constant_parser) =
                                bind(&MessageGrammar<Iterator>::parseConstant, _1) ]
                >> identifier >> lit('=')
                >> lazy(phoenix::val(phoenix::ref(current_constant_parser)))
                >> eol) [ _val = construct<Constant>(_1, _2, _3) ];

    message = *(
        field [ push_back(at_c<1>(_val), _1) ]
        | constant [ push_back(at_c<0>(_val), _1) ]
        | eol);
  }

  static qi::rule<Iterator, BaseTypes(), SkipGrammar<Iterator> > parseConstant(
      const Type &type) {
    using spirit::bool_;
    using spirit::int_;
    using spirit::double_;
    using spirit::eps;
    using spirit::eol;
    using spirit::lexeme;
    using ascii::char_;
    
    const BaseType &base_type = boost::get<BaseType>(type);
    switch (base_type.type) {
      case BaseType::BOOL: return bool_;
      case BaseType::INT8:
      case BaseType::UINT8:
      case BaseType::INT16:
      case BaseType::UINT16:
      case BaseType::INT32:
      case BaseType::UINT32:
      case BaseType::INT64:
      case BaseType::UINT64: return int_;
      case BaseType::FLOAT32:
      case BaseType::FLOAT64: return double_;
      case BaseType::STRING: return lexeme[*(char_ - eol) >> &eol];
      default: return eps(false);
    }
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

  std::cout << "parsed: " << std::string(message.begin(), iter) << std::endl;
  std::cout << "not parsed: " << std::string(iter, end) << std::endl;
  
  return r; // && iter == end;
}

}  // namespace message_parser
