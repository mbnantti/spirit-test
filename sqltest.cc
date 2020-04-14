#include "sqltest.hh"
#include <maxbase/string.hh>
#include <boost/spirit/home/x3.hpp>
#include <iostream>
#include <iomanip>

// TODO. The parsing does not take case-sensitivity into account yet. Right now nothing is done,
//       so it is fully case-sensitive. Either the incoming string can be lower-cased, or every
//       case-insensitive parser p needs to be modified to x3::no_case[p]. Further, there could be
//       reason to modify the case of some extracted strings?

// Literal ints x3::int_(42) do not work (the constructor is not there!). If needed,
// nice workaround https://github.com/boostorg/spirit/issues/390

namespace x3 = boost::spirit::x3;

BOOST_FUSION_ADAPT_STRUCT(sql_parser::Variable, name, is_global);
BOOST_FUSION_ADAPT_STRUCT(sql_parser::Function, name);
BOOST_FUSION_ADAPT_STRUCT(sql_parser::Number, value);
BOOST_FUSION_ADAPT_STRUCT(sql_parser::StringIdent, str);
BOOST_FUSION_ADAPT_STRUCT(sql_parser::ShowVariablesLike, type, pattern);
BOOST_FUSION_ADAPT_STRUCT(sql_parser::ShowStatusLike, type, pattern);
BOOST_FUSION_ADAPT_STRUCT(sql_parser::ShowMisc, type);
BOOST_FUSION_ADAPT_STRUCT(sql_parser::SetNames, char_set);
// BOOST_FUSION_ADAPT_STRUCT(sql_parser::SetSqlMode, val);
BOOST_FUSION_ADAPT_STRUCT(sql_parser::SetAutocommit, val);
BOOST_FUSION_ADAPT_STRUCT(sql_parser::SetAssignOne, lhs, rhs);
BOOST_FUSION_ADAPT_STRUCT(sql_parser::Set, setv);

namespace sql_parser
{

struct ShowTypeKeys : x3::symbols<Domain>
{
    ShowTypeKeys()
    {
        add ("global", Domain::Global)      //
        ("session", Domain::Session);
    }
} const domain_keys;

const x3::rule<struct identifier_tag, std::string> identifier = "identifier";
const x3::rule<struct quoted_tag, std::string> quoted_str = "quoted";
const x3::rule<struct number_tag, Number> number = "number";
const x3::rule<struct boolean_tag, bool> boolean = "boolean";

const x3::rule<struct variable_tag, Variable> variable = "variable";
const x3::rule<struct function_tag, Function> function = "function";
const x3::rule<struct select_str_tag, StringIdent> select_str = "select_str";
const x3::rule<struct select_tag, Select> select = "select";

const x3::rule<struct show_var_like_tag, ShowVariablesLike> show_var_like = "show_var_like";
const x3::rule<struct show_status_like_tag, ShowStatusLike> show_status_like = "show_status_like";
const x3::rule<struct show_misc_tag, ShowMisc> show_misc = "show_misc";
const x3::rule<struct select_tag, Show> show = "show";

const x3::rule<struct set_names_tag, SetNames> set_names = "set_names";
const x3::rule<struct set_sql_mode_tag, SetSqlMode> set_sql_mode = "set_sql_mode";
const x3::rule<struct set_autocommit_tag, SetAutocommit> set_autocommit = "set_autocommit";
const x3::rule<struct set_assign_st_tag, StringIdent> set_assign_str = "set_assign_str";
const x3::rule<struct set_assign_tag, SetAssign> set_assign = "set_assign";
const x3::rule<struct set_tag, Set> set = "set";

const x3::rule<struct select_tag, SqlStatement> sql_stmt = "stmt";

const auto identifier_def = x3::lexeme[(x3::alpha | x3::char_('_')) >> *(x3::alnum | x3::char_('_'))];
const auto quoted_str_def = x3::lexeme['"' >> +(('\\' >> x3::char_) | (x3::char_ - '"')) >> '"']
    | x3::lexeme['\'' >> +(('\\' >> x3::char_) | (x3::char_ - '\'')) >> '\''];
const auto session_var = '@' >> identifier >> x3::attr(false);
const auto global_var = "@@" >> x3::lexeme[-x3::lit("global.") >> identifier] >> x3::attr(true);
const auto variable_def = session_var | global_var;
const auto function_def = identifier >> x3::lit("()");
const auto boolean_def = ((x3::lit("true") | "1") >> x3::attr(true)
                          | (x3::lit("false") | "0") >> x3::attr(false));

const auto number_def = x3::double_;
const auto select_str_def = quoted_str;
const auto select_expr = variable | function | number | select_str;
const auto select_limit = "limit" >> x3::int_;
const auto select_def = x3::no_case["select"] >> select_expr % "," >> -x3::omit[select_limit];

const auto show_var_like_def = (domain_keys | x3::attr(Domain::All)) >> "variables" >> "like" >> quoted_str;
const auto show_status_like_def = (domain_keys | x3::attr(Domain::All)) >> "status" >> "like" >> quoted_str;
const auto master_stat = x3::lit("master") >> "status" >> x3::attr(ShowMiscType::MasterStatus);
const auto slave_stat = x3::lit("slave") >> "status" >> x3::attr(ShowMiscType::SlaveStatus);
const auto slave_hosts = x3::lit("slave") >> "hosts" >> x3::attr(ShowMiscType::SlaveHosts);
const auto warnings = x3::lit("warnings") >> x3::attr(ShowMiscType::Warnings);
const auto binlogs = x3::lit("binary") >> "logs" >> x3::attr(ShowMiscType::BinaryLogs);
const auto show_misc_def = master_stat | slave_stat | slave_hosts | warnings | binlogs;
const auto show_def = "show" >> (show_var_like | show_status_like | show_misc);

const auto set_names_def = "names" >> (quoted_str | x3::lexeme[*x3::char_]);
const auto set_sql_mode_def = x3::lit("sql_mode") >> '='
    >> (('"' >> (*(x3::char_ - '"') % ',') >> '"')
        | ('\'' >> (*(x3::char_ - '\'') % ',') >> '\''));
const auto set_autocommit_def = x3::lit("autocommit") >> '=' >> boolean;
const auto set_assign_str_def = quoted_str;
const auto set_assign_expr = variable >> '=' >> (variable | number | set_assign_str);
const auto set_assign_def = set_assign_expr % ',';
const auto set_def = "set" >> (set_names | set_sql_mode | set_autocommit | set_assign);

const auto sql_stmt_def = (select | set) % ';' >> -x3::omit[";"];

BOOST_SPIRIT_DEFINE(identifier, quoted_str, boolean,
                    variable, function, number, select_str, select,
                    show_var_like, show_status_like, show_misc, show,
                    set_names, set_sql_mode, set_autocommit, set_assign_str, set_assign, set,
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
