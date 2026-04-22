NAME		= codexion
HEADERS	= coders/codexion.h

CC			= cc
CFLAGS		= -Wall -Wextra -Werror -pthread

SRCS_FILES	= coder.c codexion.c dongle.c heap.c init.c monitor.c parse.c utils.c
SRCS		= $(addprefix coders/, $(SRCS_FILES))
OBJS		= $(SRCS:.c=.o)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
