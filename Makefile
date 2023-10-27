NAME = ircserv

SRCS = main.cpp Parsing.cpp

OBJS = $(SRCS:.cpp=.o)

CFLAGS = c++ -Wall -Wextra -Werror -std=c++98

HEADER = ./Irc.hpp

RM = rm -rf

all: $(NAME)

$(NAME): $(OBJS) $(HEADER)
	$(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
