#include "sqltest.hh"
#include <boost/fusion/include/std_tuple.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <iostream>
#include <iomanip>

namespace x3 = boost::spirit::x3;

struct SelVar
{
    std::string name;
};

struct SelGlob
{
    std::string name;
};

struct SelFct
{
    std::string name;
};

struct SqlStatement;

struct Command : x3::variant<SelVar, SelGlob, SelFct, x3::forward_ast<SqlStatement>>
{
    using base_type::base_type;
    using base_type::operator=;
};

struct SqlStatement
{
    Command cmd;
};

BOOST_FUSION_ADAPT_STRUCT(SelVar, name);
BOOST_FUSION_ADAPT_STRUCT(SelGlob, name);
BOOST_FUSION_ADAPT_STRUCT(SelFct, name);
BOOST_FUSION_ADAPT_STRUCT(SqlStatement, cmd);

struct TestExecutor
{
    void operator()(const SqlStatement& stm)
    {
        boost::apply_visitor(*this, stm.cmd);
    }

    void operator()(const SelVar& sel_var)
    {
        std::cout << "select variable " << sel_var.name << '\n';
    }

    void operator()(const SelGlob& sel_glob)
    {
        std::cout << "select global " << sel_glob.name << '\n';
    }

    void operator()(const SelFct& sel_fct)
    {
        std::cout << "select function " << sel_fct.name << '\n';
    }
};

struct identifier_tag;

const x3::rule<identifier_tag, std::string> identifier = "identifier";
const x3::rule<struct sel_var_tag, SelVar> sel_var = "sel_var";
const x3::rule<struct sel_var_tag, SelGlob> sel_glob = "sel_glob";
const x3::rule<struct sel_fct_tag, SelFct> sel_fct = "sel_fct";
const x3::rule<struct select_tag, SqlStatement> sql_select = "select";

const auto identifier_def = x3::lexeme[(x3::alpha | '_') >> *(x3::alnum | '_')];
const auto sel_var_def = '@' >> identifier;
const auto sel_glob_def = "@@" >> identifier;
const auto sel_fct_def = identifier >> x3::lit("()");
const auto sql_select_def = "select" >> (sel_var | sel_glob | sel_fct);

BOOST_SPIRIT_DEFINE(identifier, sel_var, sel_glob, sel_fct, sql_select);

SqlTest::SqlTest(const std::string& sql)
{
    SqlStatement stm;
    auto success = phrase_parse(begin(sql), end(sql), sql_select, x3::space, stm);
    if (success)
    {
        TestExecutor exec;
        exec(stm);
    }
    else
    {
        std::cout << "err\n";
    }
}
