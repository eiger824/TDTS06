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
#include <signal.h>
#include "definitions.h"
#include "log.h"

int signo;                                               /* Signal nr to pass on to signal handler */
bool hex = false, omit = false;                          /* Data in hex/dec, print data on/off */
unsigned nr_active_threads = 0;                           /* Number of currently running threads */
int sockfd;                                              /* Server socket fd (client-proxy side) */
int newsockfd;                                           /* Client socket fd (new incoming
                                                            connection) */
int sockfdp;                                             /* Socket file descriptor (proxy-server
                                                            side) */
struct thread_data thread_data_array[MAX_BUFFER_LENGTH]; /* Array of thread data structs */
pthread_mutex_t lock;                                    /* Mutex thread */

//Functions
void help(const char* prog);
void info();
void *handle_client(void *arg);
void sig_handler(int signo);
void mutex_lock();
void mutex_unlock();
void init_thread_array();
void reset_thread_struct(struct thread_data* ptr);

int main(int argc, char *argv[])
{
  int clilen, err, c, i;
  struct sockaddr_in serv_addr, cli_addr;
  
  init_thread_array();

  //First of all, register our signal handler
  if (signal(SIGINT, sig_handler) == SIG_ERR)
    {
      perror("Error registering signal handler");
    }

  while ((c = getopt(argc,argv,"Hhm:op:v")) != -1)
    {
      switch(c)
	{
	case 'h':
	  help(argv[0]);
	  exit(0);
	case 'H':
	  hex = true;
	  if (omit)
            {
	      fprintf(stderr, "ERROR: -H and -o options cannot be provided at the same time\n");
	      help(argv[0]);
	      exit(1);
            }
	  break;
	case 'm':
	  MAX_SIM_REQUESTS = atoi(optarg);
	  break;
	case 'o':
	  omit = true;
	  if (hex)
            {
	      fprintf(stderr, "ERROR: -H and -o options cannot be provided at the same time\n");
	      help(argv[0]);
	      exit(1);
            }
	  break;
	case 'p':
	  portno = atoi(optarg);
	  break;
	case 'v':
	  info(argv[0]);
	  exit(0);
	default:
	  fprintf(stderr, "Unknown option\n");
	  exit(1);
	}
    }

#ifdef DEBUG_MODE
  printf("Debug compilation flag:\t\tenabled\n");
  printf("Selected port:\t\t\t%d\n", portno);
  printf("Maximum simultaneous requests:\t%d\n", MAX_SIM_REQUESTS);
  if (!omit)
    printf("Hex mode when printing data:\t%s\n", (hex)?"enabled":"disabled");
  else
    printf("Display received data buffer:\tdisabled\n");
  printf("------------------------------------------------------\n");
#endif

  //Allocate memory for the threads
  threads = malloc(MAX_SIM_REQUESTS * sizeof(pthread_t));

  //Initialize or mutex thread
  if ((err = pthread_mutex_init(&lock, NULL)) != 0)
    {
      perror("Mutex init failed");
      return 1;
    }
#ifdef DEBUG_MODE
  else
    {
      printf("Mutex init success!\n");
    }
#endif

  //Socket creation (proxy, server-side)
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

  //Get size of the structure
  clilen = sizeof(cli_addr);

  //main loop
  printf("Starting server, listening on port %d ...\n", portno);
  while(1)
    {
      newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
      if (newsockfd < 0)
	{
	  perror("ERROR on accept");
	  exit(1);
	}
      else
	{
	  int found = 0;
	  while (!found)
	    {
	      for (i=0; i<MAX_SIM_REQUESTS; ++i)
		{
		  if (thread_data_array[i].thread_id < 0)
		    {
		      thread_data_array[i].thread_id = i;
		      thread_data_array[i].priority = HIGH;
		      thread_data_array[i].cli_sock_fd = newsockfd;
		      //Create a thread to handle the incoming connection
		      err = pthread_create(&(threads[i]), NULL,
					   &handle_client, &thread_data_array[i]);
		      if (err != 0)
			{
			  printf("Could not create thread :[%s]\n", strerror(err));
			}

		      else
			{
			  mutex_lock();
			  ++nr_active_threads;
#ifdef DEBUG_MODE
			  printf("Thread created successfully. Starting request handling\n");	    
			  printf("# Active threads: %d (position in array: %d)\n", nr_active_threads, i);
#endif
			  mutex_unlock();
			}
		      found = 1;
		      break;
		    }
		} //for
	      //Sleep calling thread if no empty slots were found, i.e. if # of requests
	      //is over the maximum. After a while (0.3), then try again to find a slot
	      //in the thread structure list
	      printf("No empty slots\n");
	      usleep(1000);
	    }
	}
    }
   
  return 0; 
}

void help(const char* prog)
{
  printf("USAGE: %s [ARGS]\n", prog);
  printf("Args:\n");
  printf("-h\tShow this help and exit\n");
  printf("-H\tShow the HTTP data in hex format\n");
  printf("-m max  Specify the maximum number of simultaneous connections the proxy\n");
  printf("        can have (defaults to 10)\n");
  printf("-o\tOmit printing of data\n");
  printf("-p port Specify the listening port (defaults to %d)\n", portno);
  printf("-v\tShow version & authors and exit\n");
}

void info()
{
  printf("Proxy, version %s, by Goutam Bhat (goubh275) and Santiago Pagola (sanpa993)\n", VERSION);
}

void *handle_client(void *arg)
{   
  //***************** Variable declaration **************************
  /*
    The following variables are visible only in the local-scope
    since we want them to be used individually in every thread
  */
  char buffer[MAX_BUFFER_LENGTH];             /* Buffer to use when reading/writing data */
  int n, ret;                                 /* Return values (local) */
  char *hostname = malloc(200);               /* Hostname to parse */
  struct addrinfo hints, *servinfo, *p;       /* Use with getaddrinfo() -> host discovery */
  struct thread_data *data;                   /* Thread data structure passed onto this thread
						 function handler*/
  //******************************************************************

  data = (struct thread_data*) arg;
  printf("Thread id: %d, priority: [%s], associated client socket fd: %d\n", data->thread_id,
	 (data->priority)?"HIGH":"LOW",
	 data->cli_sock_fd);

  mutex_lock();
  bzero(buffer,MAX_BUFFER_LENGTH);
  n = read(data->cli_sock_fd,buffer,MAX_BUFFER_LENGTH-1);
  mutex_unlock();
  if (n < 0) perror("ERROR reading from socket");

#ifdef DEBUG_MODE
  printf("(C -> P )Client sends data:\n");
  if (!omit)
    print_data(buffer, hex);
#endif


  if ((ret = parse_hostname(hostname, buffer)) != -1)
    {
#ifdef DEBUG_MODE
      printf("The hostname extracted is [%s]\n", hostname);
#endif
    }
  else
    {
      fprintf(stderr, "Error parsing hostname\n");
    }
   
  //Init addrinfo struct
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET; // IPv4
  hints.ai_socktype = SOCK_STREAM; //TCP
   
  //Get address info of the parsed hostname
  if ((ret = getaddrinfo(hostname, "http", &hints, &servinfo)) != 0)
    {
      char reason[200];
      strcpy(reason, "getaddrinfo(");
      strcat(reason, hostname);
      strcat(reason, "):");
      strcat(reason, gai_strerror(ret));
      fprintf(stderr, "%s\n", reason);
      //Then abort the operation
      //Decrease nr. active threads
      mutex_lock();
      --nr_active_threads;
      //Release mutex
      mutex_unlock();
      //free allocated resources
      free(hostname);
      memset(buffer, 0, MAX_BUFFER_LENGTH);
      freeaddrinfo(servinfo);
      printf("Terminating thead-ID %d\n", data->thread_id);
      reset_thread_struct(data);
      //Exit thread
      pthread_exit(NULL);
    }

  // loop through all the results and connect to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfdp = socket(p->ai_family, p->ai_socktype,
			  p->ai_protocol)) == -1) {
      perror("socket");
      exit(1);
    }

    if (connect(sockfdp, p->ai_addr, p->ai_addrlen) == -1) {
      perror("connect");
      close(sockfdp);
      exit(1);
    }

    break; // if we get here, we must have connected successfully
  }

  if (p == NULL) {
    // looped off the end of the list with no connection
    perror("failed to connect");
    exit(2);
  }


  /*
    Write the request to the server
  */
  if ((n = write(sockfdp, buffer, strlen(buffer))) < 0 )
    {
      perror("Write");
    }
#ifdef DEBUG_MODE
  else
    {
      printf("(P --> S)Proxy forwards data (%d bytes)\n", n);
    }
#endif

  /*
    Read response from server after cleaning sending buffer
  */
  memset(buffer, 0, MAX_BUFFER_LENGTH);
  if ((n = read(sockfdp, buffer, MAX_BUFFER_LENGTH)) < 0)
    {
      perror("Error reading from socket");
    }
#ifdef DEBUG_MODE
  else
    {
      printf("(P <-- S)Received %d bytes from server:\n", n);
      //Send it back to the client
      if ((ret = write(data->cli_sock_fd, buffer, n)) < 0)
	{
	  perror("Write");
	}
      else
	{
	  printf("(C <-- P)Proxy forwarded data back to client (%d bytes)\n", ret);
	  close(data->cli_sock_fd);
	}
#endif

    }
  mutex_lock();
  --nr_active_threads;
  mutex_unlock();
  //free allocated resources
  free(hostname);
  memset(buffer, 0, MAX_BUFFER_LENGTH);
  freeaddrinfo(servinfo);
  //Exit thread
  printf("Terminating thead-ID %d\n", data->thread_id);
  reset_thread_struct(data);
  pthread_exit(NULL);
}

void sig_handler(int signo)
{
  if (signo == SIGINT)
    {
      printf("\rReceived SIGINT, Exiting program ...\n");
      if (sockfd >= 0) close(sockfd);
      if (newsockfd >= 0) close(newsockfd);
      if (sockfdp >= 0) close(sockfdp);

      //And exit program
      exit(0);
    }
}

void mutex_lock()
{
  pthread_mutex_lock(&lock);
}
void mutex_unlock()
{
  pthread_mutex_unlock(&lock);
}

void init_thread_array()
{
  unsigned i;
  for (i=0; i<MAX_SIM_REQUESTS; ++i)
    {
      thread_data_array[i].thread_id = DEAD;
      thread_data_array[i].priority = DEAD;
      thread_data_array[i].cli_sock_fd = DEAD;
    }

}

void reset_thread_struct(struct thread_data* ptr)
{
  ptr->thread_id = DEAD;
  ptr->priority = DEAD;
  ptr->cli_sock_fd = DEAD;
}
