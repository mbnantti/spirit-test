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
// The reason for inheriting vectors (containers), and the operator= in other classes is two-fold.
// First, boost spirit x3 single-element fusion sequences do not work as expected, and second,
// by doing it this way, fusion is not needed in several cases (c++14 takes care of it):
// https://github.com/boostorg/spirit/pull/178 and https://github.com/boostorg/spirit/issues/463.

struct Variable
{
    std::string name;
    bool        is_global;
};

struct Number
{
    double value;
};

/**
 * @brief StringIdent - string or identifier, the context it is used in knows the difference
 */
struct StringIdent
{
    std::string str;
};

/**
 * @brief Function - no arguments need to be supported yet
 */
struct Function
{
    std::string name;
};

// SELECT

struct Select;

struct SelectExpr : boost::spirit::x3::variant<
                      Variable      // select @x
                      , Function    // select foo()
                      , Number      // select 1, select 3.14
                      , StringIdent // select "Hello World!"
                      >
{
    using base_type::base_type;
    using base_type::operator=;
};

struct Select : public std::vector<SelectExpr>
{
    using std::vector<SelectExpr>::vector;
};

// SHOW
enum class Domain {Global, Session, All};

// All 'show' queries are grouped together, meaning they run in the same Visitor. No
// particular reason for that, but it keeps things better organized.
struct ShowVariablesLike    // show [global|session] variables like 'hello'
{
    Domain      type;
    std::string pattern;
};

struct ShowStatusLike   // show [global|session] status like 'hello'
{
    Domain      type;
    std::string pattern;
};

// There are some very specific show queries that do not carry any data, so an enum
// and a single type for them. If needed, they can be split up later.
// show master status
// show slave status
// show slave hosts
// show warnings
// show binary logs
enum class ShowMiscType {MasterStatus, SlaveStatus, SlaveHosts, Warnings, BinaryLogs};
struct ShowMisc
{
    ShowMiscType type;
};

struct Show : boost::spirit::x3::variant<
                ShowVariablesLike
                , ShowStatusLike
                , ShowMisc
                >
{
    using base_type::base_type;
    using base_type::operator=;
};

// SET
struct SetNames
{
    std::string char_set;
};

struct SetSqlMode : public std::vector<std::string>
{
    using std::vector<std::string>::vector;
};

struct SetAutocommit
{
    bool val;
};

struct SetAssignRhs : boost::spirit::x3::variant<
                        Variable
                        , Number
                        , StringIdent
                        >
{
    using base_type::base_type;
    using base_type::operator=;
};

struct SetAssignOne
{
    Variable     lhs;
    SetAssignRhs rhs;
};

struct SetAssign : std::vector<SetAssignOne>
{
    using std::vector<SetAssignOne>::vector;
};

struct SetVariant : boost::spirit::x3::variant<
                      SetNames          // set names latin1
                      , SetSqlMode      // set sql_mode "a,b,c"
                      , SetAutocommit   // set autocommit = true | false | 0 | 1
                      , SetAssign       // set variable = variable | number | string
                      >
{
    using base_type::base_type;
    using base_type::operator=;
};

/**
 * @brief Set - Extra indirection needed for fusion, this is just a SetVariant.
 *              (Maybe the indirection is not needed, it was at one point).
 */
struct Set
{
    SetVariant setv;
};

enum class Slave {Start, Stop, Reset};

struct SlaveCmd
{
    Slave cmd;
};

struct ChangeMasterTo
{
    std::string str;    // the entire key=value pair list as it appears in the sql. TODO
};

/**
 * @brief ParseError - not an actual parser class, this is returned for parsing errors.
 */
struct ParseError
{
    std::string err_msg;
};

struct Command : boost::spirit::x3::variant<
                   Select
                   , Show
                   , Set
                   , SlaveCmd
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
