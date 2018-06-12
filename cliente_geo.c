#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAX_BUF 1024
#define PORT 50005

//ARRAY DE STRINGS CON LOS COMANDOS
char * CMD[]={"SAV", "LST", "LSL", "RST"}; //Si hay un .h esto no puede ir aqui, y hay que hacer un include.
float lat, lon; //Latitud y longitud, revisar tipo.
int id;

//+++++++++FUNCIONES AUXILIARES+++++++++

void imprimirMenu(){
	printf("  Bienvenido al menu de su dispositivo \n\n");
	printf("****************************************\n");
	printf("Seleccione la opcion que quiere utilizar:\n");
	printf("\t\t\t\t-1) Guardar la localizacion actual. \n");
	printf("\t\t\t\t-2) Mostrar las ultimas localizaciones. (Maximo 10) \n");
	printf("\t\t\t\t-3) Solicitar la ultima posicion almacenada.\n");
	printf("\t\t\t\t-4) Resetear la lista de posiciones almacenadas. \n");
	printf("\t\t\t\t-5) Salir de la aplicacion. \n");
	printf("\n");
	printf("\t\t\t\tElija una opcion:\n ");
}

void generarId(){
	//id= rand() %1000;
	id= 373;
}

void generarLongLat(){
	lon= ((float)rand()/(float)(RAND_MAX)) * 90;
	lat= ((float)rand()/(float)(RAND_MAX)) * 90;
	//Estamos modificando una variable global.
}

int cuantos(){ //Se trata de una funcion para obtener un numero por teclado comprendido entre 1 y 10.
	int num = 0;
	char buffer[128];
	printf("Cuantas localizaciones desea mostrar? El maximo es de 10.\n");
	fgets(buffer,strlen(buffer),stdin);
	num = atoi(buffer); 
	return ( num > 10 || num < 1 ) ? 0 : num;//Esto es una expresion regular.
}


//+++++++++PRINCIPAL+++++++++
int main(int argc, char *argv[])
{
	int sock, n, numero;
	char buf[MAX_BUF];
	char seleccion[MAX_BUF];
	char variable[4]; //Todos los comandos tienen 3 letras + el '/0' que incluye C.
	struct sockaddr_in dir_serv;
	struct hostent *hp;
	socklen_t tam_dir;
	
	//Comprobamos que el usuario haya llamado bien a nuestro programa.
	if(argc != 2)
	{
		fprintf(stderr, "Uso: %s <NombreServidor | DireccionIPv4>\n", argv[0]);
		exit(1);
	}
	
	//Creamos un socket para UDP (SOCK_DGRAM).
	if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("Error al crear el socket");
		exit(1);
	}
	
	//Inicializaciones necesarias.
	memset(&dir_serv, 0, sizeof(dir_serv)); //Reservamos memoria para almacenar la direccion del servidor.
	dir_serv.sin_family = AF_INET;
	dir_serv.sin_port = htons(PORT);
	if((hp = gethostbyname(argv[1])) == NULL)
	{
		herror("Error al resolver el nombre del servidor");
		exit(1);
	}
	memcpy(&dir_serv.sin_addr, hp->h_addr, hp->h_length);

	//Llamadas a algunas funciones.
	
	generarId();
	imprimirMenu();
	
	
	//Bucle principal. Aqui se desarrollara el programa
	while(fgets(seleccion, MAX_BUF, stdin) != NULL)
	{
		//Comprobaciones, que comando se ha seleccionado? Que parametros tengo que solicitar?
		if (strcmp(seleccion,"1\n")==0){
		 	strcpy(variable,CMD[0]);
			printf("Esta es la seleccion: %s\n", variable);
			//generarLongLat();
			//Prefiero asignar valores fijos para las pruebas.
			lat=24.67;
			lon=31.94;
			//Probamos que tengamos valores significativos.
			printf("El comando es %s con latitud %f y longitud %f. El ID es %d \n", variable, lat, lon, id);
			//Usamos sprintf();	
			if(sprintf(buf, "%s %f %f %d", variable, lat, lon, id)<0){ //n es la longitud.
				printf("Error al generar el comando\n");
			}
			//Imprimo buf, porque quiero comprobar que se envia bien.
			printf("%s\n", buf);
			if(sendto(sock, buf, strlen(buf), 0, (struct sockaddr *) &dir_serv, sizeof(dir_serv)) < 0)
			{
				perror("Error al enviar el mensaje");
				exit(1);
			}
			//Me tengo que quedar a la espera de un mensaje, OK o ER.
			tam_dir = sizeof(dir_serv);
			if((n = recvfrom(sock, buf, MAX_BUF,0, (struct sockaddr *) &dir_serv, &tam_dir)) < 0)
			{
				perror("Error al recibir la respuesta");
				exit(1);
			}
			//Compruebo la respuesta obtenida.
			buf[n] = '\0';
			printf("%s",buf);
			if(strcmp(buf, "ER\n")==0){
				printf("Ha habido un error.\n");
			}else if(strcmp(buf, "OK\n")==0){
				printf("Operacion realizada correctamente\n");
			}else{
				printf("No se ha obtenido una respuesta del servidor\n");
			}
			memset(buf,0,strlen(buf));		
		}
		else if (strcmp(seleccion,"2\n")==0){
			strcpy(variable,CMD[1]);
			numero=0;
			while(numero==0){//Evita que el user meta 0.
				numero=cuantos();
			}
			if(sprintf(buf, "%s %d %d", variable, id, numero)<0){
				printf("Error al generar el comando");
			}
			//Envio el mensaje
			if(sendto(sock, buf, strlen(buf), 0, (struct sockaddr *) &dir_serv, sizeof(dir_serv)) < 0)
			{
				perror("Error al enviar el mensaje");
				exit(1);
			}
			//Recepción:
			int i = 0;

			for(i=0; i<numero; i++){
				tam_dir = sizeof(dir_serv);
				if((n = recvfrom(sock, buf, MAX_BUF,0, (struct sockaddr *) &dir_serv, &tam_dir)) < 0)
				{
					perror("Error al recibir la respuesta");
					exit(1);
				}
				buf[n] = '\0';
				printf("%s",buf);
				memset(buf,0,strlen(buf));//Vacio el bufer a la espera del siguiente.
			}
			//Espero el ultimo mensaje, que es OK o ER.
			tam_dir = sizeof(dir_serv);
			if((n = recvfrom(sock, buf, MAX_BUF,0, (struct sockaddr *) &dir_serv, &tam_dir)) < 0)
			{
				perror("Error al recibir la respuesta");
				exit(1);
			}
			//Compruebo la respuesta obtenida.
			buf[n] = '\0';
			printf("%s",buf);
			if(strcmp(buf, "ER\n")==0){
				printf("Ha habido un error.\n");
			}else if(strcmp(buf, "OK\n")==0){
				printf("Operacion realizada correctamente\n");
			}else{
				printf("No se ha obtenido una respuesta del servidor\n");
			}
			memset(buf,0,strlen(buf));			
			//LST + ID + N
		}else if (strcmp(seleccion,"3\n")==0){
			strcpy(variable,CMD[2]);
			if(sprintf(buf, "%s %d", variable, id)<0){
				printf("Error al generar el comando");
			}
			if(sendto(sock, buf, strlen(buf), 0, (struct sockaddr *) &dir_serv, sizeof(dir_serv)) < 0)
			{
				perror("Error al enviar el mensaje");
				exit(1);
			}
			//Espero la ultima localizacion
			tam_dir = sizeof(dir_serv);
			if((n = recvfrom(sock, buf, MAX_BUF,0, (struct sockaddr *) &dir_serv, &tam_dir)) < 0)
			{
				perror("Error al recibir la respuesta");
				exit(1);
			}
			//Compruebo la respuesta obtenida.
			//buf[n] = '\0';
			printf("%s",buf);
			memset(buf,0,strlen(buf));	//Limpio el buf.			
			//Me tengo que quedar a la espera de un mensaje, OK o ER.					
			tam_dir = sizeof(dir_serv);
			if((n = recvfrom(sock, buf, MAX_BUF,0, (struct sockaddr *) &dir_serv, &tam_dir)) < 0)
			{
				perror("Error al recibir la respuesta");
				exit(1);
			}
			//Compruebo la respuesta obtenida.
			//buf[n] = '\0';
			printf("%s",buf);
			
			if(strcmp(buf, "ER\n")==0){
				printf("Ha habido un error.\n");
			}else if(strcmp(buf, "OK\n")==0){
				printf("Operacion realizada correctamente\n");
			}else{
				printf("No se ha obtenido una respuesta del servidor\n");
			}
			memset(buf,0,strlen(buf));	
			//LSL + ID
		}else if (strcmp(seleccion,"4\n")==0){
			printf("Se ha enviado al servidor una peticion para borrar sus localizaciones de la base de datos.\n");
			strcpy(variable,CMD[3]);
			if(sprintf(buf, "%s %d", variable, id)<0){
				printf("Error al generar el comando");
			}
			if(sendto(sock, buf, strlen(buf), 0, (struct sockaddr *) &dir_serv, sizeof(dir_serv)) < 0)
			{
				perror("Error al enviar el mensaje");
				exit(1);
			}
			//Me tengo que quedar a la espera de un mensaje, OK o ER.
			tam_dir = sizeof(dir_serv);
			if((n = recvfrom(sock, buf, MAX_BUF,0, (struct sockaddr *) &dir_serv, &tam_dir)) < 0)
			{
				perror("Error al recibir la respuesta");
				exit(1);
			}
			//Compruebo la respuesta obtenida.
			buf[n] = '\0';
			printf("%s",buf);
			if(strcmp(buf, "ER\n")==0){
				printf("Ha habido un error.\n");
			}else if(strcmp(buf, "OK\n")==0){
				printf("Operacion realizada correctamente\n");
			}else{
				printf("No se ha obtenido una respuesta del servidor\n");
			}
			memset(buf,0,strlen(buf));			
			//RST + ID
		}else if(strcmp(seleccion,"5\n")==0){
							close(sock);
							exit(0);
		}
		imprimirMenu(); //Muestra el menu de nuevo para que el usuario pueda ver las opciones		
	}	
	
	//Finalizacion de la comunicacion
	if((n = recvfrom(sock, NULL, MAX_BUF,0, (struct sockaddr *) &dir_serv, &tam_dir)) < 0)	
	{
		perror("Error al enviar el mensaje de fin de comunicacion");
		exit(1);
	}
	exit(0);
return 0;
}
