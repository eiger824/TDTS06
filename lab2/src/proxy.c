#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <getopt.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "definitions.h"

void help(const char* prog);
void info();
void *handle_client(void *arg);

int main(int argc, char *argv[])
{
  int sockfd, sockfdp, newsockfd, clilen, err;
  char buffer[MAX_BUFFER_LENGTH];
  bool debug = false;
  bool hex = false;
  struct sockaddr_in serv_addr, cli_addr;
 
  //to use with getaddrinfo() -> host discovery
  struct addrinfo hints, *servinfo, *p;
  int n,c,rv,ret;

  while ((c = getopt(argc,argv,"dHhm:p:v")) != -1)
    {
      switch(c)
	{
	case 'd':
	  debug = true;
	  break;
	case 'p':
	  portno = atoi(optarg);
	  break;
	case 'm':
	  MAX_SIM_REQUESTS = atoi(optarg);
	  break;
	case 'H':
	  hex = true;
	  break;
	case 'h':
	  help(argv[0]);
	  exit(0);
	case 'v':
	  info(argv[0]);
	  exit(0);
	default:
	  fprintf(stderr, "Unknown option\n");
	  exit(1);
	}
    }

#ifdef DEBUG_MODE
  printf("Debug flag is %s, selected port is %d and maximum simultaneous requests is %d\n",
	 (debug)?"enabled":"disabled",
	 portno,
	 MAX_SIM_REQUESTS);
#endif
  
  //Allocate memory for the threads
  threads = malloc(MAX_SIM_REQUESTS * sizeof(pthread_t));

  //Socket creation
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    {
      perror("ERROR opening socket");
      exit(1);
    }

  //Initialize the data structure
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  //Bind the socket with the server address
  if (bind(sockfd, (struct sockaddr *) &serv_addr,
	   sizeof(serv_addr)) < 0)
    {
      perror("ERROR on binding");
      exit(1);
    }

  //Put the socket on listen mode
  if (listen(sockfd, MAX_SIM_REQUESTS) == -1)
    {
      perror("ERROR: could not set listen mode");
      exit(1);
    }
  
  //Init addrinfo struct
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET; // IPv4
  hints.ai_socktype = SOCK_STREAM; //TCP

  //Get size of the structure
  clilen = sizeof(cli_addr);

  printf("Starting server, listening on port %d ...\n", portno);
  while(1) {
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0)
      {
	perror("ERROR on accept");
	exit(1);
      }
    else
      {
	//Create a thread to handle the incoming connection
	err = pthread_create(&(threads[get_size(threads)]), NULL, &handle_client, NULL);
	if (err != 0)
	  {
	    printf("Could not create thread :[%s]\n", strerror(err));
	  }
#ifdef DEBUG_MODE
        else
	  {
	    printf("Thread created successfully. Starting request handling\n");
	  }
#endif
      }
    bzero(buffer,MAX_BUFFER_LENGTH);
    n = read(newsockfd,buffer,MAX_BUFFER_LENGTH-1);
    if (n < 0) fprintf(stderr, "ERROR reading from socket\n");

#ifdef DEBUG_MODE
    if (debug)
      printf("Received data: [");
      if (hex)
	hexify(buffer);
      else
	printf("%s", buffer);
      printf("]\n");
#endif
   
    char hostname[200];
    if (parse_hostname(hostname, buffer))
      {
	printf("The hostname extracted is [%s]\n", hostname);
      }
    else
      {
	fprintf(stderr, "Error parsing hostname\n");
      }
    
    //Get address info of the parsed hostname
    if ((rv = getaddrinfo(hostname, "http", &hints, &servinfo)) != 0)
      {
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	exit(1);
      }
  

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
      if ((sockfdp = socket(p->ai_family, p->ai_socktype,
			    p->ai_protocol)) == -1) {
	perror("socket");
	continue;
      }

      if (connect(sockfdp, p->ai_addr, p->ai_addrlen) == -1) {
	perror("connect");
	close(sockfdp);
	continue;
      }

      break; // if we get here, we must have connected successfully
    }

    if (p == NULL) {
      // looped off the end of the list with no connection
      perror("failed to connect");
      exit(2);
    }


    /*
      Action!
    */

    if ((n = write(sockfdp, buffer, strlen(buffer))) < 0 )
      {
	perror("Write");
      }
#ifdef DEBUG_MODE
    else
      {
	if (debug)
	  printf("(%d bytes written)\n", n);
      }
#endif

    /*
      Read response from server after cleaning sending buffer
    */
    memset(buffer, 0, strlen(buffer));
    if ((n = read(sockfdp, buffer, MAX_BUFFER_LENGTH)) < 0)
      {
	perror("Error reading from socket");
      }
#ifdef DEBUG_MODE
    else
      {
	printf("Received %d bytes from server", n);
      }
#endif
  }
  
  freeaddrinfo(servinfo); // all done with this structure
  return 0; 
}

  void help(const char* prog)
  {
    printf("USAGE: %s [ARGS]\n", prog);
    printf("Args:\n");
    printf("-d\tTurn on debug mode\n");
    printf("-h\tShow this help and exit\n");
    printf("-H\tShow the HTTP data in hex format\n");
    printf("-m max  Specify the maximum number of simultaneous connections the proxy\n");
    printf("        can have (defaults to 10)\n");
    printf("-p port Specify the listening port (defaults to %d)\n", portno);
    printf("-v\tShow version & authors and exit\n");
  }

  void info()
  {
    printf("Proxy, version %s, by Goutam Bhat (goubh275) and Santiago Pagola (sanpa993)\n", VERSION);
  }

  void *handle_client(void *arg)
  {
    pthread_t id = pthread_self();
#ifdef DEBUG_MODE
    printf("Thread id: %lu\n", id);
#endif
  }
