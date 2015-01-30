 /* 
# * Author: David Vela Tirado
# * Software Developer
# * email:  david.vela.tirado@gmail.com
# * 
  */
#include <stdio.h>
#include "estructuras.h"
#if !defined(_FUNCIONES_H)
#define _FUNCIONES_H



	struct Clientes_serv
	{
		struct openCmd_t usu;
		struct transferCmd_t transferencia; //contiene la ruta y el fichero a trasmitir
		int tam_bloque;
		int activo;		//0 no activado 1 activado
		int puerto;
		int ip;
		int transfer; 	//0 ninguna 1 read 2 write
		int ite;		//iteraciones de la transferencia
		int ds_tcp;
		int puertomio_tcp;
		//int descrip;

	};


//0->error
int separaItems (char * expresion,   // Palabras a separar
                 char *** items,     // Resultado
                 int * background);  // 1 si hay un & al final

void liberaItems (char ** items);
void error (char * mensaje);


void empaquetar(struct CmdPkt_t *pq, void *estructura);
void desempaquetar(struct CmdPkt_t *pq, void *estructura);
void serializar (char * buff,struct CmdPkt_t *pq); //objetivo obtener buff
void deserializar(char * buff,struct CmdPkt_t *pq );	//objetivo obtener pq


int comprobacion_usu(struct openCmd_t open,struct Clientes_serv **usu, struct ackCmd_t *respuesta,int numacs,int puerto,int ip);//0->ack, 1->nack
int lectura_acceso(struct Clientes_serv **usuarios);
int comprobacion_ruta(struct listCmd_t *list, struct ackCmd_t *respuesta);
int comprobacion_transfer(struct transferCmd_t *trans, struct ackCmd_t *respuesta);	 

int identificacion_usu( int puerto, int ip, struct Clientes_serv **usu); //debuelve el indice en la matric usu y -1 si erro o no esta

//si llego al final de fichero devuelvo un 0 para saber que he parado.
int recibir_fichero(int ds_tcp,struct transferCmd_t transfer, int tam_bloque, int itera, tipo_mq maq  );
int enviar_fichero( int ds_tcp,struct transferCmd_t transfer, int tam_bloque, int itera, tipo_mq maq  );
int gestion_tcp(struct Clientes_serv **usuarios); 

#endif

