NAME = Server

FILES = main.cpp\
		server.cpp

OBJ = $(FILES:.cpp=.o)

CXXFLAGS = -std=c++98 -Wall -Werror -Wextra -g3 

CXX = c++

all: $(NAME) $(OBJ)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re