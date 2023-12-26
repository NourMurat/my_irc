NAME = ircserv

FILES = main.cpp				\
		parsingCommandLine.cpp	\
		Server.cpp				\
		User.cpp				\
		Channel.cpp

OBJ = $(FILES:.cpp=.o)

CXXFLAGS = -std=c++98 -Wall -Werror -Wextra -g3

CXX = c++

HEADER = 	irc.hpp		\
			Server.hpp	\
			User.hpp

all: $(NAME) $(OBJ)

$(NAME): $(OBJ) $(HEADER)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re