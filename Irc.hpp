#pragma once

#include <iostream>
#include <cstdlib>  // Для strtol и errno
#include <cstring>  // Для strlen
#include <stdexcept> // Для std::invalid_argument etc
#include <cerrno>    // Для обработки ошибок

int     parsingCommandLine(int ac, char **av);
int     checkPort(char *port);
int     checkPass(std::string pass);
