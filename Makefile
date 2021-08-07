NAME = server
FLAGS = -Wall -Wextra -Werror -std=c++98

SRCDIR = ./srcs/
SRC = \
	context.struct.cpp \
	create_autoindex_response.cpp \
	create_get_form_response.cpp \
	execute_cgi_utils.cpp \
	execute_cgi.cpp \
	get_env.cpp \
	handle_GET_HEAD.cpp \
	handle_POST_PUT_DEL.cpp \
	handle_webform_POST.cpp \
	parse_config_utils.cpp \
	parse_config.cpp \
	process_request.cpp \
	r.s.chunked_data.cpp \
	r.s.content_data.cpp \
	r.s.parse_first_line.cpp \
	r.s.parse_headers.cpp \
	request.struct.cpp \
	response_page_creation.cpp \
	response_part_creation.cpp \
	select.cpp \
	server.cpp \
	server.create_listen_sds.cpp \
	server.handle_existing_sd.cpp \
	server.handle_new_sd.cpp \
	server.send_response.cpp \
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
