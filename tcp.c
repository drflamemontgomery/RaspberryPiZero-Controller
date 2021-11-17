#include <stdio.h>
#include <stdbool.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define MAX 8 // Report Descriptor Length in Bytes
#define PORT 8080 // TCP Port
#define SA struct sockaddr

typedef struct { 
  int sockfd;
  int id;
  char file[12];
} data_t; // allows us to pass data easily to functions

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
unsigned char empty[8] = {
  0x00, 0x00, 0x08, 0x80, 0x80, 0x80, 0x80, 0x80
};
// Controller With released Buttons, Dpad, and Sticks

// Writes Report to /dev/hidgX
void send_report(unsigned char* data, int fd) {
  write(fd, data, 8);
  fdatasync(fd);
}

// Function designed for chat between client and server.
// Used in a thread
void * func(void *arg)
{
  // Contains file info, TCP Socket info, and Client ID
  data_t* args = (data_t *)(arg);
  printf("Client %d\n", args->id);
  unsigned char buf[MAX]; // Report Descriptor of 8 bytes
  
  // Open and prepare /dev/hidgX
  int fd = open(args->file, O_RDWR);
  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(fd, &rfds);
  
  
  // forever loop for chat
  for (;;) {
    // Recieve Report Descriptor
    read(args->sockfd, buf, 8);
    
    //Lock pthread to prevent multi-access
    pthread_mutex_lock(&lock);
    
    bool sbreak = true;
    
    int i;
    for(i = 0; i < 8; i++) {
      // Is the client about to close the socket
      // close report descriptor = 0xFFFFFFFFFFFFFFFF
      // If there is a none 0xff byte we are still receiving
      if(buf[i] != 0xff) {
        sbreak = false;
        break;
      }
    }
    
    // If the client is going to close the socket
    // unlock pthread so the other threads can run
    // and exit forever loop
    if(sbreak) {
      pthread_mutex_unlock(&lock);
      break;
    }
    
    // push report_descriptor to /dev/hidgX
    send_report(buf, fd);
    
    // Allow other threads to run
    pthread_mutex_unlock(&lock);
  }
  
  // Close socket with client
  close(args->sockfd);
  
  // Reset Controller for USB Host
  send_report(empty, fd);
  
  // Close /dev/hidgX
  close(fd);
  
  // Allow a new Client to use this controller
  args->sockfd = -1;
  
  // Exit the Thread
  pthread_exit(NULL);
}

// Main function
int main()
{
  int sockfd, connfd, len;
  data_t data[4];
  
  // Allow all data sockets to be assigned
  data[0].sockfd = -1;
  data[1].sockfd = -1;
  data[2].sockfd = -1;
  data[3].sockfd = -1;
  
  // Keep track of all socket threads with the clients
  pthread_t clients[4];
  struct sockaddr_in servaddr, cli;
  int optval;
   
  // socket create and verification
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    printf("socket creation failed...\n");
    exit(0);
  }
  optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));
  bzero(&servaddr, sizeof(servaddr));
   
  // assign IP, PORT
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr("0.0.0.0");
  servaddr.sin_port = htons(PORT);
   
  // Binding newly created socket to given IP and verification
  if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
    printf("socket bind failed...\n");
    exit(0);
  }
   
  // Now server is ready to listen and verification
  if ((listen(sockfd, 4)) != 0) {
    printf("Listen failed...\n");
    exit(0);
  }
  len = sizeof(cli);

  int id;
  // Accept the data packet from client and verification
  while(1) {
    connfd = accept(sockfd, (SA*)&cli, &len);
    if (connfd < 0) {
      printf("server accept failed...\n");
      exit(0);
    }

    char full = 1;
    for(id = 0; id < 4; id++) {
      // Make sure not to overwrite a clients socket
      if(data[id].sockfd == -1) {
        full = 0;
	data[id].sockfd = connfd;
	data[id].id = id;
	sprintf(data[id].file, "/dev/hidg%d", id);
	pthread_create(&clients[id], NULL, func, &data[id]);
        break;
      }
    }

    // If there a no more client spaces
    // Close Connection with incoming Client
    if(full) {
      close(connfd);
    }
  }

  for(id = 0; id < 4; id++) {
    pthread_join(clients[id], NULL);
  }
  // Close Server Socket
  close(sockfd);
  return 0;
}
