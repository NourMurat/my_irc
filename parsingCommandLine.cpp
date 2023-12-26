#include "Irc.hpp"
// #include "Server.hpp" //only for colors

int parsingCommandLine(int ac, char **av)
{
	char *port;
	std::string pass;

	if (ac != 3)
	{
		std::cerr << RED << "ERROR! Usage: ./ircserv <port> <password>\n\n" << RESET;
		return 1;
	}

	port = av[1];
	pass = av[2];
	if (checkPort(port) || checkPass(pass))
		return 1;

	std::cout << YELLOW << "Starting IRC server on PORT: " << port << " with PASSWORD: " << pass << RESET << std::endl;
	return 0;
}

int checkPort(char *port)
{
	char *endptr;
	long portNbr;
	if (port == NULL || strlen(port) == 0)
	{
		throw std::invalid_argument("ERROR! Port argument is Empty!\n");
	}

	errno = 0; // Обнуление errno перед вызовом strtol
	portNbr = std::strtol(port, &endptr, 10);
	if (endptr == port)
	{
		throw std::invalid_argument("ERROR! No digits were found.\n");
	}
	if (errno == ERANGE || portNbr < 1024 || portNbr > 65535)
	{
		throw std::out_of_range("ERROR! Port number should be in the range 1024-65535.\n");
	}
	if (*endptr != '\0')
	{
		throw std::invalid_argument("ERROR! Port argument contains non-numeric characters.\n");
	}
	return 0;
}

int checkPass(std::string pass)
{
	if (pass.empty())
	{
		throw std::invalid_argument("ERROR! Password is empty.\n");
	}
	for (std::size_t i = 0; i < pass.length(); ++i)
	{
		if (std::isspace(static_cast<unsigned char>(pass[i])))
		{
			throw std::invalid_argument("ERROR! Password with spaces.\n");
		}
	}
	return 0;
}
