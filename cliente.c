 /* 
# * Author: David Vela Tirado
# * Software Developer
# * email:  david.vela.tirado@gmail.com
# * 
  */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
//#include "parser.h"
#include <arpa/inet.h>
#include <signal.h>
#include "estructuras.h"
#include "funciones.h"

#define MAXBUFF 100		 //longitud máxima mensajes replicar con ECO
#define PORTSERV 10021 		 //puerto del servidor
#define PORTCLIE  5006 		 //puerto del cliente


int main()		// ./cliente  
{

int ds_udp; 				//descriptor del socket
int long_serv, long_cli;	//LONGITUDES...
tipo_mq mq=MQ_CLIENTE; 

struct sockaddr_in servidor, cliente;  //info. servidor y cliente 
struct openCmd_t s_open;			//Orden HELLO
struct ackCmd_t s_respuesta;		//Orden ACK o NACK
struct listCmd_t s_list;			//Orden LIST
struct transferCmd_t s_read,s_write;	//Orden READ o WRITE
struct CmdPkt_t s_paquete;		//Paquete de órdenes
//parametros del parser
	char expresion[MAX_CMD];  
	char ** items;
  	int i, num, background,n,m;	//contadores.....
//parametros de conexion:
	int puerto;
	char dirIP[50];  
	int conexion=0;	//si conexion=1 ->conexion establecida (UDP)
	int close_con=0; 	
	char *buff;	//tamaño maximo de la orden.
  		buff = malloc ((ENVIO)*sizeof(char*));  // Pedir array
	char *buff_envio;	//tamaño maximo de la orden.
  		buff_envio = malloc ((ENVIO)*sizeof(char*));  // Pedir array

//parametros de conexion tcp:
int ds_tcp; 				//descriptor del socket
struct sockaddr_in destino_tcp;  //info. servidor y cliente 
int long_desttcp ;	//LONGITUDES...
int puerto_tcp,iteracion,respu,tam_bloque;

//creo socket para comunicarme via udp:	
	if( (ds_udp = socket(AF_INET, SOCK_DGRAM, 0) ) == -1)
	{ 	error("socket");	}		

	do
	{
		//limpiar buffer:
			for(i=0; i<ENVIO; i++) {buff[i]=0;buff_envio[i]=0;}
			strcpy(s_respuesta.info," ");
	  	printf ("clienteftp>  ");
	  	fgets (expresion, MAX_CMD, stdin);
	  	num = separaItems (expresion, &items, &background);
	 	//printf ("Numero de parametros: %d\n", num);	
		s_paquete.cmdType=0;
		if(num!=0)
		{
			if( !strcmp(items[0],"open") )	{ s_paquete.cmdType=CMD_OPEN; }// printf("orden open \n");}
			if( !strcmp(items[0],"list") )	{ s_paquete.cmdType=CMD_LIST; }// printf("orden list \n");}
			if( !strcmp(items[0],"read") )	{ s_paquete.cmdType=CMD_READ; }// printf("orden read \n");}
			if( !strcmp(items[0],"write") )	{ s_paquete.cmdType=CMD_WRITE;}//printf("orden write\n");}
			if( !strcmp(items[0],"close") )	{ s_paquete.cmdType=CMD_CLOSE;}//printf("orden close\n");}
			if( !strcmp(items[0],"exit") )	{ s_paquete.cmdType=CMD_EXIT; }//printf("orden exit \n");}
		}

	switch (s_paquete.cmdType)
	{	
		case (CMD_OPEN):	//open dir puerto username password 
					//ejemplo: open 127.0.0.1 10021 user1 key1
		{	
			if(conexion==1){ printf("Ya has establecido una conexion\n"); break;}
			
			if(num!= 5) { printf("Parametros incorrectos, introducir: open IP port username password.\n");
					break;}
			strcpy(s_open.username,items[3]);
			strcpy(s_open.password,items[4]);
			//printf("usuario: %s \tcontraseña: %s \n",s_open.username, s_open.password);
			//compruebo puerto:
				puerto=atoi(items[2]);			//si puerto no es 10021 dara error.
				strcpy(dirIP,items[1]);
				if(puerto !=PORTSERV){ printf("\nError: puerto incorrecto.\n"); break;}			
			bzero( (char*) & servidor, sizeof (servidor));   //inicializo a cero la estructura
			bzero( (char*) & cliente, sizeof (cliente));   //inicializo a cero la estructura
			cliente.sin_family = AF_INET; 			//familia	
			cliente.sin_port = htons (puerto); 	//formato red 
			cliente.sin_addr.s_addr = inet_addr(items[1]); 		// IP del servidor
			long_cli = sizeof(cliente);	
			long_serv = sizeof(servidor);

				//if( bind(ds, (struct sockaddr *) &cliente, long_cli) == -1)
				//{	error("bind UDP");		}break;
					s_paquete.cmdType=CMD_OPEN;
					empaquetar(&s_paquete,&s_open);
					serializar(buff_envio,&s_paquete);
					m= sendto (ds_udp, buff_envio , ENVIO, 0, (struct sockaddr *) &cliente, long_cli);

					if(m == -1)	{ error("sendto");}
					if(m != ENVIO)	{printf("\nse han perdido caracteres\n");}

				//printf("\nLe he enviado esta cadena al servidor: %s\n",buff_envio);
				//recibo contestacion:
					n= recvfrom(ds_udp, buff, ENVIO, 0, (struct sockaddr *) &servidor, &long_serv);
	
					if(n == -1)	{error("recvfrom");}
					//printf("\nhe recibido: %s\n",buff);
					deserializar(buff,&s_paquete);
					//analizar paquete:					
					if((int)s_paquete.cmdType==CMD_ACK)
					{	 printf("ACK\n"); printf("Session started at %s:%d\n",items[1],puerto); 						conexion=1;close_con=0;
					}
					else
					{	desempaquetar(&s_paquete,&s_respuesta);	printf("NACK\n");
						printf("error: %s\n",s_respuesta.info);				
					}
			break;		
		}
			
		case(CMD_LIST):	//list ruta
		{			
					// compruebo conexion, empaquetar, serializar (list), enviar sendto
					if(conexion==0){ printf("No tienes establecida una conexion\n"); break;}
					if(num==1){ printf("Introduce una ruta\n"); break;}
					strcpy(s_list.ruta,items[1]);
					if(num>2) { printf("Error demasiados parametros\n"); break;}
					//printf("ruta:%s \n",s_list.ruta);
					s_paquete.cmdType=CMD_LIST;
					empaquetar(&s_paquete,&s_list);
					serializar(buff_envio,&s_paquete);
				m= sendto (ds_udp, buff_envio , ENVIO, 0, (struct sockaddr *) &cliente, long_cli);

					if(m == -1)	{ error("sendto");}
					if(m != ENVIO)	{printf("\nse han perdido caracteres\n");}
					//despues->recibo->desempaqueto ack(contenido) o nack (error) 
	
					n= recvfrom(ds_udp, buff, ENVIO, 0, (struct sockaddr *) &servidor, &long_serv);
	
					if(n == -1)	{error("recvfrom");}
					deserializar(buff,&s_paquete);
					//analizar paquete:					
					if((int)s_paquete.cmdType==CMD_ACK)
					{   	desempaquetar(&s_paquete,&s_respuesta);
						printf("ACK\n"); printf("archivos: %s\n",s_respuesta.info);
					}
					else
					{   	desempaquetar(&s_paquete,&s_respuesta);	printf("NACK\n");
						printf("error: %s\n",s_respuesta.info);
					}
					break;
		}   
				
		case(CMD_READ):	//read(ruta, fichero)
		{		//(compruebo conexion) empaquetar serializar read,
				if(conexion==0){ printf("No tienes establecida una conexion\n"); break;}		
				if(num!=3){printf("Error de formato: read ruta fichero\n"); break;}
					strcpy(s_read.ruta,items[1]);
					strcpy(s_read.fichero,items[2]);
				s_paquete.cmdType=CMD_READ;
				empaquetar(&s_paquete,&s_read);
				serializar(buff_envio,&s_paquete);
				m= sendto (ds_udp, buff_envio , ENVIO, 0, (struct sockaddr *) &cliente, long_cli);

					if(m == -1)	{ error("sendto");}
					if(m != ENVIO)	{printf("\nse han perdido caracteres\n");}
				//recibo ack(puerto), o nack (error); 
				n= recvfrom(ds_udp, buff,ENVIO, 0, (struct sockaddr *) &servidor, &long_serv);

					if(n == -1)	{error("recvfrom");}
				deserializar(buff,&s_paquete);
				if((int)s_paquete.cmdType==CMD_NACK)
				{		desempaquetar(&s_paquete,&s_respuesta);	printf("NACK\n");
						printf("error: %s\n",s_respuesta.info);
				}
				else//->informacion = puerto + tamaño bloque + tamaño del archivo;
				{//realizar conexion tcp, dividir y hacer un for para llamar a recibir fichero

					desempaquetar(&s_paquete,&s_respuesta);	printf("ACK\n");
					printf("info: %s\n",s_respuesta.info);
					//saco el puerto y el tamaño de bloque de la informacion
					liberaItems (items);
					num = separaItems (s_respuesta.info, &items, &background);
					puerto_tcp=atoi(items[0]); //puerto
					tam_bloque=atoi(items[1]); //tamaño de bloque
					
					//inicializo la estructura destino:
					bzero( (char*) & destino_tcp, sizeof (destino_tcp));   //inicializo a cero la estructura
					destino_tcp.sin_family = AF_INET; 			//familia	
					destino_tcp.sin_port = htons (puerto_tcp); 	//formato red 	
					destino_tcp.sin_addr.s_addr = inet_addr(dirIP); 	// IP de servidor
					long_desttcp = sizeof(destino_tcp);
					if( (ds_tcp = socket(AF_INET, SOCK_STREAM, 0) ) == -1)
					{	error("socket");	}

					if( connect(ds_tcp, (struct sockaddr *) &destino_tcp, long_desttcp) == -1)
					{	error("connect");		}
			
					//llamo a la funcion recibir fichero
					printf("Starting transfer: %s  ",s_read.fichero);

					respu=recibir_fichero(ds_tcp,s_read,tam_bloque,iteracion,mq);
						
						iteracion=0;
						if(respu==0)
						{	
							iteracion=0;
							printf("done\n");
							close(ds_tcp);
							break;
						}
						
		

					

				}						



		break;

		}
		case(CMD_WRITE):	//write(ruta, fichero)
		{			//desempaquetar write, comprobar ruta, fichero...
					if(conexion==0){ printf("No tienes establecida una conexion\n"); break;}		
				if(num!=3){printf("Error de formato: read ruta fichero\n"); break;}
					strcpy(s_write.ruta,items[1]);
					strcpy(s_write.fichero,items[2]);
				//compruebo en el cliente que la ruta y el fichero que quiero enviar al servidor son correctos
				s_respuesta.inReplyTo=0;	
				m=comprobacion_transfer(&s_write,&s_respuesta);
				if(m!=0)//error
				{	printf("No a sido posible enviar el fichero debido a que: %s.\n",s_respuesta.info);
					break;
				}
				s_paquete.cmdType=CMD_WRITE;
				empaquetar(&s_paquete,&s_write);
				serializar(buff_envio,&s_paquete);
				m= sendto (ds_udp, buff_envio , ENVIO, 0, (struct sockaddr *) &cliente, long_cli);

					if(m == -1)	{ error("sendto");}
					if(m != ENVIO)	{printf("\nse han perdido caracteres\n");}
				//recibo ack(puerto), o nack (error); 
				n= recvfrom(ds_udp, buff, ENVIO, 0, (struct sockaddr *) &servidor, &long_serv);

					if(n == -1)	{error("recvfrom");}
				deserializar(buff,&s_paquete);
				if((int)s_paquete.cmdType==CMD_NACK)
				{		desempaquetar(&s_paquete,&s_respuesta);	printf("NACK\n");
						printf("error: %s\n",s_respuesta.info);
				}
				else//->informacion = puerto + tamaño bloque + tamaño del archivo;
				{//realizar conexion tcp, dividir y hacer un for para llamar a recibir fichero

					desempaquetar(&s_paquete,&s_respuesta);	printf("ACK\n");
					printf("info: %s\n",s_respuesta.info);
					//saco el puerto y el tamaño de bloque de la informacion
					liberaItems (items);
					num = separaItems (s_respuesta.info, &items, &background);
					puerto_tcp=atoi(items[0]); //puerto
					tam_bloque=atoi(items[1]); //tamaño de bloque

					//inicializo la estructura destino:
					bzero( (char*) & destino_tcp, sizeof (destino_tcp));   //inicializo a cero la estructura
					destino_tcp.sin_family = AF_INET; 			//familia	
					destino_tcp.sin_port = htons (puerto_tcp); 	//formato red 	
					destino_tcp.sin_addr.s_addr = inet_addr(dirIP); 	// IP de servidor
					long_desttcp = sizeof(destino_tcp);
					if( (ds_tcp = socket(AF_INET, SOCK_STREAM, 0) ) == -1)
					{	error("socket");	}

					if( connect(ds_tcp, (struct sockaddr *) &destino_tcp, long_desttcp) == -1)
					{	error("connect");		}
			
					//llamo a la funcion recibir fichero
					printf("Starting transfer: %s  ",s_write.fichero);

					iteracion=0;
					respu=enviar_fichero(ds_tcp,s_write,tam_bloque,iteracion,mq);

						if(respu==0)
						{	
							iteracion=0;
							printf("done\n");
							close(ds_tcp);
							break;
						}

		

				}




		break;

		}
		case(CMD_CLOSE):	//close()
		{			
				if(conexion==0){ printf("No tienes establecida una conexion\n"); break;}
				if(num>2){printf("Error de formato \n"); break;}
					//ack... restar contador, poner a 0 usu...

				s_paquete.cmdType=CMD_CLOSE;s_paquete.cmdlength=0;
				//empaquetar(&s_paquete);
				serializar(buff_envio,&s_paquete);
				m= sendto (ds_udp, buff_envio , ENVIO, 0, (struct sockaddr *) &cliente, long_cli);

					if(m == -1)	{ error("sendto");}
					if(m != ENVIO)	{printf("se han perdido caracteres\n");}
				n= recvfrom(ds_udp, buff, ENVIO, 0, (struct sockaddr *) &servidor, &long_serv);

					if(n == -1)	{error("recvfrom");}
					//printf("\nhe recibido: %s\n",buff);
					deserializar(buff,&s_paquete);
					//analizar paquete:					
				if((int)s_paquete.cmdType==CMD_ACK)
				{ printf("ACK\n");close_con=1;conexion=0;}
										
			break;				
		}
				

		
		case(CMD_EXIT):	//EXIT
		{
			//printf("salimos del programa cliente\n");
			//enviar close al servidor
			if(num>2){printf("Error de formato \n"); break;}
			if(conexion==0) exit(0);
			else
			{	if(close_con==1) exit(0);
				else {printf("Error: antes de salir ejecuta la orden close.\n");break;}
			}			
			close(ds_udp); //cierro el socket udp	
			exit(0);
			break;
		}

		default: break;
	

	}
	

liberaItems (items);
	
	}while(1);

free(buff);
free(buff_envio);

return(0);

}		
