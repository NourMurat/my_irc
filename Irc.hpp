#pragma once

#include <iostream>
#include <cstdlib>  // Для strtol и errno
#include <cstring>  // Для strlen
#include <stdexcept> // Для std::invalid_argument etc
#include <cerrno>    // Для обработки ошибок

#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m" 
#define MAGENTA "\033[35m"   
#define CYAN    "\033[36m" 
#define RESET	"\033[0m"

int     parsingCommandLine(int ac, char **av);
int     checkPort(char *port);
int     checkPass(std::string pass);
