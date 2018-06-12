#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Librerias para sockets.
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

//Librerias para RT y scheduling-
#include <sched.h>
#include <sys/resource.h>

#define MAX_BUF 1024
#define PORT 50005

//Definicion de los comandos.
#define SAV 0
#define LST 1
#define LSL 2
#define RST 3

//Array para buscar los comandos
char * COMANDOS[] = {"SAV", "LST", "LSL", "RST", NULL};
//ID posibles
int  ID=373;

///////////////FUNCIONES AUXILIARES/////////////
void terminate(char* msg){
	perror(msg);
	exit(0);
}

int buscar_substring(char *string, char **strings)
{
	int i = 0;
	while(strings[i] != NULL)
	{
		if(!strncmp(string,strings[i],strlen(strings[i])))
			return i;
		i++;
	}
	return -1;
}

////////////PROGRAMA PRINCIPAL////////////////
int main(){
	
	struct sched_param param;
	param.sched_priority = 85;
	struct rlimit limit;
	
	//Se configura el scheduler.
	if(sched_setscheduler(0, SCHED_RR, &param) == -1)
		terminate("sched_setscheduler() failed");
	
	int sock, n;
	struct sockaddr_in dir_serv, dir_cli;
	socklen_t tam_dir;
	char buf[MAX_BUF];

	//Para la creacion de los numero aleatorios
	float lat;
	float lon;	

	int i;
	int comando;
	int result;

	//Obtener del buffer la longitud y la latitud
	float num_lat, num_lon;

	//Obtenidas del buffer el ID y las N posiciones
	int num_id, num_n;
	
	//Creo el socket para el protocolo UDP
	if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("Error al crear el socket");
		exit(1);
	}
	
	memset(&dir_serv, 0, sizeof(dir_serv));
	dir_serv.sin_family = AF_INET;
	dir_serv.sin_addr.s_addr = htonl(INADDR_ANY);
	dir_serv.sin_port = htons(PORT);
		
	//Asigno una direccion al socket	
	if(bind(sock, (struct sockaddr *) &dir_serv, sizeof(dir_serv)) < 0)
	{
		perror("Error al asignarle una direccion al socket");
		exit(1);
	}
	
	//Bucle principal de la aplicacion
	while(1)
	{
		tam_dir = sizeof(dir_cli);
		//Esperamos a recibir algo del cliente
		if((n=recvfrom(sock, buf, MAX_BUF, 0, (struct sockaddr *) &dir_cli, &tam_dir)) < 0)
		{
			perror("Error al recibir datos");
			exit(1);
		}
		printf("He recibido una peticion del cliente,es %s \n",buf);

		//Comprobar si el comando es conocido
		if((comando=buscar_substring(buf,COMANDOS))<0)
		{
			perror("Error al recibir comando.");
			exit(1);
		}
		//Realizar la operación correspondiente segun comando recibido
		switch(comando){
			//En caso de que el cliente desee guardar la localizacion ->SAV LAT LON ID
			case  SAV:
				//Miro si el ID existe
				sscanf(buf,"SAV %f %f %d", &num_lat, &num_lon, &num_id);	
				if(num_id!=ID)
				{	//Si no coincide el ID con el ID especificado, envio un mensage de error
					if(sendto(sock, "ER\n", 3, 0, (struct sockaddr *) &dir_cli, tam_dir) < 0)
					{
						perror("Error al enviar datos");
						exit(1);
					}
				}else{
					//De encontrar el ID devuelvo OK
					if(sendto(sock, "OK\n", 3, 0, (struct sockaddr *) &dir_cli, tam_dir) < 0)
					{
						perror("Error al enviar datos");
						exit(1);
					}
				}
				break;

			//En caso de que el cliente desee obtener las ultimas n localizaciones ->LST ID N
			case LST:
				//Miro si el ID exite
				sscanf(buf,"LST %d %d", &num_id, &num_n);
				if(num_id!=ID)
				{	//Si no coincide el ID con el ID especificado, envio un mensage de error
					if(sendto(sock, "ER\n", 3, 0, (struct sockaddr *) &dir_cli, tam_dir) < 0)
					{
						perror("Error al enviar datos");
						exit(1);
					}
				}else{

					//Envio tantas coordenadas como se hayan pedido
					for(i=0;i<num_n;i++){
				
						//Creo las coordenadas
						lon= ((float)rand()/(float)(RAND_MAX)) * 90;
						lat= ((float)rand()/(float)(RAND_MAX)) * 90;
					
						//Imprimo las coordenadas en el buffer			
						if(sprintf(buf, "LAT:%f LON:%f \n", lat, lon)<0)
						{
							printf("Error al generar el comando\n");
						}
						//Envio las coordenadas al cliente
						if(sendto(sock, buf, MAX_BUF, 0, (struct sockaddr *) &dir_cli, tam_dir) < 0)
						{
							perror("Error al enviar datos");
							exit(1);
						}
						buf[n] = '\0';
						memset(buf,0,strlen(buf));//Vacio el bufer a la espera del siguiente envio

					}
					//Envio el codigo "OK" de que todo a salido bien 
					if(sendto(sock, "OK\n", 3, 0, (struct sockaddr *) &dir_cli, tam_dir) < 0)
					{
						perror("Error al enviar datos");
						exit(1);
					}
				}
				break;
			//En caso de que el cliente desee obtener la ultima localizacion ->LSL ID
			case LSL:
				//Miro si el ID exite
				sscanf(buf,"LSL %d", &num_id);
				if(num_id!=ID)
				{	//Si no coincide el ID con el ID especificado, envio un mensage de error
					if(sendto(sock, "ER\n", 3, 0, (struct sockaddr *) &dir_cli, tam_dir) < 0)
					{
						perror("Error al enviar datos");
						exit(1);
					}
				}else{
					//Establezco coordenadas fijas, como si fueran las ultimas coordenadas
					lat = 13.4; 
					lon = 12.5;
					//Imprimo las coordenadas en el buffer
					if(sprintf(buf, "LAT:%f  LON:%f \n", lat, lon)<0){
						printf("Error al generar el comando\n");
					}
					//Envio las coordenadas al cliente
					if(sendto(sock, buf, MAX_BUF, 0, (struct sockaddr *) &dir_cli, tam_dir) < 0)
					{
						perror("Error al enviar datos");
						exit(1);
					}
					//Envio el codigo "OK" de que todo ha ido bien
					if(sendto(sock, "OK\n", 3, 0, (struct sockaddr *) &dir_cli, tam_dir) < 0)
					{
						perror("Error al enviar datos");
						exit(1);
					}
				}
				break;
			//En caso de que el cliente desee borrar las localizaciones ->RST ID
			case RST:
				//Miro si el ID exite
				sscanf(buf,"RST %d", &num_id);
				if(ID!=num_id)
				{	//Si no coincide el ID con el ID especificado, envio un mensage de error
					if(sendto(sock, "ER\n", 3, 0, (struct sockaddr *) &dir_cli, tam_dir) < 0)
					{
						perror("Error al enviar datos");
						exit(1);
					}
				}else{
					//De encontrar el ID devuelvo OK
					if(sendto(sock, "OK\n", 3, 0, (struct sockaddr *) &dir_cli, tam_dir) < 0)
					{
						perror("Error al enviar datos");
						exit(1);
					}
				}
				break;

		}
		memset(buf,0,strlen(buf));//Vacio el bufer a la espera del siguiente comando
	}
	close(sock);
}
