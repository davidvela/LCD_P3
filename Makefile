#/* 
# * Author: David Vela Tirado
# * Software Developer
# * email:  david.vela.tirado@gmail.com
# * 
# */

PHONY: all
CFLAGS= -g -Wall -c
all: servidor cliente 

servidor: servidor.o  funciones.o
	gcc servidor.o funciones.o  -o servidor

cliente: cliente.o  funciones.o
	gcc cliente.o  funciones.o    -o cliente

servidor.o: servidor.c
	gcc $(CFLAGS) servidor.c 

cliente.o: cliente.c
	gcc $(CFLAGS) cliente.c 	

#esto es un comentario
funciones.o: funciones.c
	gcc $(CFLAGS) funciones.c

#parser.o: parser.c
#	gcc $(CFLAGS) parser.c

clean: 
	-rm -f *.o
	-rm -f  servidor cliente
	
