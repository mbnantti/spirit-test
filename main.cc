#include <sqltest.hh>

#include <boost/fusion/include/std_tuple.hpp>
#include <boost/spirit/home/x3.hpp>
#include <maxbase/stopwatch.hh>
#include <maxbase/string.hh>

#include <iostream>
#include <string>
#include <vector>
#include <map>

namespace x3 = boost::spirit::x3;

bool parse_gtid(const std::string& gtid_str,
                uint32_t& domain,
                uint32_t& server_id,
                uint64_t& sequence_nr)
{
    const auto uint32_parser = x3::uint_parser<uint32_t, 10>();
    const auto uint64_parser = x3::uint_parser<uint64_t, 10>();
    const auto gtid_parser = uint32_parser >> '-' >> uint32_parser >> '-' >> uint64_parser;

    std::tuple<uint32_t, uint32_t, uint64_t> result;

    bool success = x3::parse(begin(gtid_str), end(gtid_str), gtid_parser, result);

    if (success)
    {
        domain = std::get<0>(result);
        server_id = std::get<1>(result);
        sequence_nr = std::get<2>(result);
    }
    else
    {
        std::cout << "Invalid\n";
    }

    return success;
}

int main1()
{
    std::string gtid_str;
    while (getline(std::cin, gtid_str))
    {
        if (gtid_str.empty() || gtid_str[0] == 'q' || gtid_str[0] == 'Q')
        {
            break;
        }

        uint32_t domain;
        uint32_t server_id;
        uint64_t seq_nr;
        bool success;
        maxbase::StopWatch sw;
        for (int n = 0; n < 1; ++n)
        {
            success = parse_gtid(gtid_str, domain, server_id, seq_nr);
        }
        std::cout << sw.split() << '\n';

        if (success)
        {
            std::cout << "OK " << domain << ',' << server_id << ',' << seq_nr << '\n';
        }
        else
        {
            std::cout << "Invalid gtid_str " << gtid_str << "\n";
        }
    }

    return 0;
}

// struct TestExecutor
// {
//    void operator()(const sql_parser::SqlStatement& stm)
//    {
//        for (auto& c : stm)
//        {
//            boost::apply_visitor(*this, c);
//        }
//    }

//    void operator()(const sql_parser::SelectVar& var)
//    {
//        std::cout << "var=" << var.name << '\n';
//    }

//    void operator()(const sql_parser::SelectGlob& glob)
//    {
//        std::cout << "glob=" << glob.name << '\n';
//    }

//    void operator()(const sql_parser::ParseError& error)
//    {
//        std::cout << "error " << error.err_msg << '\n';
//    }
// };

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
            // TestExecutor exec;
            // exec(result);
            SelectExecutor sel;
            sel(result);
        }
    }

    return 0;
}
