#pragma once

#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/fusion/include/std_tuple.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <string>
#include <vector>

namespace sql_parser
{
// For x3 see https://www.boost.org/doc/libs/develop/libs/spirit/doc/x3/html/index.html
// The documentation is sparse. Lots of googling, and paying attention to posts by @sehe on stackoverflow
// helps: https://stackoverflow.com/users/85371/sehe.
// The reason for inheriting vectors (conainers), and the operator= in other classes is two-fold.
// First, boost spirit x3 single-element fusion sequences do not work as expected, and second,
// by doing it this way, fusion is not needed in several cases (c++14 takes care of it):
// https://github.com/boostorg/spirit/pull/178 and https://github.com/boostorg/spirit/issues/463.

struct SelectVar
{
    std::string name;   // select @x
};

struct SelectGlob
{
    std::string name;   // select @@z
};

struct SelectFct
{
    std::string name;   // select foo()
};

struct SelectNumber
{
    double value;       // select 1, select 3.14
};

struct SelectString
{
    std::string str;    // select "Hello World!"
};

struct Select;

struct SelectExpr : boost::spirit::x3::variant<
                      SelectVar
                      , SelectGlob
                      , SelectFct
                      , SelectNumber
                      , SelectString
                      >
{
    using base_type::base_type;
    using base_type::operator=;
};

struct Select : public std::vector<SelectExpr>
{
    using std::vector<SelectExpr>::vector;
};

struct ParseError   // Not an actual parser class, will stuff a good error message here
{
    std::string err_msg;
};

struct Command : boost::spirit::x3::variant<
                   Select
                   , ParseError>
{
    using base_type::base_type;
    using base_type::operator=;
};

struct SqlStatement : public std::vector<Command>
{
    using std::vector<Command>::vector;
};

SqlStatement parse_sql(const std::string& sql);
}
