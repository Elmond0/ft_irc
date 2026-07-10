MAKEFLAGS += --no-print-directory

NAME = ircserv

CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -std=c++98

INC_DIR = inc

OBJ_DIR = obj

SRC_DIR = src

CMD_DIR = cmd

SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/Server.cpp $(SRC_DIR)/Client.cpp \
       $(SRC_DIR)/Channel.cpp $(SRC_DIR)/Parser.cpp $(SRC_DIR)/dispatch.cpp \
       $(SRC_DIR)/utils.cpp

CMDS = $(CMD_DIR)/PASS.cpp $(CMD_DIR)/NICK.cpp $(CMD_DIR)/USER.cpp \
       $(CMD_DIR)/JOIN.cpp $(CMD_DIR)/PRIVMSG.cpp $(CMD_DIR)/NOTICE.cpp \
       $(CMD_DIR)/KICK.cpp $(CMD_DIR)/INVITE.cpp $(CMD_DIR)/TOPIC.cpp \
       $(CMD_DIR)/MODE.cpp $(CMD_DIR)/QUIT.cpp $(CMD_DIR)/PING.cpp \
       $(CMD_DIR)/PART.cpp

OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o) $(CMDS:$(CMD_DIR)/%.cpp=$(OBJ_DIR)/cmd/%.o)

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

$(OBJ_DIR)/cmd/%.o : $(CMD_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@
	@echo -e " $(GRAY) ● $< compiled$(RESET)"

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