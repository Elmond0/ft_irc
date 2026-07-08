MAKEFLAGS += --no-print-directory

NAME = ircserv

CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -std=c++98

INC_DIR = inc

OBJ_DIR = obj

SRC_DIR = src

SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/Server.cpp $(SRC_DIR)/Client.cpp \
       $(SRC_DIR)/Parser.cpp $(SRC_DIR)/dispatch.cpp $(SRC_DIR)/utils.cpp \
       $(SRC_DIR)/handle_PASS.cpp $(SRC_DIR)/handle_NICK.cpp $(SRC_DIR)/handle_USER.cpp

OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

GREEN := $(shell tput setaf 2)
RED := $(shell tput setaf 1)
YELLOW := $(shell tput setaf 3)
GRAY := $(shell tput setaf 8)
RESET := $(shell tput sgr0)

all: $(NAME)

$(NAME) : $(OBJ_DIR) $(OBJS)
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $@
	@echo " $(GREEN) ✔ $(NAME) created!$(RESET)"

$(OBJ_DIR) :
	@mkdir -p obj

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@
	@echo " $(GRAY) ● $< compiled$(RESET)"

clean :
	@for f in $(OBJS); do \
		[ -f $$f ] && rm -f $$f && echo " $(YELLOW) ✘ $$f removed$(RESET)" || true; \
	done
	@rm -rf $(OBJ_DIR)

fclean : clean
	@rm -f $(NAME)
	@echo " $(RED) ✘ $(NAME) wiped$(RESET)"

re : fclean all

.PHONY : all clean fclean re