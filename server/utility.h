#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#define BACKLOG 10
#define MAXDATASIZE 4096

void sendname(int sockfd, char * filename_ptr); // the client protocol, send length of filename, and loop thru sending filename
int recfile(int sockfd, int fd); // receive length of file in uint32_t, and loop through receiving from socket
int transferfile(int sockfd, int fd); // send length of file in uint_32t and then loop through writing to socket
void getName (int sockfd, char *); // the server side get name protocol, receive length of name, and loop until the whole name is received
int fileExists (char * filename); //returns 1 if file exists, otherwise 0
int getCode(int sockfd); // receive a 16 bit code from the otherside of this connection
void sendCode(int code, int sockfd); // send a 16 bit code from the otherside of this connectio
void getUnusedFileName(char *);  // finds an unused file to temporary store the pipe of "ls" so that it can be transferred using the same endpoints as "Transferfile" and "recfile"
int directoryExists(char *); // sees if a directory exists, returning 0 if so
void minSlashes (char *path);  // For when the user CHD's .. - this minifies hte length of the past so its not wasting space
void updatePath (char *path, char * newPath); // helper function for minSlashes
void removeFilename (char *path); // this is a convenient subroutine that removes a filename from a path. It is here, because the path is necessary to know what dir we are in (when the user cd's), and the file completes the path. We need to be able to remove the filename so the path still represents the path
void sendHash(int sockfd, int fp); // sends the hash of a filein
void getHash(int sockfd, char *); // essentially "receive hash" uses "get name" function defined earlier to receive a hash computed on the other end
void calculate_hash(char*, char*); // calculates hash code
void sendCodeUDP (int code, int sockfd, struct sockaddr_storage* src_addr, socklen_t);
int getCodeUDP(int sockfd);
void sendStringUDP (char* string, int udpsockfd, struct sockaddr_storage* src_addr, socklen_t src_addr_len);
void getStringUDP (int udpsockfd, char* string,  int maxlen);
