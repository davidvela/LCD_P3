 /* 
# * Author: David Vela Tirado
# * Software Developer
# * email:  david.vela.tirado@gmail.com
# * 
  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>  //contiene informacion para sistemas unix 
#include <fcntl.h> //me permite poner en la funcion open el flag en vez de como un int como un nombre  ((( O_RDONLY)))
#include <sys/socket.h>
#include <sys/types.h>
#include "estructuras.h"
#include "funciones.h"
#include <arpa/inet.h>
#include <sys/ioctl.h>

//#include "parser.h"

#define MAXBUFF 200 //longitud máxima 
#define MAXCONECT 5 //variable para escuchar solamente 5 peticiones (listen) valor recomendado
#define PORTSERV 10021


int main( int argc,char *argv[])		// ./servidor "puerto" "conexiones maximas"   puerto:10021
{
	if( argc!=3)
	{	printf("\nEl numero de argumentos es incorrecto.\n");	exit(0);}


int ds_udp; 				//descriptor del socket
int i,n,m,p;			//contadores
int conexiones=0;			//conexiones establecidads
int respu=0;				//respuesta de las comprobaciones --- 0->ack 1->nack
int long_serv, long_cli;		//LONGITUDES...
struct sockaddr_in servidor, cliente;  //info. servidor y cliente 
struct openCmd_t s_open;			//Orden HELLO
struct ackCmd_t s_respuesta;		//Orden ACK o NACK
struct listCmd_t s_list;			//Orden LIST
struct transferCmd_t s_read,s_write;	//Orden READ o WRITE
struct CmdPkt_t s_paquete;		//Paquete de órdenes
struct Clientes_serv **usuarios;
	usuarios=malloc(5*sizeof(struct Clientes_serv*));
	for(i=0;i<5;i++) usuarios[i]=malloc(sizeof(struct Clientes_serv));

char *buff ;
	buff = malloc ((ENVIO)*sizeof(char*)); 		 // Pedir array
char *buff_envio;						//tamaño maximo de la orden.
  		buff_envio = malloc ((ENVIO)*sizeof(char*));  // Pedir array

int numacs,puertocli,ipcli;
int serv_puerto=atoi(argv[1]);				//convierto de char a int		"10021"	
int conexiones_max=atoi(argv[2]);
int puerto_tcp;
char aux[10];	

//conexion tcp:

int ds_tcp,modo; 				//descriptor del socket
struct sockaddr_in servidor_tcp, cliente_tcp;  //info. servidor y cliente 
int long_servtcp, long_clitcp,listen_dev;	//LONGITUDES...

	//inicializo la estructura del servidor tcp
	long_clitcp=sizeof(cliente);
	bzero( (char*) &cliente_tcp, long_clitcp); 	//inicializo a cero la estructura	
	bzero( (char*) & servidor_tcp, sizeof (servidor_tcp));   //inicializo a cero la estructura
	servidor_tcp.sin_family = AF_INET; 			//familia	
	servidor_tcp.sin_addr.s_addr = INADDR_ANY; 		//cualquier IP
	//servidor_tcp.sin_port =0; 		//el puerto lo asigna el so (el primero libre)
	long_servtcp = sizeof(servidor_tcp);


	//inicializo la estructura del servidor udp
	bzero( (char*) & servidor, sizeof (servidor));   //inicializo a cero la estructura
	servidor.sin_family = AF_INET; 			//familia	
	servidor.sin_addr.s_addr = INADDR_ANY; 		//cualquier IP
	servidor.sin_port = htons (serv_puerto); 	//formato red 	10021
	long_serv = sizeof(servidor);
	long_cli=sizeof(cliente);
	bzero( (char*) &cliente, long_cli); 	//inicializo a cero la estructura	

				//UDP
	
	//1 abrir socket (canal de escucha)		
	if( (ds_udp = socket(AF_INET, SOCK_DGRAM, 0) ) == -1)
	{	error("socket de escucha udp "); exit(1);	}
	//2 asociar el canal y el puerto
	if( bind(ds_udp, (struct sockaddr *) &servidor, long_serv) == -1)
	{ error("bind"); exit(1);  }

				//TCP

	//3 abrir socket escucha 			
	if( (ds_tcp = socket(AF_INET, SOCK_STREAM, 0) ) == -1)
	{	error("socket_tcp");exit(1);	}
	//4 asociar el canal y el puerto
	if( bind(ds_tcp, (struct sockaddr *) &servidor_tcp, long_servtcp) == -1)
	{	error("bind");	exit(1);	}
	
	//para obtener el puerto asignador por el s.o 	
	if( getsockname(ds_tcp,(struct sockaddr *) &servidor, &long_servtcp) == -1)
	{perror("getsockname"); exit(1);}

	puerto_tcp=ntohs(servidor.sin_port);
	printf("puerto= %d\n",puerto_tcp);
	//5 ESCUCHA
	listen_dev=listen(ds_tcp,MAXCONECT);
	if(listen_dev==-1){	error("listen");exit(1);	}
	printf("LISTEN OK\n");
	

	
	
	
//Leo los clientes registrados: usuario,   contraseña, tamaño de bloque
		numacs=lectura_acceso(usuarios);
		printf("\nClient\tRoute\tFile\t\tProgress");
		printf("\n------\t-----\t----\t\t--------\n");

//fcntl(ds_udp,F_SETFL,O_NONBLOCK);	//para que recvfrom no sea bloqueante
modo=0; // Modo  bloqueante de funcionamiento del socket 
//modo=1; // Modo NO  bloqueante de funcionamiento del socket 
	if (ioctl(ds_udp,FIONBIO,&modo)!=0)
	{
	     perror("ioctl");
	     exit(0);
	}


//PROCESO
for(;;)		
{	
		//limpiar buffer y paquete:
			respu=24;
			for(i=0; i<MAX_CMD; i++) {buff[i]=0;buff_envio[i]=0;}
			s_paquete.cmdType=0;
			

		//3 recibir puerto UDP
	 		n = recvfrom(ds_udp, buff, ENVIO, 0, (struct sockaddr *)&cliente, &long_cli);

			if (n < 0) { error ("recvfrom");   exit (1);	}
				puertocli=cliente.sin_port;
				ipcli=cliente.sin_addr.s_addr;
			//printf("\nhe recibido: %s. del cliente: puerto=%d ip=%d \n",buff,cliente.sin_port,cliente.sin_addr.s_addr);
			deserializar(buff,&s_paquete);

			//IDENTIFICAR EL CLIENTE QUE ME HA MANDADO ESTE PAQUETE.**********************(varios clientes)
			//lograr que recvfrom sea una funcion no bloqueante (varios clientes)
			switch(s_paquete.cmdType)
			{
				case (CMD_OPEN):  	//open usuario password	
				{
					if(conexiones==conexiones_max)
					{ 
					strcpy(s_respuesta.info,"No se ha podido establecer dado que el servidor ha alcanzado su 						numero máximo de conexiones");s_respuesta.inReplyTo=CMD_OPEN;
					respu=-1;
					break;	
					}					
					//lamar a funcion desempaquetar open, comprobar usu y key, numero max
					//despues->ack o nack(error) -> empaquetar -> serializar-> [break]->enviar.
 					desempaquetar(&s_paquete,&s_open);
			 		respu=comprobacion_usu(s_open,usuarios,&s_respuesta,numacs,puertocli,ipcli);
					 //0->ack, 1->nack	
					//empaquetar, serializar y enviar se hacer fuera del switch	
					if(respu==0) conexiones++;
					s_respuesta.inReplyTo=CMD_OPEN;			
					//printf("\nrespu = %d ", respu);
					break;
				}
	 	
				case(CMD_LIST):	//list ruta
				{		
					//printf("funcion list\n");
					//desempaquetar list, comprobar ruta: directorio y existencia
					desempaquetar(&s_paquete,&s_list);
					s_respuesta.inReplyTo=CMD_LIST;
					respu= comprobacion_ruta(&s_list,&s_respuesta);	
					//despues-> ack(contenido) o nack (error)->empaquetar.... 
					break;
				}   
				
				case(CMD_READ):	//read(ruta, fichero)
				{	//desempaquetar read, comprobar ruta, fichero...
				
					p=identificacion_usu(puertocli,ipcli,usuarios);
					//printf("identificacion= %d\n",p);
					if(p==-1){strcpy(s_respuesta.info,"ya tienes una transferencia activa"); respu=-1; 						break;}
					desempaquetar( &s_paquete,&s_read);

					s_respuesta.inReplyTo=CMD_READ;
					respu=comprobacion_transfer(&s_read,&s_respuesta);	
					//ack(puerto) averiguarlo, o nack (error)->empaquetar.... abrir tcp
					//printf("respu= %d\n",respu);
					if(respu==0)//asignar puerto enviar puerto,enviar tamaño de bloque
					{
												
						sprintf(aux,"%d",(int)puerto_tcp);
						strcpy(s_respuesta.info,aux);
						strcat(s_respuesta.info," ");
						sprintf(aux,"%d",(int)usuarios[p]->tam_bloque);
						strcat(s_respuesta.info,aux);//tamaño de bloque;

						usuarios[p]->ite=0;	//1iteracion de 1envio-> aceptar conexion
						usuarios[p]->transfer=1; //read
						usuarios[p]->transferencia=s_read; //ruta y fichero.
									
					}
					break;

				}
				case(CMD_WRITE):	//write(ruta, fichero)
				{			//desempaquetar write, comprobar ruta, fichero...
					
					p=identificacion_usu(puertocli,ipcli,usuarios);
					//printf("identificacion= %d\n",p);
					if(p==-1){strcpy(s_respuesta.info,"ya tienes una transferencia activa"); respu=-1; break;}
					desempaquetar( &s_paquete,&s_write);
						s_respuesta.inReplyTo=CMD_WRITE;
						respu=0;

						sprintf(aux,"%d",(int)puerto_tcp);
						strcpy(s_respuesta.info,aux);
						strcat(s_respuesta.info," ");
						sprintf(aux,"%d",(int)usuarios[p]->tam_bloque);
						strcat(s_respuesta.info,aux);//tamaño de bloque;
						
						usuarios[p]->ite=0;	//1iteracion de 1envio-> aceptar conexion
						usuarios[p]->transfer=2; //write
						usuarios[p]->transferencia=s_write; //ruta y fichero.
									
					break;
				}

				case(CMD_CLOSE):	//close()
				{
					//ack... restar contador, poner a 0 usu...
					p=identificacion_usu(puertocli,ipcli,usuarios);
					usuarios[p]->activo=0;
					respu=0;
					strcpy(s_respuesta.info," ");
					conexiones--;
					s_respuesta.inReplyTo=CMD_CLOSE;
					break;				
				}
				
				default: break;
			}//fin switch.
	
			if(respu!=24)	//significa que no ha realizado ninguna funcion y no tengo que enviar nada
			{
				if(respu==0)
				{	s_paquete.cmdType=CMD_ACK; 
					empaquetar(&s_paquete,&s_respuesta);
				}

				else if(respu==-1)
				{	s_paquete.cmdType=CMD_NACK; 
					empaquetar(&s_paquete,&s_respuesta);
				}

				serializar(buff_envio,&s_paquete);
				m= sendto (ds_udp, buff_envio , ENVIO, 0, (struct sockaddr *) &cliente, long_cli);

					if(m == -1)	{ error("sendto");}
					if(m != ENVIO)	{printf("\nse han perdido caracteres\n");}
				//printf("\nLe he enviado esta cadena al cliente: %s\n",buff_envio);
			
				if( (   (s_respuesta.inReplyTo==CMD_READ)||(s_respuesta.inReplyTo==CMD_WRITE) ) &&(respu==0) )
				{
					if( (usuarios[p]->ds_tcp=accept(ds_tcp,(struct sockaddr *) &cliente_tcp, &long_clitcp)) == -1)
						{	error("accept");	}
						//usuarios[p]->ds_tcp= newds;
				}
			}
			//en caso de read o write hacer gestion tcp.
			respu=gestion_tcp(usuarios);

}		
	free (buff);
	free(buff_envio);	
	close(ds_udp); //cierro el socket de escucha.
	close(ds_tcp);	//tcp listen
	for(i=0; i<5; i++) free(usuarios[i]);
	free(usuarios);
return 0;
}
		
		





