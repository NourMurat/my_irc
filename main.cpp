#include "Irc.hpp"
#include "Server.hpp"

int main(int ac, char **av)
{
    try
    {
        if (parsingCommandLine(ac, av))
            return 1;

        int             port = atoi(av[1]);
        std::string     password = av[2];

        Server          S(port, password);
        S.runServer();
    }
    catch(const std::exception& e)
    {
        std::cerr << RED << e.what() << RESET << '\n';
        return 1;
    }

    return 0;
}
