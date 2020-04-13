#include "sqltest.hh"

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

    void operator()(const sql_parser::SelectVar& var)
    {
        std::cout << "var=" << var.name << '\n';
    }

    void operator()(const sql_parser::SelectGlob& glob)
    {
        std::cout << "glob=" << glob.name << '\n';
    }

    void operator()(const sql_parser::SelectFct& fct)
    {
        std::cout << "func=" << fct.name << '\n';
    }

    void operator()(const sql_parser::SelectNumber& nbr)
    {
        std::cout << "number=" << nbr.value << '\n';
    }

    void operator()(const sql_parser::SelectString& str)
    {
        std::cout << "string=" << str.str << '\n';
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
