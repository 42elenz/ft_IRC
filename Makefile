NAME = ircserv
SRCS = server.cpp main.cpp channel.cpp user.cpp
OBJS := ${SRCS:.cpp=.o}
CC = c++
CFLAGS = -g -Wall -Wextra -Werror -std=c++98

all: $(NAME)
%.o: %.cpp $(HDR)
	$(CC) $(CFLAGS) -c -o $@ $<
$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
clean:
	rm -f $(OBJS)
fclean: clean
	rm -f $(NAME)
re: fclean all

.PHONY: all clean fclean re

