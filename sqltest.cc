#include "sqltest.hh"
#include <maxbase/string.hh>
#include <boost/spirit/home/x3.hpp>
#include <iostream>
#include <iomanip>

// TODO. The parsing does not take case-sensitivity into account yet. Right now nothing is done,
//       so it is fully case-sensitive. Either the incoming string can be lower-cased, or every
//       case-insensitive parser p needs to be modified to x3::no_case[p]. Further, there could be
//       reason to modify the case of some extracted strings?

namespace x3 = boost::spirit::x3;

BOOST_FUSION_ADAPT_STRUCT(sql_parser::SelectVar, name);
BOOST_FUSION_ADAPT_STRUCT(sql_parser::SelectGlob, name);
BOOST_FUSION_ADAPT_STRUCT(sql_parser::SelectFct, name);
BOOST_FUSION_ADAPT_STRUCT(sql_parser::SelectNumber, value);
BOOST_FUSION_ADAPT_STRUCT(sql_parser::SelectString, str);
BOOST_FUSION_ADAPT_STRUCT(sql_parser::ShowVariablesLike, type, pattern);

namespace sql_parser
{

struct ShowTypeKeys : x3::symbols<ShowType>
{
    ShowTypeKeys()
    {
        add ("global", ShowType::GLOBAL)    //
        ("session", ShowType::SESSION);
    }
} const show_var_type_keys;

const x3::rule<struct identifier_tag, std::string> identifier = "identifier";
const x3::rule<struct quoted_tag, std::string> quoted_str = "quoted";

const x3::rule<struct select_var_tag, SelectVar> select_var = "select_var";
const x3::rule<struct select_glob_tag, SelectGlob> select_glob = "select_glob";
const x3::rule<struct select_fct_tag, SelectFct> select_fct = "select_fct";
const x3::rule<struct select_nbr_tag, SelectNumber> select_nbr = "select_nbr";
const x3::rule<struct select_str_tag, SelectString> select_str = "select_str";
const x3::rule<struct select_tag, Select> select = "select";

const x3::rule<struct show_vlike_tag, ShowVariablesLike> show_vlike = "show_vlike";
const x3::rule<struct select_tag, Show> show = "show";

const x3::rule<struct select_tag, SqlStatement> sql_stmt = "stmt";

const auto identifier_def = x3::lexeme[(x3::alpha | x3::char_('_')) >> *(x3::alnum | x3::char_('_'))];
const auto quoted_str_def = x3::lexeme['"' >> +(('\\' >> x3::char_) | (x3::char_ - '"')) >> '"']
    | x3::lexeme['\'' >> +(('\\' >> x3::char_) | (x3::char_ - '\'')) >> '\''];

const auto select_var_def = '@' >> identifier;
const auto select_glob_def = "@@" >> -x3::lexeme[x3::lit("global.") >> identifier];
const auto select_fct_def = identifier >> x3::lit("()");
const auto select_nbr_def = x3::double_;
const auto quote = x3::omit[x3::char_("'")];
const auto select_str_def = quoted_str;
const auto select_expr = select_var | select_glob | select_fct | select_nbr | select_str;
const auto select_limit = "limit" >> x3::int_;
const auto select_def = x3::no_case["select"] >> select_expr % "," >> -x3::omit[select_limit];

const auto show_vlike_def = (show_var_type_keys | x3::attr(ShowType::ALL)) >> "variables"
    >> "like" >> quoted_str;
const auto show_def = "show" >> show_vlike;

const auto sql_stmt_def = (select | show) % ';' >> -x3::omit[";"];

BOOST_SPIRIT_DEFINE(identifier, quoted_str,
                    select_var, select_glob, select_fct, select_nbr, select_str, select,
                    show_vlike, show,
                    sql_stmt);

SqlStatement parse_sql(const std::string& sql)
{
    SqlStatement stmt;
    auto first = begin(sql);
    auto success = phrase_parse(first, end(sql), sql_stmt, x3::space, stmt);

    if (success && first == end(sql))
    {
        return stmt;
    }
    else
    {
        Command error {ParseError {"error"}};
        return SqlStatement {error};
    }
}
}
