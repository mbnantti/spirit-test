#include "sqltest.hh"

#include <iostream>
#include <string>

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
            SqlTest s(sql);
        }
    }

    return 0;
}
