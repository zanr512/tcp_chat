#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>



#define MAX_CLIENTS	10



static volatile int delaj = 1;

void handler(int t)
{
	printf("\n\nSERVER IZKLOPLJEN\n\n");
	delaj = 0;
	exit(1);
}


static unsigned int client_st = 0;
static int uid = 10;

/* Client structure */
typedef struct {
	int socket_fd;
	int uid;
	char ime[32];
} client_t;

client_t *clients[MAX_CLIENTS];


void dodaj_client(client_t *cl){
	int i;
	for(i=0;i<MAX_CLIENTS;i++){
		if(!clients[i]){
			clients[i] = cl;
			return;
		}
	}
}

void odstrani_client(int uid){
	int i;
	for(i=0;i<MAX_CLIENTS;i++){
		if(clients[i]){
			if(clients[i]->uid == uid){
				clients[i] = NULL;
				return;
			}
		}
	}
}

void odstrani_ime(char *s, const char *odstrani){
	while(s = strstr(s,odstrani))
	{
		memmove(s,s+strlen(odstrani),1+strlen(s+strlen(odstrani)));
	}
}

void poslji_vsem(char* s)
{
	int i;
	for(i=0;i<MAX_CLIENTS;i++){
		if(clients[i]){
			send(clients[i]->socket_fd, s, strlen(s),0);
		}
	}
}




void *funkcija(void *arg){
	char data[1024];
	char warn[50];
	int prejeti_biti;

	client_st++;
	client_t *p_client = (client_t *)arg;

	printf(">>> Uporabnik %s se je povezal\n", p_client->ime);
	strcpy(warn,">>>");
	strcat(warn,p_client->ime);
	strcat(warn," se je priduzil klepetu");
	poslji_vsem(warn);
	
	while(1) 
	{
		if ((prejeti_biti = recv(p_client->socket_fd, data, 1024,0))== -1) 
		{
			perror("NAPAKA");
			exit(1);
		}
		else if (prejeti_biti == 0) 
		{
			printf(">>>Uporabnik %s je prekinil povezavo\n",p_client->ime);
			break;
		}
		
		data[prejeti_biti] = '\0';
		
		if(strncmp(data,"/name",5) == 0)
		{
			odstrani_ime(data, "/name ");
			
			strcpy(warn,">>>");
			strcat(warn,p_client->ime);
			strcat(warn," si je spremenil ime v ");
			strcat(warn,data);
			
			poslji_vsem(warn);
			
			strcpy(p_client->ime, data);
		}
		else if(strncmp(data,"/quit",5) == 0)
		{
			printf(">>>Uporabnik %s je prekinil povezavo\n",p_client->ime);
			strcpy(warn,">>>");
			strcat(warn,p_client->ime);
			strcat(warn," je prekinil povzavo");
			odstrani_client(p_client->uid);
			poslji_vsem(warn);
			close(p_client->socket_fd);
			pthread_detach(pthread_self());
			return NULL;
		}
		else
		{
			char nov[1024] = {};
			strcat(nov, p_client->ime);
			strcat(nov, ": ");
			strcat(nov, data);
			printf("Server: %s\n", data);
			poslji_vsem(nov);
		}


	}
	close(p_client->socket_fd);

	odstrani_client(p_client->uid);
	pthread_detach(pthread_self());
	
	return NULL;
}

int main(int argc, char *argv[]){
	char *port_string = "5000";

	getopt(argc, argv, "p:");
	
	int e = 0;
	for(e = 0; e < argc; e++)
	{
		if(strncmp(argv[e],"-p",2) == 0)
			 port_string = argv[e+1];
	}
	
	int port = atoi(port_string);
	
	
	
	int listenfd = 0, socket_fd = 0;
	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	pthread_t tid;

	/* Socket settings */
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port); 

	/* Bind */
	if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		perror("NAPAKA: ");
		return 1;
	}

	/* Listen */
	if(listen(listenfd, 10) < 0){
		perror("NAPAKA: ");
		return 1;
	}

	printf("SERVER DELUJE\n");
	
	signal(SIGINT,handler);

	/* Accept clients */
	while(delaj){
		socklen_t clilen = sizeof(cli_addr);
		socket_fd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);


		/* Client settings */
		client_t *p_client = (client_t *)malloc(sizeof(client_t));
		p_client->socket_fd = socket_fd;
		p_client->uid = uid++;
		
		char name_tmp[30] = {"ANON"};
		char cifra[20];
		sprintf(cifra, "%d", client_st);
		strcat(name_tmp,cifra);
		strcpy(p_client->ime, name_tmp);

		dodaj_client(p_client);
		pthread_create(&tid, NULL, &funkcija, (void*)p_client);

		sleep(1);
	}
	
}
