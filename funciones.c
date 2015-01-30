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
#include <sys/socket.h>
#include <sys/types.h>
#include "estructuras.h"
#include <unistd.h>
#include "funciones.h"
#include <fcntl.h> //me permite poner en la funcion open el flag en vez de como un int como un nombre  ((( O_RDONLY)))
#include <arpa/inet.h>
#include "/usr/include/sys/dir.h"
#include <sys/stat.h>
#include <limits.h>
#include <dirent.h>

#define MAXBUFF 200 //longitud máxima 
#define MAXCONECT 5




	void error (char * mensaje)
	{
		perror (mensaje);
		exit(1);
	}


void empaquetar(struct CmdPkt_t *pq, void *estructura)
{
	
	int tam=0;
	pq->cmdlength=tam;
	switch(pq->cmdType)
	{
	 case CMD_OPEN:
			tam=40; 	pq->cmdlength=tam; //username=MAX_USERNAME=20 + password=MAX_PASSWD=20
			if ( memcpy((char *)&pq->cmdStruct ,(struct openCmd_t *)estructura, pq->cmdlength ) == NULL)
			printf("\nError en empaquetado open");			
			break;
	 case CMD_LIST:
			tam=100; 	pq->cmdlength=tam; //ruta=MAX_RUTA=100
			if ( memcpy((char *)&pq->cmdStruct ,(struct listCmd_t *)estructura, pq->cmdlength ) == NULL)
			printf("\nError en empaquetado list");		
			break;

	 case CMD_READ:
			tam=120; 	pq->cmdlength=tam;//ruta=MAX_RUTA=100 + fichero=MAXFICHERO=20
			if ( memcpy((char *)&pq->cmdStruct ,(struct transferCmd_t *)estructura, pq->cmdlength ) == NULL)
			printf("\nError en empaquetado read");		
			break;

	 case CMD_WRITE:
			tam=120; 	pq->cmdlength=tam; //ruta=MAX_RUTA=100 + fichero=MAXFICHERO=20
			if ( memcpy((char *)&pq->cmdStruct ,(struct transferCmd_t *)estructura, pq->cmdlength ) == NULL)
			printf("\nError en empaquetado write");		
			break;
	case CMD_CLOSE:
			tam=0; 	pq->cmdlength=tam; //ruta=MAX_RUTA=100 + fichero=MAXFICHERO=20
			
			printf("\nError en empaquetado close");		
			break;
	 case CMD_ACK:
			tam=104; 	pq->cmdlength=tam; //ftpcmd=4 + info=MAX_INFO=100
			if ( memcpy((char *)&pq->cmdStruct ,(struct ackCmd_t *)estructura, pq->cmdlength ) == NULL)
			printf("\nError en empaquetado ack");	
		 	break;
	 case CMD_NACK:
			tam=104; 	pq->cmdlength=tam; //ftpcmd=4 + info=MAX_INFO=100
			if ( memcpy((char *)&pq->cmdStruct ,(struct ackCmd_t *)estructura, pq->cmdlength ) == NULL)
			printf("\nError en empaquetado nack");		
		 	break;
	 default:
		printf("\nError en empaquetado. No se ha definido correctamente\n");
	}
}



void desempaquetar(struct CmdPkt_t *pq, void *estructura)
{
	switch(pq->cmdType)
	{
	 case CMD_OPEN:
			if ( memcpy((struct openCmd_t *)estructura, (char *) &pq->cmdStruct, pq->cmdlength) == NULL)
			printf("\nError en desempaquetado open");		
			break;
	 case CMD_LIST:
			if ( memcpy((struct listCmd_t *)estructura, (char *) &pq->cmdStruct, pq->cmdlength) == NULL)
			printf("\nError en desempaquetado list");		
	 		break;

	 case CMD_READ: 
			if ( memcpy((struct transferCmd_t *)estructura, (char *) &pq->cmdStruct, pq->cmdlength) == NULL)
			printf("\nError en desempaquetado read");
		
			break;
	 case CMD_WRITE:
			if ( memcpy((struct transferCmd_t *)estructura, (char *) &pq->cmdStruct, pq->cmdlength) == NULL)
			printf("\nError en desempaquetado write");		
			break;
	case CMD_CLOSE:
			
			printf("\nError en desempaquetado close");		
			break;
	 case CMD_ACK:
			if ( memcpy((struct ackCmd_t *)estructura, (char *) &pq->cmdStruct, pq->cmdlength) == NULL)
			printf("\nError en desempaquetado ack");		
		 	break;
	 case CMD_NACK:
			if ( memcpy((struct ackCmd_t *)estructura, (char *) &pq->cmdStruct, pq->cmdlength) == NULL)
			printf("\nError en desempaquetado nack");	
		 	break;
	 default:
		printf("Error en desempaquetado. No se ha definido correctamete\n");
	}
}



void deserializar(char * buff,struct CmdPkt_t *pq )//objetivo obtener pq
{
	int tam= ENVIO ;	//int tam= pq->cmdlength+8 ;
	if ( memcpy((struct CmdPkt *) pq, (char *) buff,tam) == NULL)
	printf("\nError en deserializado");		
		

}

void serializar (char * buff,struct CmdPkt_t *pq) //objetivo obtener buff
{	
	int tam= ENVIO ;	//int tam= pq->cmdlength+8 ;
	if ( memcpy((char *)buff ,(struct CmdPkt *)pq, tam) == NULL)
		printf("\nError en empaquetado");			
	
}	





/*
    parser.c
*/



int separaItems (char * expresion,   // Palabras a separar
                 char *** items,     // Resultado
                 int * background)   // 1 si hay un & al final
{
  int i, j, num, ult;
  char ** pp;
                    // En principio:
  *items = NULL;    //   cero parametros
  *background = 0;  //   ejecucion en primer plano

  for (i=0; expresion[i]!='\0'; i++)  // Cambiar saltos de
    if (expresion[i]=='\n' ||         // linea y tabuladores
        expresion[i]=='\t')           // por espacios
      expresion[i] = ' ';

  while (*expresion==' ')   // Quitar espacios del principio
    expresion ++;

  if (*expresion=='\0')     // Si cadena vacia ...
    return 0;               // ... cero parametros

  for (i=1, num=1, ult=0; expresion[i]!='\0'; i++)
    if (expresion[i]!=' ' &&
        expresion[i-1]==' ')            // Contar palabras
    {                                   // (parametros)
      num ++;
      ult = i;     // Recordar posicion de la ultima palabra
    }

  i --;
  while (i>=0 && expresion[i]==' ')   // Quitar espacios
    expresion[i--] = '\0';            // del final

                                      // Mirar si la ultima
  if (strcmp(expresion+ult,"&")==0)   // palabra es un &
  {
    *background = 1;  // Marcar ejecucion en segundo plano

    expresion[ult] = ' ';               // Quitar &

    while (i>=0 && expresion[i]==' ')   // Quitar espacios
      expresion[i--] = '\0';            // del final
                                        // (otra vez)
    num --;           // Descontar &

    if (*expresion=='\0')  // Si solo estaba el & ...
      return 0;            // ... cero parametros
  }

  pp = malloc ((num+1)*sizeof(char*));  // Pedir array
  if (pp==NULL) return -1;              // de punteros

  pp[0] = expresion;  // El primer parametro es facil

  for (i=1, j=1; expresion[i]!='\0'; i++)  // Localizar los
    if (expresion[i]!=' ' &&             // demas parametros,
        expresion[i-1]==' ')           // apuntar a ellos con
    {                                // los punteros del
      expresion[i-1] = '\0';       // array, y poner \0 en
      pp[j++] = expresion + i;   // lugar de espacios
    }
                  // Anyadir un puntero a NULL al final del
  pp[j] = NULL;   // array de punteros (asi es mas facil
                  // trabajar con el)
  *items = pp;
  return num;     // Devolver array y numero de palabras
}

void liberaItems (char ** items)
{
  free (items);
}


int lectura_acceso(struct Clientes_serv **usuarios)	//devuelve el numero de usuarios registrados en el fichero acceso.txt
{			
			int fd,p,i,numitems,background,rep;
			char ** items;
			char acceso[MAXBUFF]; 
			int longn,longk;
			int tama;
	
			for(i=0;i<5;i++) usuarios[i]=malloc(sizeof(struct Clientes_serv));

			fd=open("accesos.txt",O_RDONLY);
			if (fd==-1) 	//compruebo quela apertura es correcta
			{ printf("error de apertura de fichero"); exit(0);}

			p=read(fd,acceso,MAXBUFF);
			numitems=separaItems (acceso, &items, &background);	
				//printf("\n numero de items = %d \n\n",numitems);
				//printf("\n he leido    %s Y %d bits\n",acceso,p);	
			p=0;
			rep=numitems/3;
			for(i=0; i<rep;i++)
			{	p=i*3;
				strcpy(usuarios[i]->usu.username,items[p + 0]);
				strcpy(usuarios[i]->usu.password,items[p+1]);
				usuarios[i]->tam_bloque=atoi(items[p+2]);	
				usuarios[i]->activo=0; usuarios[i]->ip=0; usuarios[i]->transfer=0;
				usuarios[i]->ite=0;		

			//printf("usuario%d: \n",i);
			longn=strlen(usuarios[i]->usu.username);longk=strlen(usuarios[i]->usu.password);
			tama=sizeof(usuarios[i]->tam_bloque);
			//printf("nombre: %s de %d cara \n clave: %s de %d cara \n tam bloque: %d de %d (tam) \n",
			//usuarios[i]->usu.username,longn,usuarios[i]->usu.password,longk,usuarios[i]->tam_bloque,tama);
			
			}
			liberaItems (items);
			return rep;
}

int comprobacion_usu(struct openCmd_t open,struct Clientes_serv **usu, struct ackCmd_t *respuesta,int numacs,int puerto,int ip)	
//0->ack, -1->nack
{	
	int i,boolusu,boolkey; 	//boolusu->0 no usuario 1->si usuario
	i=boolusu=boolkey=0;
	respuesta->inReplyTo=1;

	//printf("longitud usuario: %d\n", k);	
	//printf("longitud contraseña: %d\n", strlen(open.password));

	for(i=0;i<numacs;i++)
		{	//printf("comparo:%s y%s \n",usu[i]->usu.username,open.username);
			if( !strcmp(open.username,usu[i]->usu.username) ) {boolusu=1;break;}
			//if( !strcmp("david",usu[i]->usu.username) ) {boolusu=1;break;}

		}
	if(boolusu==1)
	{	//printf("comparo: %s y %s \n",usu[i]->usu.password,open.password);
		if( (strcmp(usu[i]->usu.password,open.password))==0) {
		boolkey=1; 
		if(usu[i]->activo==1){strcpy(respuesta->info,"Este usuario esta activo\n"); return -1; }
		else usu[i]->activo=1;
		usu[i]->puerto=puerto;
		usu[i]->ip=ip;
		}
	}
		

	if((boolusu==1)&&(boolkey==0))
		// {strcpy(respuesta->info,"usuario correcto pero contraseña incorrecta"); return -1;}
		 {strcpy(respuesta->info,"username correct but password isn't correct"); return -1;}
	if((boolusu+boolkey)==0) 
		//{ strcpy(respuesta->info,"usuario incorrecto"); return -1; }
		{ strcpy(respuesta->info,"username isn't correct"); return -1; }
	if((boolusu+boolkey)==2) 
		//{strcpy(respuesta->info,"usuario y contraseña correctas"); return 0;}
		{strcpy(respuesta->info,"username and password correct"); return 0;}
	return -1;	
}

int comprobacion_ruta(struct listCmd_t *list, struct ackCmd_t *respuesta)
{	//0->ack, -1->nack
	struct dirent **estru_dirent;
	struct stat estru_dir;
	char archivos[100];
	int i, entradas=0;


	if(!strcmp(list->ruta,".")){ strcpy(respuesta->info,"directorio no valido");return -1;}
	//printf("ruta=%s \n", list->ruta);
	if( stat( list->ruta, &estru_dir)) //obtenemos estructura stat de la ruta especificada
	{ perror("stat: ");strcpy(respuesta->info,"Route not exist");return -1;}	//ruta especificada erronea

	if (S_ISDIR(estru_dir.st_mode) )//comprobamos si es directorio
	{ 	entradas = scandir(list->ruta, &estru_dirent, 0, alphasort);			
		if( entradas == -1)
		{ perror("scandir: "); }
		else
		{	
		
			//printf("\n nºentradas: %d\n", entradas);
			if(entradas==2) { strcpy(respuesta->info,"directorio vacio"); return 0;}
			strcpy(archivos, estru_dirent[2]->d_name);	
			for (i=3; i < entradas; i++)//la 1 y la 2 entrada son . y ..
			{
					//printf("\n nºentradas[%d] = %s\n",i,estru_dirent[i]->d_name);
					strcat(archivos, " " );
					strcat(archivos, estru_dirent[i]->d_name);						
			}
	
		strcpy(respuesta->info,archivos);
		return 0;//todo correcto
		}
	}
	else {strcpy(respuesta->info,"Not directory");return -1;}
	return -1; //ha habido algun fallo
}

int comprobacion_transfer(struct transferCmd_t *trans, struct ackCmd_t *respuesta)
{
	int i,numitems,background,res,k;
	char ** items;
	struct listCmd_t lista;
	k=strlen(trans->ruta);
	strcpy(lista.ruta,trans->ruta);

	res=comprobacion_ruta(&lista,respuesta);
	if(res!=0) return -1;

	if(respuesta->inReplyTo==CMD_WRITE) return 0;

	numitems=separaItems (respuesta->info, &items, &background);
	for(i=0; i<numitems;i++)
	{
		if( !strcmp(trans->fichero,items[i])) {liberaItems (items);return 0;} //existe el fichero
	}	
	strcpy(respuesta->info,"File not found"); //El fichero no existe en la ruta introducida
	liberaItems (items);
	return -1;

}

int identificacion_usu( int puerto, int ip, struct Clientes_serv **usu) //debuelve el indice en la matric usu y -1 si erro o no esta
{
	int i=0;
	for(i=0; i<5;i++)
	{	if(usu[i]->activo==1)
		{
			//printf("puerto cli =%d puertousu= %d ipcli=%d ipusu=%d\n",puerto,usu[i]->puerto,ip,usu[i]->ip);
			if((usu[i]->puerto==puerto)&&(usu[i]->ip==ip)&&(usu[i]->activo==1))
			{
				if(usu[i]->transfer!=0) return -1; //tiene una transferencia activa
				return i;
			}
		}	
	}
	return -1;

}

//si llego al final de fichero devuelvo un 0 para saber que he parado.

int enviar_fichero(int ds_tcp,struct transferCmd_t transfer, int tam_bloque, int itera, tipo_mq maq  )
{
	int m,fd,dev,p,i;
	char men_env[tam_bloque];
	char path[MAX_RUTA];
	int posicion;

	strcpy(path,transfer.ruta);
	strcat(path,"/");
	strcat(path,transfer.fichero);
	itera=0;posicion=0;p=0;dev=24;
	
		//abro fichero de disco
		fd=open(path,O_RDONLY);
		if (fd==-1) 	//compruebo quela apertura es correcta
		{ printf("error de apertura de fichero (enviar fichero)\n"); exit(0);}

		for(;;)
		{
		//leo fichero en la posicion indicada por iteracion
		//posicion=tam_bloque*itera;
		posicion=posicion+p;	
		lseek(fd,posicion,SEEK_SET);
		p=read(fd,men_env,tam_bloque);
		if(p==0 )dev=0;	
		if(p==-1 )printf("Error en la lectura del fichero\n");	
		if(p<tam_bloque) dev=0;
		//printf("he leido del fichero %d bytes y %s y el tamaño de bloque es %d \n",p,men_env,tam_bloque);
		//escribo en el puerto tcp
		m= write (ds_tcp, men_env,p);
		if(m == -1)
		{	error("write");	}
		if(m != p)
		{	printf("\nse han perdido caracteres\n");	}
		//printf("\nLe he enviado esta cadena: %s\n",men_env);
		for(i=0;i<tam_bloque;i++) men_env[i]=0;
		itera++;
	
		if(maq == MQ_CLIENTE ) printf("#");else printf("X");

		if(dev==0) break;//he llegado al final
		}	
		close(fd);
	


		
		
return dev;	
}

int recibir_fichero(int ds_tcp,struct transferCmd_t transfer, int tam_bloque, int itera, tipo_mq maq )
{
	int n,fd,dev,i,m;
	char men_rec[tam_bloque];
	char path[MAX_RUTA];
	int posicion; int num; 
	char aux[10];

	struct ackCmd_t s_respuesta;
	s_respuesta.inReplyTo=CMD_READ;
	itera=0;
	posicion=0;
	n=0;
	for(i=0;i<tam_bloque;i++) men_rec[i]=0;
	dev=24;


		if(maq== MQ_CLIENTE)
		{
			strcpy(transfer.ruta,"cliente_descargas");
			m=comprobacion_transfer(&transfer,&s_respuesta);
			if(m==0) {	
			printf("\nEl fichero que intentas recibir ya existe en la carpeta de descargas.\n");num=rand(); 			sprintf(aux,"%d",num); strcpy(transfer.fichero,aux);
				}
	
			strcpy(path,transfer.ruta);
			strcat(path,"/");
			strcat(path,transfer.fichero);	
			fd=open(path,O_CREAT|O_WRONLY|O_TRUNC,S_IRWXU|S_IRWXG|S_IRWXO);
			if (fd==-1) 	//compruebo quela apertura es correcta
			{ printf("error de apertura de fichero1\n"); exit(0);}
		}

		if(maq == MQ_SERVIDOR)
		{	
			strcpy(transfer.ruta,"servidor_descargas");
			m=comprobacion_transfer(&transfer,&s_respuesta);
			if(m==0) {	
			printf("\nEl fichero que intentas recibir ya existe en la carpeta de descargas.\n");
			num=rand(); 	sprintf(aux,"%d",num); strcpy(transfer.fichero,aux);

			}
			strcpy(path,transfer.ruta);
			strcat(path,"/");
			strcat(path,transfer.fichero);	
			fd=open(path,O_CREAT|O_WRONLY|O_TRUNC,S_IRWXU|S_IRWXG|S_IRWXO);
			if (fd==-1) 	//compruebo quela apertura es correcta
			{ printf("error de apertura de fichero1\n"); exit(0);}


		}

		
		for(;;)
		{
		posicion=posicion+n;	
		//recibo informacion desde el puerto tcp
		n= read(ds_tcp, men_rec, tam_bloque);
		if(n == -1)
		{ error("read");	}
		//printf("\nHe recibido: %s de %dbytes ", men_rec,n);
		if(n<tam_bloque) dev=0; //he terminado
		if(n==0) break;
		//escribo en fichero 
		//printf("posicion= %d y meto: %d bytes\n",posicion,n);

		lseek(fd,posicion,SEEK_SET);
		write(fd,men_rec,n);

		for(i=0;i<tam_bloque;i++) men_rec[i]=0;

		if(maq == MQ_CLIENTE ) printf("#");else printf("X");
		if(dev==0) break;
		}


	close(fd);
	return dev;
}


int gestion_tcp(struct Clientes_serv **usuarios)
{	
	int i=0;
	int respu=2;
	tipo_mq mq=MQ_SERVIDOR; 
	
for(i=0;i<5;i++)
{
	if(usuarios[i]->activo==1)

	if (  (usuarios[i]->transfer==1)||(usuarios[i]->transfer==2))
	{	
		if(usuarios[i]->transfer==1)//read
				{	
					printf("%s >> /%s/\t%s\t",usuarios[i]->usu.username,
					usuarios[i]->transferencia.ruta,usuarios[i]->transferencia.fichero);
		respu=enviar_fichero(usuarios[i]->ds_tcp,usuarios[i]->transferencia,usuarios[i]->tam_bloque,usuarios[i]->ite,mq); 
					printf("\n");

		}	

		else		//write
		{	
			printf("%s << /%s/\t%s\t",usuarios[i]->usu.username,
			usuarios[i]->transferencia.ruta,usuarios[i]->transferencia.fichero);	
		respu=recibir_fichero(usuarios[i]->ds_tcp,usuarios[i]->transferencia,usuarios[i]->tam_bloque,usuarios[i]->ite,mq);
			printf("\n");
		}
	}
	if(respu==0)
	{
		usuarios[i]->transfer=0; //fin
		close(usuarios[i]->ds_tcp);


	}

}


return 0;


}


