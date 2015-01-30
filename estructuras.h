 /* 
# * Author: David Vela Tirado
# * Software Developer
# * email:  david.vela.tirado@gmail.com
# * 
  */

#include <stdio.h>

#if !defined(_ESTRUCTURAS_H)
#define _ESTRUCTURAS_H


#define MAX_USERNAME 20
#define MAX_PASSWD 20
#define MAX_INFO 100
#define MAX_RUTA 100
#define MAX_FICHERO 20
#define MAX_CMD 150
#define ENVIO 158

typedef enum
	{
		CMD_OPEN = 1,
		CMD_LIST = 2,
		CMD_READ = 3,
		CMD_WRITE = 4,
		CMD_CLOSE = 5,
		CMD_ACK = 6,
		CMD_NACK =7,
		CMD_EXIT =8
	} FtpCmd;
typedef enum
	{
		MQ_CLIENTE  = 1,
		MQ_SERVIDOR = 2,
	} tipo_mq;	//tipo de maquina


	struct openCmd_t 			//Orden HELLO
	{
		char username[MAX_USERNAME];
		char password[MAX_PASSWD];
	};
	
	struct ackCmd_t 			//Orden ACK o NACK
	{
		FtpCmd inReplyTo;		//El tipo de paquete al que asentimos
		char info[MAX_INFO];		//Información complementaria (error, puerto... )
	};	
	
	struct listCmd_t		 	//Orden LIST
	{
		char ruta[MAX_RUTA];
	};

	struct transferCmd_t		 	//Orden READ o WRITE
	{
		char ruta[MAX_RUTA];
		char fichero[MAX_FICHERO];
	};

	
	struct CmdPkt_t			 	//Paquete de órdenes
	{
		FtpCmd cmdType;			//El tipo de orden que contiene el paquete		
		size_t cmdlength;		//Longitud de la estructura de orden que se envia
		char cmdStruct [MAX_CMD];	//Buffer que contiene la estructura
	};

#endif

