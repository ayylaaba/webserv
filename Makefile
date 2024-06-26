NAME = webserv

CXX = c++
 
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 #-fsanitize=address -g3

SRCS = 	main.cpp \
		./cgi/cgi.cpp \
		./cgi/cgi_resp.cpp \
		./multplixing/Client.cpp \
		multplixing.cpp \
		./request/response.cpp \
		./request/request.cpp \
		./server/server.cpp \
		./server/location.cpp \
		./delete/delete.cpp \
		./get/get_method.cpp \
		./post/for_body.cpp \
		./post/for_header.cpp \
		./post/get_extension.cpp \
		./post/helpers.cpp \
		./post/post_methods.cpp \

OBJ = ${SRCS:.cpp=.o}

all : ${NAME}

${NAME} : ${OBJ}
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

clean :
	rm -f ${OBJ}

fclean : clean
	rm -f ${NAME}

re : fclean all