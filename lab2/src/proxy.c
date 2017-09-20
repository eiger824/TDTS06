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
#include <sys/poll.h>

#include "log.h"
#include "definitions.h"
#include "url_filtering.h"
#include "content_filtering.h"
#include "filtering_common.h"

int signo;                                               /* Signal nr to pass on to signal handler */
bool hex = false, omit = false;                          /* Data in hex/dec, print data on/off */
unsigned nr_active_threads = 0;                          /* Number of currently running threads */
int sockfd;                                              /* Server socket fd (client-proxy side) */
int newsockfd;                                           /* Client socket fd (new incoming
                                                            connection) */

struct thread_data thread_data_array[MAX_BUFFER_LENGTH]; /* Array of thread data structs */
pthread_mutex_t lock;                                    /* Mutex thread */

//Functions
void help(const char* prog);
void info();
void free_resources(char* hostname,
                    char* current_hostname,
                    char* buffer,
                    struct addrinfo* serv,
                    struct thread_data* ptr);
void *handle_client(void *arg);
void sig_handler(int signo);
void mutex_lock();
void mutex_unlock();
void init_thread_array();
void reset_thread_struct(struct thread_data* ptr);

int setup_host_get_connection(char *hostname, struct addrinfo *servinfo,  int *sockfdp);  
int setup_host_connect_connection(char *hostname, struct addrinfo *servinfo,  int *sockfdp);  

int main(int argc, char *argv[])
{
   int err, c;
   socklen_t clilen;
   struct sockaddr_in serv_addr, cli_addr;

   //init variable
   gettimeofday(&time_before, NULL);

   //First of all, register our signal handler
   if (signal(SIGINT, sig_handler) == SIG_ERR)
   {
      perror("Error registering signal handler");
   }
   if (signal(SIGPIPE, sig_handler) == SIG_ERR)
   {
      perror("Error registering SIGPIPE");
   }

   while ((c = getopt(argc,argv,"dHhm:op:v")) != -1)
   {
      switch(c)
      {
         case 'd':
            log_set(1);
            break;
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
   printf("Debug compilation flag:\t\t%s\n", (log_get())?"enabled":"disabled");
   printf("Selected port:\t\t\t%d\n", portno);
   printf("Maximum simultaneous requests:\t%d\n", MAX_SIM_REQUESTS);
   if (!omit)
      printf("Hex mode when printing data:\t%s\n", (hex)?"enabled":"disabled");
   else
      printf("Display received data buffer:\tdisabled\n");
   printf("------------------------------------------------------\n");
#endif

   //Init the thread struct array with default values
   init_thread_array();

   //Init the logging mutex
   log_init();

   //Initialize or mutex thread
   if ((err = pthread_mutex_init(&lock, NULL)) != 0)
   {
      perror("Mutex init failed");
      return 1;
   }
#ifdef DEBUG_MODE
   else
   {
      log_info("Mutex init success!");
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
   log_info("Starting server, listening on port %d ...", portno);
   unsigned i;
   while(1)
   {
      newsockfd = accept((socklen_t)sockfd, (struct sockaddr *) &cli_addr, &clilen);
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
            //found = 0;
            for (i=0; i<MAX_SIM_REQUESTS; ++i)
            {
               if (thread_data_array[i].thread_id == DEAD)
               {

                  //Create a thread to handle the incoming connection
                  err = pthread_create(&(threads[i]), NULL,
                                       &handle_client, &thread_data_array[i]);
                  if (err != 0)
                  {
                     log_error("Could not create thread :[%s]", strerror(err));
                  }

                  else
                  {
                     thread_data_array[i].thread_id = i;
                     thread_data_array[i].priority = HIGH;
                     thread_data_array[i].cli_sock_fd = newsockfd;
                     mutex_lock();
                     ++nr_active_threads;
#ifdef DEBUG_MODE
                     log_info("Created thread. # Active threads: %d (position in array: %d)\n",
                              nr_active_threads, i);
#endif
                     mutex_unlock();
                  }
                  found = 1;
                  break;
               }
            } //for
            //if (!found)
            log_error("No empty slots (nr active threads is %d)", nr_active_threads);
         }
      }
   }

   return 0; 
}

void help(const char* prog)
{
   printf("USAGE: %s [ARGS]\n", prog);
   printf("Args:\n");
   printf("-d\tEnable debug mode\n");
   printf("-h\tShow this help and exit\n");
   printf("-H\tShow the HTTP data in hex format\n");
   printf("-m max  Specify the maximum number of simultaneous connections the proxy\n");
   printf("        can have (defaults to %d)\n", MAX_SIM_REQUESTS);
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
   char *current_hostname = malloc(200);       /* Current Hostname  */
   struct addrinfo *servinfo;       /* Use with getaddrinfo() -> host discovery */
   struct thread_data *data;                   /* Thread data structure passed onto this thread function handler*/
   int sockfdp;                                /* Socket file descriptor (proxy-server side) */
   int connection_type = 1;                    /* 1 for get, 2 for connect */

   struct pollfd ufds[2];                      /* Poll for events */
   int tid;                                    /* Thread ID of the current thread */
   //******************************************************************

   data = (struct thread_data*) arg;
   tid = data->thread_id;
   log_info("Thread id: %d, priority: [%s], associated client socket fd: %d",
            tid,
            (data->priority)?"HIGH":"LOW",
            data->cli_sock_fd);

   bzero(buffer,MAX_BUFFER_LENGTH);
   n = read(data->cli_sock_fd,buffer,MAX_BUFFER_LENGTH-1);
   if (n < 0)
   {
      perror("ERROR reading from socket");
      //and cancel the operation
      free_resources(hostname, current_hostname, buffer, servinfo, data);
      pthread_exit(NULL);
   }

#ifdef DEBUG_MODE
   log_info("(C -> P )Client sends data:");
   if (!omit)
      print_data(buffer, n, hex);
#endif

   /************* Perform the URL-based filtering ********************/
   mutex_lock();
   if ((ret = ub_url_permitted(buffer)) == -1)
   {
      log_info("URL is banned!");
      int nbytes;
      if ((nbytes = proxy_send_redirect(data->cli_sock_fd, URL_BASED)) != -1)
      {
         log_info("(C <== P)Success! Wrote %d bytes of HTTP redirection to client", nbytes);
         mutex_unlock();
      }
      else
      {
         log_error("Something wrong happened when writing");
         mutex_unlock();
      }
      free_resources(hostname, current_hostname, buffer, servinfo, data);
      //Exit thread
      pthread_exit(NULL);
   }
   else if (ret == -2) //an empty buffer was received
   {
      log_error("ERROR: an empty buffer was received");
      free_resources(hostname, current_hostname, buffer, servinfo, data);
      //Exit thread
      pthread_exit(NULL);
   }
   else
   {
      log_info("URL seems okay, forwarding request to server");
      mutex_unlock();
   }
   /******************************************************************/
   if ((ret = parse_hostname(hostname, buffer)) > 0)
   {
#ifdef DEBUG_MODE
      log_info("The hostname extracted is [%s]", hostname);
#endif
   }
   else if (ret == -1)
   {
      log_error("Error parsing hostname");
   }
   else if (ret == -2)
   {
      log_error("Not an HTTP GET");
      // CLose everything
   }

   // Update current hostname
   memcpy(current_hostname,hostname,strlen(hostname)+1);

   // http get
   if (ret == 1)
   {
      if (setup_host_get_connection(current_hostname, servinfo, &sockfdp) != 0 )
      {
         log_error("Error[1] connecting to the host [%s]  HTTP GET", hostname);
         free_resources(hostname, current_hostname, buffer, servinfo, data);
         pthread_exit(NULL);
      }
      connection_type = 1;
      log_error("Set http get connection");
   } else {
      if (setup_host_connect_connection(current_hostname, servinfo, &sockfdp) != 0 )
      {
         log_error("Error[1] connecting to the host [%s]   HTTP CONNECT", hostname);
         free_resources(hostname, current_hostname, buffer, servinfo, data);
         pthread_exit(NULL);
      }	
      // tell client
      connection_type = 2;

      //Tell client that connection established
      if ((ret = write(data->cli_sock_fd, "HTTP/1.1 200 Connection established\r\n\r\n", 39)) < 0)
      {
         perror("Write");
      }
#ifdef DEBUG_MODE
      else
      {
         log_info("(C <-- P)Proxy forwarded data back to client (%d bytes)", ret);
      }
#endif

      log_error("Set http connect connection");
      // TODO clean this up
      bzero(buffer,MAX_BUFFER_LENGTH);
      n = 0;
   }

   /*
     Write the request to the server
   */
   if ((n = write(sockfdp, buffer, n)) < 0 )
   {
      perror("Write");
   }
#ifdef DEBUG_MODE
   else
   {
      log_info("(P --> S)Proxy forwards data (%d bytes)", n);
   }
#endif

   // Set info for the polling
   ufds[0].fd = data->cli_sock_fd;
   ufds[0].events = POLLIN;

   ufds[1].fd = sockfdp;
   ufds[1].events = POLLIN;

   while (1)
   {
      /*
        Wait for data from either client or server
      */
      int rv = poll(ufds, 2, 0.0001);

      if (rv == -1) {
         perror("poll"); // error occurred in poll()
      } 
      else if (rv != 0) 
      {
         // check for events on client side:
         if (ufds[0].revents & POLLIN) {
            /*
              Read response from server after cleaning sending buffer
            */
            memset(buffer, 0, MAX_BUFFER_LENGTH);

            n = read(data->cli_sock_fd, buffer, MAX_BUFFER_LENGTH);
            if (n < 0)
            {
               perror("Error reading from socket");
            }
            else if (n == 0)
            {
               log_error("WARNING: Client closed the connection");
               //close proxy-server socket
               close(sockfdp);
               break;
            }
            else
            {
#ifdef DEBUG_MODE
               log_info("(P <-- S)Received %d bytes from client:", n);
#endif

               // Check if host name has changed
               if ((ret = parse_hostname(hostname, buffer)) > 0)
               {
                  if (strcmp(hostname, current_hostname) != 0 || ret != connection_type)
                  {
                     // hostname has changed
                     // TODO maybe close the existing connection???
#ifdef DEBUG_MODE
                     log_info("The hostname extracted is [%s] and current hostname is [%s]",
                              hostname, current_hostname);
#endif
                     // Update the current hostname
                     memset(current_hostname, 0, 200);
                     
                     memcpy(current_hostname,hostname,strlen(hostname)+1);
                     log_info("New hostname is [%s]", current_hostname);
                     if (ret == 1)
                     {
                        if (setup_host_get_connection(current_hostname, servinfo, &sockfdp) != 0 )
                        {
                           log_error("Error[1] connecting to the host [%s] HTTP GET", hostname);
                           free_resources(hostname, current_hostname, buffer, servinfo, data);
                           pthread_exit(NULL);
                        }
                        connection_type = 1;
                        log_error("Set http get connection");
                     } else {
                        if (setup_host_connect_connection(current_hostname, servinfo, &sockfdp) != 0 )
                        {
                           log_error("Error[1] connecting to the host [%s] HTTP CONNECT", hostname);
                           free_resources(hostname, current_hostname, buffer, servinfo, data);
                           pthread_exit(NULL);
                        }	

                        // tell client
                        connection_type = 2;

                        //Tell client that connection established
				  
                        if ((ret = write(data->cli_sock_fd,
                                         "HTTP/1.1 200 Connection established\r\n\r\n", 39)) < 0)
                        {
                           perror("Write");
                        }
#ifdef DEBUG_MODE
                        else
                        {
                           log_info("(C <-- P)Proxy forwarded data back to client (%d bytes)", ret);
                        }
#endif
                        log_error("Set http connect connection");
                        bzero(buffer,MAX_BUFFER_LENGTH);
                        n = 0;
                     }
             
                     ufds[1].fd = sockfdp;
                     ufds[1].events = POLLIN;
                  }
               }


               //Send it to server
               if ((ret = write(sockfdp, buffer, n)) < 0)
               {
                  perror("Write");
               }
#ifdef DEBUG_MODE
               else
               {
                  log_info("(C <-- P)Proxy forwarded data to server (%d bytes)", ret);
               }
#endif
            }        
         }

         // check for events on server side:
         if (ufds[1].revents & POLLIN) {
            /*
              Read response from server after cleaning sending buffer
            */
            memset(buffer, 0, MAX_BUFFER_LENGTH);

            n = read(sockfdp, buffer, MAX_BUFFER_LENGTH);
            if (n < 0)
            {
               perror("Error reading from socket");
            }
            else if (n == 0)
            {
               log_error("WARNING: Server closed the connection");
               //close proxy-server socket
               close(sockfdp);
               break;
            }

            else
            {
#ifdef DEBUG_MODE
               log_info("(P <-- S)Received %d bytes from server:", n);
#endif
               // Perform content-based filtering
               if (cb_page_permitted(buffer) == -1)
               {
                  log_info("Page got a match in content. Sending redirect");
                  int nbytes = proxy_send_redirect(data->cli_sock_fd, CONTENT_BASED);
                  if (nbytes > 0)
                     log_info("(C <== P)Succes! Wrote %d bytes of redirect to client!", nbytes);
                  else
                     log_error("Could not write redirect to client");

                  //Free resources and exit
                  free_resources(hostname, current_hostname, buffer, servinfo, data);
                  pthread_exit(NULL);
               }
               else //send it otherwise to the client
               {
                  log_info("Page's contents seem okey, forwarding to client");
                  //Send it back to the client
                  if ((ret = write(data->cli_sock_fd, buffer, n)) < 0)
                  {
                     perror("Write");
                  }
#ifdef DEBUG_MODE
                  else
                  {
                     log_info("(C <-- P)Proxy forwarded data back to client (%d bytes)", ret);
                  }
#endif
               }
            }
         }

      }
   } //while

   //free allocated resources
   free_resources(hostname, current_hostname, buffer, servinfo, data);
   pthread_exit(NULL);
}

void free_resources(char* hostname,
                    char* current_hostname,
                    char* buffer,
                    struct addrinfo* serv,
                    struct thread_data* ptr)
{
   free(hostname);
   free(current_hostname);
   memset(buffer, 0, MAX_BUFFER_LENGTH);
   freeaddrinfo(serv);
   log_info("Terminating thread-ID %d", ptr->thread_id);
   mutex_lock();
   --nr_active_threads;
   reset_thread_struct(ptr);
   mutex_unlock();
}

void sig_handler(int signo)
{
   if (signo == SIGINT)
   {
      printf("\nReceived SIGINT, Exiting program ...\n");
      if (sockfd >= 0) close(sockfd);
      if (newsockfd >= 0) close(newsockfd);

      //And exit program
      exit(0);
   }
   else if (signo == SIGPIPE)
   {
      fprintf(stderr, "CAUGHT SIGPIPE\n");
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
   //Allocate memory for the threads
   threads = malloc(MAX_SIM_REQUESTS * sizeof(pthread_t));

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
   close(ptr->cli_sock_fd);
   ptr->cli_sock_fd = DEAD;
}


int setup_host_get_connection(char *hostname,  struct addrinfo *servinfo, int *sockfdp){
   struct addrinfo hints, *p;
   int ret;

   //Init addrinfo struct
   memset(&hints, 0, sizeof(hints));
   hints.ai_family = AF_INET; // IPv4
   hints.ai_socktype = SOCK_STREAM; //TCP

   //Get address info of the parsed hostname
   if ((ret = getaddrinfo(hostname, "http", &hints, &servinfo)) != 0)
   {
      return -1;
   }

   // loop through all the results and connect to the first we can
   for(p = servinfo; p != NULL; p = p->ai_next) 
   {
      if ((*sockfdp = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) 
      {
         perror("socket");
         return -1;
         //continue;
      }

      if (connect(*sockfdp, p->ai_addr, p->ai_addrlen) == -1) 
      {
         perror("connect");
         //close(sockfdp);
         return -1;
         //break;
      }

      break; // if we get here, we must have connected successfully
   }

   if (p == NULL) 
   {
      // looped off the end of the list with no connection
      perror("failed to connect");
      return -1;
   }

   return 0;
}

int setup_host_connect_connection(char *hostname,  struct addrinfo *servinfo, int *sockfdp){
   struct addrinfo hints, *p;
   int ret;

   //Init addrinfo struct
   memset(&hints, 0, sizeof(hints));
   hints.ai_family = AF_INET; // IPv4
   hints.ai_socktype = SOCK_STREAM; //TCP

   //Get address info of the parsed hostname
   if ((ret = getaddrinfo(hostname, "https", &hints, &servinfo)) != 0)
   {
      return -1;
   }

   // loop through all the results and connect to the first we can
   for(p = servinfo; p != NULL; p = p->ai_next) 
   {
      if ((*sockfdp = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) 
      {
         perror("socket");
         return -1;
         //continue;
      }

      if (connect(*sockfdp, p->ai_addr, p->ai_addrlen) == -1) 
      {
         perror("connect");
         //close(sockfdp);
         return -1;
         //break;
      }

      break; // if we get here, we must have connected successfully
   }

   if (p == NULL) 
   {
      // looped off the end of the list with no connection
      perror("failed to connect");
      return -1;
   }

   return 0;
}
