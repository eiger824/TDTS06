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
//#include <regex.h>

const unsigned MAX_BUFFER_LENGTH = 10000;

int main(int argc, char *argv[])
{
  int sockfd, sockfdp, newsockfd, portno, clilen;
  char buffer[MAX_BUFFER_LENGTH];
  struct sockaddr_in serv_addr, cli_addr;
  //regex_t regex;
  //regmatch_t pmatch;
  //to use with getaddrinfo() -> host discovery
  struct addrinfo hints, *servinfo, *p;
  int n,c,rv,ret;
  //Default listen port

  portno = 3422;

  while ((c = getopt(argc,argv,"p:")) != -1)
    {
      switch(c)
	{
	case 'p':
	  portno = atoi(optarg);
	  break;
	default:
	  fprintf(stderr, "Unknown option\n");
	  exit(1);
	}
    }

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) 
    fprintf(stderr, "ERROR opening socket\n");
  bzero((char *) &serv_addr, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
 
  if (bind(sockfd, (struct sockaddr *) &serv_addr,
	   sizeof(serv_addr)) < 0) 
    fprintf(stderr, "ERROR on binding\n");

  if (listen(sockfd,5) == -1)
    {
      fprintf(stderr, "ERROR: could not set listen mode. Exiting\n");
      exit(1);
    }
  
  printf("Starting server, listening on port %d ...\n", portno);
  clilen = sizeof(cli_addr);

  // Init addrinfo struct
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET; // IPv4
  hints.ai_socktype = SOCK_STREAM; //TCP

  while(1) {
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) 
      fprintf(stderr, "ERROR on accept\n");
    bzero(buffer,MAX_BUFFER_LENGTH);
    n = read(newsockfd,buffer,MAX_BUFFER_LENGTH-1);
    if (n < 0) fprintf(stderr, "ERROR reading from socket\n");
    //buffer[strlen(buffer)-1] = 0;
		
    printf("Received data: [%s]\n", buffer);
   

    /*    //define our regex to extract host name
	  ret = regcomp(&regex, "^Host:", 0);
	  if (ret)
	  {
	  fprintf(stderr, "ERROR, could not compile regexp\n");
	  }
    
	  ret = regexec(&regex, buffer, 0 , pmatch, 0);
	  if (!ret)
	  {
	
	  }
    */
    char hostname[200];
    for (unsigned i=0; i < strlen(buffer)-5; ++i)
      {
	if (buffer[i] == 'H' &&
	    buffer[i+1] == 'o' &&
	    buffer[i+2] == 's' &&
	    buffer[i+3] == 't' &&
	    buffer[i+4] == ':')
	  {
	    unsigned j=i+6;
	    do {
	      hostname[j-(i+6)] = buffer[j];
	      ++j;
	    } while (buffer[j] != '\r');
	    hostname[j-(i+6)] = 0; //Null-terminate
	    break;
	  }
      }

    printf("The hostname extracted is [%s]\n", hostname);
    
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
      fprintf(stderr, "failed to connect\n");
      exit(2);
    }


    /*
      Action!
    */

    if ((n = write(sockfdp, buffer, strlen(buffer))) < 0 )
      {
	perror("Write");
      }
    else
      {
	printf("(%d bytes written)\n");
      }

  }
  freeaddrinfo(servinfo); // all done with this structure
  return 0; 
}

