NAME = webserv
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++17
SRC_DIR = src
SRC_FILES = main_config.cpp ParceConf.cpp  #WebServer.cpp
SRCS = $(SRC_FILES:%=$(SRC_DIR)/%)
OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all