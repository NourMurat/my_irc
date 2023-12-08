#include "irc.hpp"
#include "Server.hpp"
#include <signal.h>

int main(int ac, char **av)
{
    try
    {
        if (parsingCommandLine(ac, av))
            return 1;

        int             port = atoi(av[1]);
        std::string     password = av[2];

        Server          S(port, password);

        signal(SIGINT, Server::sigIntHandler);
        signal(SIGTERM, Server::sigTermHandler);
        S.runServer();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        // delete S;
        return 1;
    }

    // delete S;
    
    return 0;
}
