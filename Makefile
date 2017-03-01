#******************************************************************************#
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: cyildiri <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2016/11/08 19:17:08 by cyildiri          #+#    #+#              #
#    Updated: 2016/11/08 19:17:11 by cyildiri         ###   ########.fr        #
#                                                                              #
#******************************************************************************#

NAME = loc42C
SRC =   main.c
OFILES = $(SRC:.c=.o)

LIBFT = libft/
LIBCURL = /nfs/2016/c/cyildiri/libs/lib/
JANSSON = /nfs/2016/c/cyildiri/cLibs/jansson_lib/

LIBS = -L $(LIBFT) -lft
LIBS += -L $(LIBCURL) -lcurl
LIBS += -L $(JANSSON)/lib -ljansson

LIBI = -I $(LIBFT)
LIBI += -I $(LIBCURL)include
LIBI += -I $(JANSSON)include

all: $(NAME)

$(NAME): dependencies
	gcc -c $(SRC) $(LIBI)
	gcc -o $(NAME) $(OFILES) $(LIBS) -lpthread

clean:
	rm -rf $(OFILES)
	make -C $(LIBFT) clean

fclean: clean
	rm -f $(NAME)
	make -C $(LIBFT) fclean

re: fclean all

dependencies:
	make -C $(LIBFT)

test:
	rm -f $(NAME)
	rm -rf $(OFILES)
	gcc -c $(SRC) $(LIBI)
	gcc -o $(NAME) $(OFILES) $(LIBS) -lpthread

norm:
	norminette *.c *.h ./lib*/*.c ./lib*/*.h