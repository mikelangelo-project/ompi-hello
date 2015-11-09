#include <stdio.h>
#include <mpi.h>

#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#define ARRLEN(aa) (sizeof(aa)/sizeof(aa[0]))

/* 
 * Load OSv .so file immediately
 */
asm(".pushsection .note.osv-mlock, \"a\"; .long 0, 0, 0; .popsection");

#define DATASZ 8
long data[DATASZ] = {0};
int size, rank;

#define STDO stderr

void print_data(int rank) {
  int delay;
  delay = (rank % 2)*500000,
  usleep(delay);
  int ii;
  fprintf(STDO, "(rank=% 2d) ", rank);
  for(ii=0; ii<DATASZ; ii++) {
    fprintf(STDO, "%04d ", (int)data[ii]);
  }
  fprintf(STDO, "\n");
}

void print_env(char **env) {
  char **ch;
  int ii=0;
  fprintf(stderr, "ENV /*--------------------------\n");
  for(ch=env; *ch != NULL; ch++, ii++) {
    fprintf(stderr, "ENV %02d %s\n", ii, *ch);
  }
  fprintf(stderr, "ENV --------------------------*/\n");
}

void do_send_recv() {
  int rank_to, rank_from;
  rank_to = rank==size-1? 0: rank + 1;
  rank_from = rank==0? size-1: rank - 1;
  fprintf(STDO, "rank from=%d me=%d to=%d\n", rank_from, rank, rank_to);

  if(rank == 0) {
    data[0] = 1000;
    fprintf(STDO, "Send %d -> %d\n", rank, rank_to);
    print_data(rank);
    MPI_Send(data, ARRLEN(data), MPI_LONG, rank_to, 123, MPI_COMM_WORLD);
  }
  
  fprintf(STDO, "Recv %d -> %d\n", rank_from, rank);
  MPI_Recv(data, ARRLEN(data), MPI_LONG, rank_from, 123, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  print_data(rank);

  // update data
  int ii0, iim1, iip1;
  ii0 = rank % DATASZ; // my slot
  iim1 = ii0==0? DATASZ-1: ii0 - 1; // previous slot
  data[ii0] = data[iim1] + 1;

  if(rank != 0) {
    fprintf(STDO, "Send %d -> %d\n", rank, rank_to);
    MPI_Send(data, ARRLEN(data), MPI_LONG, rank_to, 123, MPI_COMM_WORLD);
    print_data(rank);
  }

}

void do_dbg_wait() {
  fprintf(stderr, "In do_dbg_wait\n");
  sleep(3);
#if 0
  fprintf(stderr, "sbrk test\n");
  void* ret = sbrk(0);
  fprintf(stderr, "sbrk ret %p\n", ret);
#endif
  fprintf(stderr, "do_dbg_wait DONE\n");
}

int socket_ping(int argc, char *argv[]) {
  int sockfd, portno, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;
   
  char buffer[256];
   
  if (argc < 3) {
    fprintf(stderr,"usage %s hostname port\n", argv[0]);
    return 0;
  }
  fprintf(stderr, "Connecting to %s:%s\n", argv[1], argv[2]);
  
  portno = atoi(argv[2]);
   
  /* Create a socket point */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
  if (sockfd < 0) {
    perror("ERROR opening socket");
    return 1;
  }
        
  server = gethostbyname(argv[1]);
  
  if (server == NULL) {
    fprintf(stderr,"ERROR, no such host\n");
    return 1;
  }
   
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(portno);
   
  /* Now connect to the server */
  if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("ERROR connecting");
    return 1;
  }
   
  strncpy(buffer, "Some test msg\n", 256);
   
  /* Send message to the server */
  n = write(sockfd, buffer, strlen(buffer));
   
  if (n < 0) {
    perror("ERROR writing to socket");
    return 1;
  }
   
  /* Now read server response */
  bzero(buffer,256);
  n = read(sockfd, buffer, 255);
   
  if (n < 0) {
    perror("ERROR reading from socket");
    return 1;
  }
        
  printf("%s\n",buffer);
  return 0;
}

//int main(int argc, char **argv, char **env)
extern char **environ;
int main(int argc, char **argv)
{
  printf("In mpi_hello.c main.\n");
  printf(" argc %d\n", argc);
  int ii;
  for(ii=0; ii<argc; ii++) {
    printf(" argv[%d] = %s\n", ii, argv[ii]);
  }
  print_env(environ);
  
  //do_dbg_wait();
  //socket_ping(argc, argv);
  
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  printf("SIZE = %d RANK = %d\n",size,rank);

  do_send_recv();

  MPI_Finalize();   
  printf("Sleep...\n");
  sleep(2);
  printf("Done\n");
   
  printf("main EXIT\n");
  return(0);
}
