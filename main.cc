#include "sqltest.hh"
#include <maxbase/string.hh>
#include <iostream>
#include <string>
#include <vector>
#include <map>

struct SelectExecutor
{
    void operator()(const sql_parser::Select& sel)
    {
        for (auto& s : sel)
        {
            boost::apply_visitor(*this, s);
        }
    }

    void operator()(const sql_parser::Variable& var)
    {
        std::cout << "var=" << var.name << " global=" << std::boolalpha << var.is_global << '\n';
    }

    void operator()(const sql_parser::Function& fct)
    {
        std::cout << "func=" << fct.name << '\n';
    }

    void operator()(const sql_parser::Number& nbr)
    {
        std::cout << "number=" << nbr.value << '\n';
    }

    void operator()(const sql_parser::StringIdent& str)
    {
        std::cout << "string=" << str.str << '\n';
    }
};

struct SetExecutor
{
    void operator()(const sql_parser::Set& set)
    {
        boost::apply_visitor(*this, set.setv);
    }

    void operator()(const sql_parser::SetNames& names)
    {
        std::cout << "char_set " << names.char_set << '\n';
    }

    void operator()(const sql_parser::SetSqlMode& mode)
    {
        std::cout << maxbase::join(mode) << "\n";
    }

    void operator()(const sql_parser::SetAutocommit& autocommit)
    {
        std::cout << "autocommit = " << std::boolalpha << autocommit.val << "\n";
    }

    void operator()(const sql_parser::SetAssign& sass)
    {
        for (const auto& ass : sass)
        {
            std::cout << (ass.lhs.is_global ? "@@" : "@") << ass.lhs.name << " <- ";
            boost::apply_visitor(*this, ass.rhs);
            std::cout << '\n';
        }
    }

    void operator()(const sql_parser::Variable& v)
    {
        std::cout << (v.is_global ? "@@" : "@") << v.name;
    }

    void operator()(const sql_parser::Number& n)
    {
        std::cout << n.value;
    }

    void operator()(const sql_parser::StringIdent& si)
    {
        std::cout << si.str;
    }
};

struct ShowExecutor
{
    void operator()(const sql_parser::Show& show)
    {
        boost::apply_visitor(*this, show);
    }

    void operator()(const sql_parser::ShowVariablesLike& var_like)
    {
        std::cout << "var like " << int(var_like.type) << ' ' << var_like.pattern << "\n";
    }

    void operator()(const sql_parser::ShowMisc& gen)
    {
        std::cout << "general show " << int(gen.type) << "\n";
    }

    void operator()(const sql_parser::ShowStatusLike& slike)
    {
        std::cout << "status like " << int(slike.type) << ' ' << slike.pattern << "\n";
    }
};

struct StmtExecutor
{
    void operator()(const sql_parser::SqlStatement& stm)
    {
        for (auto& c : stm)
        {
            boost::apply_visitor(*this, c);
        }
    }

    void operator()(const sql_parser::Select& sel)
    {
        SelectExecutor exec;
        exec(sel);
    }

    void operator()(const sql_parser::Show& sel)
    {
        ShowExecutor exec;
        exec(sel);
    }

    void operator()(const sql_parser::Set& set)
    {
        SetExecutor exec;
        exec(set);
    }

    void operator()(const sql_parser::ParseError& error)
    {
        std::cout << "error " << error.err_msg << '\n';
    }
};

int main()
{
    std::string sql;
    while (getline(std::cin, sql))
    {
        if (sql.empty() || sql[0] == 'q' || sql[0] == 'Q')
        {
            break;
        }
        for (int n = 0; n < 1; ++n)
        {
            auto result = sql_parser::parse_sql(sql);
            StmtExecutor exec;
            exec(result);
        }
    }

    return 0;
}
