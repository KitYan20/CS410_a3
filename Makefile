CC = gcc
DEBUG=-g
OPT=-O0
%.o:%.c
	$(CC) -c ${OPT} ${DEBUG} $< -o $@
webserv:webserv.o
	$(CC) ${OPT} ${DEBUG} $^ -o $@  		
clean:
	rm -r *.o webserv