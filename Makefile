MAKEFLAGS += --no-print-directory

NAME = ircserv

CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -std=c++98 

OBJ_DIR = obj

SRC_DIR = src

SRCS = 

OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME) : $(OBJS)
	@$(CXX) $(CXXFLAGS) $^ -o $@
	@echo "\033[32m$(NAME) created!\033[0m"

$(OBJ_DIR) :
	@mkdir -p obj

$(OBJ_DIR)/.o : $(SRC_DIR)/.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $< -o $@
	@echo -e "\033[90m$< compiled\033[0m"

clean :
	@rm -rf $(OBJ_DIR)

fclean : clean
	rm -f $(NAME)

re : fclean all

.PHONY : all clean fclean re