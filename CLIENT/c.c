
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <ncurses.h>


int hostname_to_ip(char *  , char *);



void *handle_client(void *arg){
	int *socket_fd = (int *)arg;
	char data[1024];
	
	WINDOW *a;
	int maxx, maxy;
	initscr();
	refresh();
	getmaxyx(stdscr, maxy, maxx);
	a = newwin(maxy*0.75, maxx, 0, 0);
	wrefresh(a);
	scrollok(a,TRUE);
	
	while(1)
	{
		int prejeti_biti;
	
		prejeti_biti = recv(socket_fd, data, sizeof(data),0);
		if ( prejeti_biti <= 0 )
		{
			printf("Povezava je bila prekinjena\n");
			endwin();
			exit(1);
		}

		data[prejeti_biti] = '\0';

			
		wprintw(a,"%s\n",data);
		
		
		
		
		if(strstr(data,"www") != 0)
		{
			char url[50];
			char *s = strstr(data,"www.");
			sprintf(data,"%s",data+(s-data));
			strcpy(url,"x-www-browser ");
			strcat(url,data);
			system(url);
			
		}
		
		wrefresh(a);
	}
	
	
}

int main(int argc, char *argv[]){
	pthread_t tid;
	
	char data[1000];
	
	char ip[100];

	char *addr_string = 0;
    char *port_string = "9999";

	getopt(argc, argv, "a:p:");
	int e = 0;
	for(e = 0; e < argc; e++)
	{
		if(strncmp(argv[e],"-p",2) == 0)
			 port_string = argv[e+1];
		if(strncmp(argv[e],"-a",2) == 0)
			 addr_string = argv[e+1];
	}
   


    int ai_family = AF_INET; // ipv4, AF_INET6 za ipv6
    int ai_type = SOCK_STREAM; // SOCK_STREAM za tcp, SOCK_DGRAM za UDP

    int socket_fd;
    socket_fd = socket(ai_family, ai_type, 0);
    if(socket_fd == -1){
        perror("NAPAKA");
        return -1;
    }
    
    // pripravimo in povezemo vtic z naslovom
    struct sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(struct sockaddr_in));
    sock_addr.sin_family = ai_family;
    // port najprej spremenimo v stevilo
    int port_int = atoi(port_string);
    // nato je treba to stevilo transformirati iz lokalnega formata
    // v omrezni format 
    sock_addr.sin_port = htons(port_int);
    
    
    hostname_to_ip(addr_string , ip);
    
    
    
    if(inet_aton(ip, &(sock_addr.sin_addr))==0){
        printf("Napaka pri spreminjanju naslova v binarnega\n");
        return -1;
    }
    
    // povezimo se na streznik
    if(connect(socket_fd, (struct sockaddr*)&sock_addr, sizeof(struct sockaddr_in))!=0)
    {
        perror("Napaka pri povezavi na server");
        return -1;
    }
    


	int stevec = 0;
	
	WINDOW  *b, *c;
	int maxx, maxy;
	initscr();
	refresh();
	start_color();
	init_pair(2, COLOR_WHITE, COLOR_RED);
	init_pair(3, COLOR_BLACK, COLOR_CYAN);
	getmaxyx(stdscr, maxy, maxx);
	b = newwin(maxy*0.22, maxx, maxy*0.75, 0);
	c = newwin(maxy*0.03, maxx, maxy*0.97, 0);
	wbkgd(b, COLOR_PAIR(2));
	wbkgd(c, COLOR_PAIR(3));
	wrefresh(b);
	mvwaddstr(c, 0, 0, "Spremeni si ime: /name <ime>  | Izhod: /quit");
	wrefresh(c);
	
	
	pthread_create(&tid, NULL, &handle_client, (void*)socket_fd);
	
	
	while(1)
	{
		wgetstr(b,data);
		if ((send(socket_fd,data, strlen(data),0))== -1) {
                fprintf(stderr, "Napaka pri pošiljanju sporocila\n");
                close(socket_fd);
                exit(1);
        }
        if(strncmp(data,"/quit",5) == 0)
        {
			break;
		}
		werase(b);
		wrefresh(b);
	}
	
    close(socket_fd);

	endwin();
    exit(1);
}

int hostname_to_ip(char * addr_string , char* ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
         
    if ( (he = gethostbyname( addr_string ) ) == NULL) 
    {
        // get the host info
        herror("gethostbyname");
        return 1;
    }
 
    addr_list = (struct in_addr **) he->h_addr_list;
     
    for(i = 0; addr_list[i] != NULL; i++) 
    {
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;
    }
    return 1;
}

