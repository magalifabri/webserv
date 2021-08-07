NAME = server
FLAGS = -Wall -Wextra -Werror -std=c++98

SRCDIR = ./srcs/
SRC = \
	context.struct.cpp \
	execute_cgi.cpp \
	execute_cgi.utils.cpp \
	get_env.cpp \
	parse_config.cpp \
	parse_config.utils.cpp \
	request_to_response.2xx_and_error.cpp \
	request_to_response.GET_HEAD.cpp \
	request_to_response.POST_PUT_DEL.cpp \
	request_to_response.autoindex.cpp \
	request_to_response.cpp \
	request_to_response.form_GET.cpp \
	request_to_response.form_POST.cpp \
	request_to_response.utils.cpp \
	request.struct.chunked.cpp \
	request.struct.content_data.cpp \
	request.struct.cpp \
	request.struct.parse_first_line.cpp \
	request.struct.parse_headers.cpp \
	select.cpp \
	server.cpp \
	server.create_listen_sds.cpp \
	server.handle_selected_sds.cpp \
	set_directive.cpp \
	set_directive.utils.cpp \
	tests.cpp \
	utils.cpp \
	webserv.struct.cpp
SRCS = $(addprefix $(SRCDIR), $(SRC))

OBJDIR = ./obj/
OBJ = $(SRC:.cpp=.o)
OBJS = $(addprefix $(OBJDIR), $(OBJ))

INCDIR = ./includes/
INC = \
	context.struct.hpp \
	request.struct.hpp \
	webserv.hpp
INCS = $(addprefix $(INCDIR), $(INC))

all: $(NAME)
$(NAME): $(OBJDIR) $(OBJS) $(INCS)
	clang++ $(FLAGS) -I$(INCDIR) $(OBJS) -o $(NAME)

$(OBJDIR)%.o: $(SRCDIR)%.cpp $(INCS)
	clang++ $(FLAGS) -I$(INCDIR) -c -o $@ $<

$(OBJDIR):
	@ mkdir -p $(OBJDIR)

# MISC
clean:
	@ /bin/rm -rf $(OBJDIR)

fclean: clean
	@ /bin/rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re
