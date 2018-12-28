#include "pipe_networking.h"

static void sighandler(int signo){
  // Catches the SIGUSR1 signal

	if (signo == SIGINT){
		//Before exiting, append a message to a file noting that the program exited due to SIGINT
		printf("\nExited process %d due to SIGINT \n", getpid());
		remove("wkp");
		//	remove(client_pipe);
		// char message[] =  "The program ended due to SIGINT\n";
		// int w = write(fd, message, sizeof(message));
		// close(fd);
		exit(0);
	}
}

/*=========================
  server_handshake
  args: int * to_client
  Performs the client side pipe 3 way handshake.
  Sets *to_client to the file descriptor to the downstream pipe.
  returns the file descriptor for the upstream pipe.
  =========================*/
int server_handshake(int *to_client) {
  signal(SIGINT, sighandler);
  char buffer[HANDSHAKE_BUFFER_SIZE];
  if ( mkfifo("wkp", 0644) < 0 ) {
    printf("[SERVER] Error %d: %s\n", errno, strerror(errno));
    exit(0);
  }
  printf("[SERVER] Congrats! The server pipe was just created.\n");
  printf("[SERVER] Waiting for a connection...\n");
  int from_client = open("wkp", O_RDONLY);
  printf("[SERVER] Connecting to a client!\n");
  memset(buffer, 0, sizeof(buffer));
  read(from_client, buffer, sizeof(buffer));
  printf("[SERVER] Connecting to pipe %s (from client!) \n", buffer);

  *to_client = open(buffer, O_WRONLY);
  strcpy(buffer, ACK);
  write(*to_client, buffer, sizeof(buffer));
  read(from_client, buffer, sizeof(buffer));
  printf("[SERVER] Houston. We have a connection.\n");
  remove("wkp");
  return from_client;

}


/*=========================
  client_handshake
  args: int * to_server
  Performs the client side pipe 3 way handshake.
  Sets *to_server to the file descriptor for the upstream pipe.
  returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {
  char buffer[HANDSHAKE_BUFFER_SIZE];
  char client_pipe[256];
  sprintf(client_pipe,"%d",getpid());
  strcpy(buffer,client_pipe);
  if ( mkfifo(client_pipe, 0644) < 0 ) {
    printf("[CLIENT] Error %d: %s\n", errno, strerror(errno));
    exit(1);
  }
  printf("[CLIENT] Congrats! A pipe was created! Name: %s\n", client_pipe);

  *to_server = open("wkp", O_WRONLY);

  write(*to_server, buffer, sizeof(buffer));
  if (*to_server < 0){
    printf("[CLIENT] Error %d: %s\n", errno, strerror(errno));
    if (errno == 9){
      printf("[CLIENT] There is likely no server open.\n" );
    }
    remove(client_pipe);
    exit(1);
  }

  printf("[CLIENT] A connection was just established with the server!\n");


  int from_server = open(client_pipe, O_RDONLY);

  read(from_server, buffer, sizeof(buffer));
  printf("[CLIENT] Server sent a message: %s\n", buffer);

  remove(client_pipe);
  strcpy(buffer, ACK);
  write(*to_server, buffer, sizeof(buffer));

  return from_server;
}
