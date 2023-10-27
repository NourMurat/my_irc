/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parsing.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: numussan <numussan@student.42abudhabi.a    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/27 16:32:36 by numussan          #+#    #+#             */
/*   Updated: 2023/10/27 18:22:14 by numussan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Irc.hpp"

int     checkPass(std::string pass)
{
    if (pass.find('\n') != std::string::npos || pass.find('\r') != std::string::npos)
    {
        std::cerr << "Error: Password shouldn`t contain Newline or Carriage Return characters\n";
        return 1;
    }
    return 0;
}

int     checkPort(char *port)
{
    int     portNbr;
    try
    {
        size_t pos;
        portNbr = std::stoi(port, &pos);
        if (pos != strlen(port) || portNbr < 1024 || portNbr > 65535)
        {
            std::cerr << "Error: Port number should be in the range 1024-65535 and must be a valid Number\n";
            return 1;
        }
    }
    catch (const std::invalid_argument& e)
    {
        std::cerr << "Error: Port argument is not a valid Number or Empty!\n";
        return 1;
    }
    catch (const std::out_of_range& e)
    {
        std::cerr << "Error: Port number is out of range Integer!\n";
        return 1;
    }
    return 0;
}

int     parsing(int ac, char **av)
{
    char*           port;
    std::string     pass;
    
    if (ac != 3)
    {   
        std::cerr << "Usage: ./ircserv <port> <password>\n";
        return 1;
    }
    
    port = av[1];
    pass = av[2];
    if (checkPort(port) || checkPass(pass))
        return 1;
    
    std::cout << "Starting IRC server on PORT: " << port << " with PASSWORD: " << pass << std::endl;
    return 0;
}