#include "utility.h"
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



void sendname (int sockfd, char *filename_ptr) {
        uint16_t fileNameLength = strlen(filename_ptr);
        uint16_t fileNameLengthN = htons(fileNameLength);
        if (write(sockfd, &fileNameLengthN, sizeof(fileNameLengthN)) == -1)
                      perror("send");
	// get address
	int add = *filename_ptr;
        int maxline = 5; int totalSent=0; int i = 0;
        char tempBuf[10];
        while (fileNameLength > 0) {
                maxline = strlen(filename_ptr) < maxline ? strlen(filename_ptr): maxline;
                if (write(sockfd, filename_ptr, maxline*sizeof(char)) == -1)
                      perror("send");
		//move down filename_ptr
                for (i=0; i< maxline; i++) {
			++filename_ptr;
		}
		fileNameLength-=maxline;
                totalSent+=maxline;
                // move ptr down the word

        }
        for (i=0; i<totalSent; i++) --filename_ptr; //reset filename ptr
}
// the sock file descriptor as well as the pointer to the filename, which we will
//set in the function
void  getName (int sockfd, char * filename_ptr) {
        int numbytes;
        uint16_t fileNameLength;
        if ((numbytes = recv(sockfd, &fileNameLength, sizeof(fileNameLength), 0)) == -1) {
                    perror("recv");
                    exit(1); }
        int fileLength = ntohs(fileNameLength);
        int maxline = 5;
        char catBuf[maxline];
        char filenameF[200];
        memset(filenameF, '\0', strlen(filenameF));
  //      printf("the length of the filename is %d\n", fileLength);
	int totalBytes = 0;
        char tempBuf[10];
        int maxChunk=5;

        while (fileLength > 0) {
		maxChunk = maxChunk > fileLength ? fileLength : maxChunk;
                memset(tempBuf, '\0', sizeof(tempBuf));
		if ((numbytes = recv(sockfd, tempBuf, maxChunk*sizeof(char), 0)) == -1)
                      perror("rcv");
                tempBuf[numbytes] = '\0';
		strcat(filenameF, tempBuf);
                fileLength-=numbytes;
                totalBytes+= numbytes;
                // move ptr down the word
            }
        filenameF[totalBytes] = '\0';
	//filename_ptr=filenameF;
	strcpy(filename_ptr, filenameF);

	 // see if file exists:
	struct stat st;
        mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
}


int transferfile(int sockfd, int fp) {

        int maxline = 96;
        // number of characters from the file
        char buffer[maxline];
        // integers to keep track of how many characters we've move
        int readInt, writeInt;
        //readInt
        // remember endian-ness. Have to use same integer format, and convert to network protocol
        uint32_t network_byte_order;
        struct stat fileStat;
        //get length of file
        if(fstat(fp,&fileStat) < 0)
                fprintf(stderr, "Error reading file\n");
        network_byte_order = htonl((uint32_t) fileStat.st_size);
	if (write(sockfd, &network_byte_order, sizeof(uint32_t)) == -1)
                perror("send");
	int totalBits = 0;
        int bitsLeft = fileStat.st_size;
	while (1) {
		readInt = read (fp, buffer, maxline);
                if (readInt == 0) {
                        break;
                }
        // send this over
        if (write(sockfd, buffer, readInt) == -1) {
		perror("send");
                exit(1);
        }
        totalBits += readInt;
        }
	return totalBits;
}

int recfile (int sockfd, int fd2) {
        int totalbytes = 0;
        int maxline = 96;
        int numbytes;
        char buf[maxline];
        uint32_t num;
        int writeInt;
        // transfer into network byte order

        // Now receive the number
	if ((numbytes = recv(sockfd, &num, sizeof(num), 0)) == -1) {
                        perror("recv");
                        exit(1); }

        int l = ntohl(num);
        if (l<0) {
	//	pritnf
	}
	while (l > 0) {
		if ((numbytes = recv(sockfd, buf, maxline, 0)) == -1) {
                        perror("recv");
                        exit(1); }
                totalbytes += numbytes;
		l-= numbytes;
                writeInt = write (fd2, buf, numbytes);
                if (writeInt < 0) {
                        perror("writing file:");
                }

        }
        if (close (fd2) < 0) {
                perror("closing new file");
        }
	return totalbytes;
        //state of receiving file
}

int fileExists (char* filename) {
	FILE *fp = fopen(filename, "r");
	if (fp!=NULL) {
		fclose(fp);
		return 1;
	}
	else
		return 0;

}

int getCode(int sockfd) {
	uint32_t code16;
	int numbytes;
	if ((numbytes = recv(sockfd, &code16, sizeof(uint32_t), 0)) == -1) {
		perror("recv");
		exit(1); }
	return  ntohs(code16)-50; // the minus 50 is to return to negatives, since only an unsigned int would go over the line

}

void sendCode(int code, int sockfd) {
	uint32_t code16 = code + 50; // the plus 50 is to offset negatives, since we only send unsigned ints
	code16 = htons(code16);

	if (write(sockfd, &code16, sizeof(uint32_t)) == -1)
                                              perror("send");

	// now send

}

void sendCodeUDP (int code, int udpsockfd, struct sockaddr_storage* src_addr, socklen_t src_addr_len) {
  uint32_t code16 = code + 50; // the plus 50 is to offset negatives, since we only send unsigned ints
  code16 = htons(code16);
  if (sendto(udpsockfd, &code16, sizeof(uint32_t), 0, (struct sockaddr*)src_addr, src_addr_len) == -1)
    perror("send");
}

void sendStringUDP (char* string_to_send, int udpsockfd, struct sockaddr_storage* src_addr, socklen_t src_addr_len) {
  if (sendto(udpsockfd, string_to_send, strlen(string_to_send), 0, (struct sockaddr*)src_addr, src_addr_len) == -1)
    perror("send");
}

void getStringUDP (int udpsockfd, char * string, int maxlen) {
  struct sockaddr_storage src_addr;
  socklen_t src_addr_len=sizeof(src_addr);
  int numbytes;
  if ((numbytes=recvfrom(udpsockfd, string, maxlen, 0, (struct sockaddr*)&src_addr, &src_addr_len)) == -1)
  	perror ("get password");
  string[numbytes] = '\0';

}

int getCodeUDP (int sockfd) {
  uint32_t code16;
  int numbytes;
  struct sockaddr_storage src_addr;   // wil store the address for UDP, important to continue
	socklen_t src_addr_len=sizeof(src_addr);  //passing these two parameters
  if ((numbytes = recvfrom(sockfd, &code16, sizeof(uint32_t), 0, (struct sockaddr*)&src_addr, &src_addr_len)) == -1) {
		perror("recv");
		exit(1); }
  return  ntohs(code16)-50;

}



// used for ls, since I am piping the contents into a file and using
// the api for my transfer function. Need to make sure we pick a file
// that is not already existent
void getUnusedFileName (char * basename) {
	char ending[3];
	strcpy(basename, "lsfile");
	int i =0;
	while (fileExists(basename)==1) {
		sprintf(ending, "%d", i); // increment the number
		strcat (basename, ending);
		i++;
	}

}
// see if a directory exists already
int directoryExists(char * directory) {
	DIR* dir = opendir(directory);
	if (dir) {
		closedir(dir);
		return 0; // directory exists
	}
	else if (ENOENT == errno) {
		return 1; //directory does not exists
	}
	else
		return 2; //some other errro


}


void updatePath (char *path, char * newPath) {
        if (strcmp(newPath, "..")==0) {
                // need to see if we can simply
                //remove the last part ofthe path with
                // util function, or if we need to actually append
                // ".."
                minSlashes(path);
        }
        else {
                strcat(path, newPath);
                strcat(path, "/");

        }
}

void minSlashes (char *path) {
        // starting length will be 2 since "./
	 if (strlen (path) > 1) {
                //make sure second to last character is not . IF it is we can't remove anything. If it's not, we can simply remove up until the slash
                if (path[strlen(path)-2] != '.') {
                        int i = strlen(path)-2;
                        path[strlen(path)-1]='\0';
                        while (path[i] != '/') {
                                path[i]='\0'; // reset the end part of the string
                                i--;
                        }
                }
                else strcat(path, "../"); // we must add to the length otherwise
        }
}

void removeFilename (char *path) {
        if (strlen (path) > 2) {
                int i = strlen(path);
                i--; // 0 indexed friendly
                path[i] = '\0'; //remove the trailing slash
                i--; // get second to last index
                while (path[i] != '/') {
                        path[i] = '\0';
                        i--;
                }
        }



}
