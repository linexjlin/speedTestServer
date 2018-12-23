/*
AUTHOR: linexjlin

A simple http server for server network speed test.

usage  
$./a.out -p 10000 
*/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<signal.h>
#include<fcntl.h>

#define CONNMAX 10
#define BYTES 1024
#define BUFSIZE 512*1024/sizeof(int)

int listenfd, clients[CONNMAX];
int int_len;
void error(char *);
void startServer(char *);
void respond(int);

int buffer[1][BUFSIZE];

int init_buffer() {
	int i,j;
	for ( i = 0; i <1; i++) {
		for (j=0; j<BUFSIZE; j++) {
			buffer[i][j]=fastrand();
		}
	}
}

int main(int argc, char* argv[])
{
    init_buffer();
    int_len=sizeof(int);
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    char c;    
    
    //Default Values PATH = ~/ and PORT=10000
    char PORT[6];
    strcpy(PORT,"10000");

    int slot=0;

    //Parsing the command line arguments
    while ((c = getopt (argc, argv, "p:r:")) != -1)
        switch (c)
        {
         /*   case 'r':
                ROOT = malloc(strlen(optarg));
                strcpy(ROOT,optarg);
                break;*/
            case 'p':
                strcpy(PORT,optarg);
                break;
            case '?':
                fprintf(stderr,"Wrong arguments given!!!\n");
                exit(1);
            default:
                exit(1);
        } 
    printf("Server started at port no. %s%s%s \n","\033[92m",PORT,"\033[0m");
    // Setting all elements to -1: signifies there is no client connected
    int i;
    for (i=0; i<CONNMAX; i++)
        clients[i]=-1;
    startServer(PORT);

    // ACCEPT connections
    while (1)
    {
        addrlen = sizeof(clientaddr);
        clients[slot] = accept (listenfd, (struct sockaddr *) &clientaddr, &addrlen);

        if (clients[slot]<0)
            error ("accept() error");
        else
        {
            if ( fork()==0 )
            {
                respond(slot);
                exit(0);
            }
        }

        while (clients[slot]!=-1) slot = (slot+1)%CONNMAX;
    }

    return 0;
}

//start server
void startServer(char *port)
{
    struct addrinfo hints, *res, *p;

    // getaddrinfo for host
    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo( NULL, port, &hints, &res) != 0)
    {
        perror ("getaddrinfo() error");
        exit(1);
    }
    // socket and bind
    for (p = res; p!=NULL; p=p->ai_next)
    {
        listenfd = socket (p->ai_family, p->ai_socktype, 0);
        if (listenfd == -1) continue;
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
    }
    if (p==NULL)
    {
        perror ("socket() or bind()");
        exit(1);
    }

    freeaddrinfo(res);

    // listen for incoming connections
    if ( listen (listenfd, 1000000) != 0 )
    {
        perror("listen() error");
        exit(1);
    }
}

//client connection
void respond(int n)
{
    char mesg[99999], *reqline[3], data_to_send[BYTES], path[99999];
    int rcvd, fd, bytes_read;

    memset( (void*)mesg, (int)'\0', 99999 );

    rcvd=recv(clients[n], mesg, 99999, 0);

    if (rcvd<0)    // receive error
        fprintf(stderr,("recv() error\n"));
    else if (rcvd==0)    // receive socket closed
        fprintf(stderr,"Client disconnected upexpectedly.\n");
    else    // message received
    {
        printf("%s", mesg);
        reqline[0] = strtok (mesg, " \t\n");
        if ( strncmp(reqline[0], "GET\0", 4)==0 )
        {
            reqline[1] = strtok (NULL, " \t");
            reqline[2] = strtok (NULL, " \t\n");
            if ( strncmp( reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0 )
            {
                write(clients[n], "HTTP/1.0 400 Bad Request\n", 25);
            }
            else
            {
                if ( strncmp(reqline[1], "/\0", 2)==0 )
                    reqline[1] = "/index.html";        //Because if no file is specified, index.html will be opened by default (like it happens in APACHE...

                char file_name[sizeof(reqline)];
                strcpy(file_name,reqline[1]);
                int file_size;
                
                //printf("reqline: %s; file_name: %sis: %d\n",reqline[1],file_name,strcmp(reqline[1],"/100M"));
                if ( strncmp(file_name,"/128M",5)==0) {
                   file_size=128*1024*1024; 
                } else if ( strncmp(file_name,"/256M",5)==0) {
                   file_size=256*1024*1024; 
                } else if ( strncmp(file_name,"/512M",5)==0) {
                   file_size=512*1024*1024;
                } else if ( strncmp(file_name,"/1G",3)==0) {
                   file_size=1024*1024*1024;
                } else {
                   file_size=0;
                }

                printf("file name is: %s\n", file_name);
                printf("file size is: %d bytes\n",file_size);
	        
		char *header;
                send(clients[n], "HTTP/1.0 200 OK\r\n", 17, 0);
                if (file_size>0) {
                  int i;
                  srand( (unsigned)time( NULL ) );          

		  asprintf(&header,"Accept-Ranges: bytes\r\nContent-Length: %d\r\n\r\n",file_size);
                  write(clients[n], header,strlen(header));
                  for( i = 0; i < file_size/(BUFSIZE*sizeof(int));i++ )
                  {
                    write (clients[n], buffer[0], sizeof(buffer[0]));
                  }
                } else {
                  char msg[]="<html>\
<title>Speed Test File List</title>\
<body>\
<h2>Speed Test Files:</h2>\
<hr>\
<ul>\
<li><a href=\"128M\">128M</a>\
<li><a href=\"256M\">256M</a>\
<li><a href=\"512M\">512M</a>\
<li><a href=\"1G\">1G</a>\
</ul>\
<hr>\
</body>\
</html>\n";
		  asprintf(&header,"Content-Type: text/html; charset=UTF-8\r\n\
Accept-Ranges: bytes\r\n\
Connection: keep-alive\r\n\
Content-Length: %d\r\n\r\n",strlen(msg));

                  write(clients[n], header,strlen(header));
                  write(clients[n], msg, strlen(msg));
                }
		free(header);
            }
        }
    }

    //Closing SOCKET
    shutdown (clients[n], SHUT_RDWR);         //All further send and recieve operations are DISABLED...
    close(clients[n]);
    clients[n]=-1;
}


inline int fastrand() {
      int g_seed = (214013*g_seed+2531011);
            return (g_seed>>16)&0x7FFF;
}
