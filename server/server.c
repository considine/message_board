// message board server
//   John Considine
//     jconsidi
//       November 11, 2016

#include "utility.h"
int set_socket(char* arg, int, struct addrinfo **p);
int read_line(int fd, char* block);
int check_users(int fd, char * req_user);
int check_pass(int, int, char *);
int client_loop (char * arg1, int udpfd, int tcpfd);
int op_loop(int tcpsockfd, int udpsockfd, struct sockaddr_storage* src_addr, socklen_t src_addr_len, char* username);
int sign_in();



int main (int argc, char *argv[]) {
	if (argc != 2) {
		printf("please enter prot name as argument 1\n");
		exit(1);
	}
	struct addrinfo *p;
	int udpfd = set_socket(argv[1], 1, &p);
	int tcpfd = set_socket(arg1, 0, &p);

	int cont = 1;
	while (cont == 1) {
		cont = client_loop(argv[1], udpfd, tcpfd);
	}

	return 0;
}


int client_loop (char * arg1, int udpfd, int tcpfd) {

	//int tcpfd = 1;

	char username[30];
	int numbytes;

	struct sockaddr_storage src_addr;
  socklen_t src_addr_len=sizeof(src_addr);

	//GET THE USER SIGNED IN!!!!
	int succ = 0;
	int fd;
	while (succ==0) {
		printf ("from the top\n");
		memset(username, strlen(username), '\0');
		if ((numbytes = recvfrom(udpfd, username, 100, 0,  (struct sockaddr*)&src_addr, &src_addr_len)) == -1) {

			perror("recv");
			exit(1);
		}
		username[numbytes] = '\0';
		printf("received\n");
		//add username to file
		// first make sure there is a username file
		if (fileExists(".user_file")==0) {
			printf("it does not exist\n");
			// create the file
			system("touch .user_file");
		}

		// open file
		fd = open(".user_file", O_RDONLY, 0);
		int userexist=check_users(fd, username);
		char password[40];
		if (userexist!=0) {
			// check password
			// tell client this user exists
			sendCodeUDP(1, udpfd, &src_addr, src_addr_len);
			//get password

			if ((numbytes=recvfrom(udpfd, password, 40, 0, (struct sockaddr*)&src_addr, &src_addr_len)) == -1)
				perror ("get password");
			succ = check_pass(fd, userexist, password);
			sendCodeUDP(succ, udpfd, &src_addr, src_addr_len);
			printf("the password is %d\n", succ);
			//sendCodeUDP(30, udpfd, &src_addr, src_addr_len);
			//printf("the password is %d\n", check_pass(fd, userexist, password));
		}
		else {
			//TODO send negative code
			sendCodeUDP(1, udpfd, &src_addr, src_addr_len);
			// if ((numbytes=recvfrom(udpfd, password, 40, 0, (struct sockaddr*)&src_addr, &src_addr_len)) == -1)
			// 	perror ("get password");
			getStringUDP(udpfd, password, 40);
			printf("password is %s\n", password);
			//concatenate to the account
			char tempCommand[20 + strlen(username) + strlen(password)];
			strcpy(tempCommand, "echo ");
			strcat(tempCommand, username);
			strcat(tempCommand, " >> ");
			strcat(tempCommand, " .user_file; ");
			strcat(tempCommand, "echo ");
			strcat(tempCommand, password);
			strcat(tempCommand, " >> ");
			strcat(tempCommand, ".user_file");
			system(tempCommand);
			succ = 1;
			sendCodeUDP(succ, udpfd, &src_addr, src_addr_len);
		}

		close(fd);
	}

	// GET OPERATION state

	return op_loop(tcpfd, udpfd, &src_addr, src_addr_len, username);
	// return 0;

}


// 	return 0;
// }

int op_loop(int tcpsockfd, int udpsockfd, struct sockaddr_storage* src_addr, socklen_t src_addr_len, char* username) {
	char op[10];
	while (1) {
		memset(op, '\0', sizeof op);
		getStringUDP(udpsockfd, op, 5);
		if (strcmp(op, "CRT")==0) {
			char board[30];
			getStringUDP(udpsockfd, board, 30);
			printf("the board is %s\n", board);

			// see if the board exists
			char boardPath[strlen(board)+10];
			strcpy(boardPath, "boards/");
			strcat(boardPath, board);

			if (fileExists(boardPath)==1) {
				sendCodeUDP(-1, udpsockfd, src_addr, src_addr_len);
			}
			else {
				char tempCommand[strlen(boardPath) + strlen(username) + 10];
				strcpy(tempCommand, "echo ");
				strcat(tempCommand, username);
				strcat(tempCommand, " >> ");
				strcat(tempCommand, boardPath);
				int res = system(tempCommand);
				sendCodeUDP(1, udpsockfd, src_addr, src_addr_len);
			}
		}
		else if (strcmp(op, "LIS")==0) {

		}else if (strcmp(op, "MSG")==0) {

		}else if (strcmp(op, "DLT")==0) {

		}else if (strcmp(op, "RDB")==0) {

		}else if (strcmp(op, "APN")==0) {

		}else if (strcmp(op, "DWN")==0) {

		}else if (strcmp(op, "DST")==0) {

		}else if (strcmp(op, "XIT")==0) {
			close(tcpsockfd);
			return 1;
		}else if (strcmp(op, "SHT")==0) {
			close(tcpsockfd);
			close(udpsockfd);
			return 0;
		}
	}
}



int check_pass(int fd, int line, char* pass) {
	char password[40];
	//reset fd
	lseek(fd, 0, SEEK_SET);
	int i=0;
	while (1) {

		int brea = read_line(fd, password);
		if (i==line)
		{
			break;
		}
		memset(password, '\0', sizeof password);
		i++;
	}
	if (strcmp(password, pass)==0)
		return 1;
	else return 0;


}

int check_users(int fd, char* req_user) {
	printf("req user is %s\n", req_user);
	int i = 0;
	char username[40];
	while (1) {
	if ((i%2)==0) {
		int brea= read_line(fd, username);
		printf("userline is %s\n", username);
		if (brea==0)
			break;
		if (strcmp(username, req_user)==0) {
			memset(username, '\0', sizeof username);
				return i+1; // so it doesnt return 0, default up one

		}
		memset(username, '\0', sizeof username);
	}
	else {
		//skip line
		int brea= read_line(fd, username);
		memset(username, '\0', sizeof username);
		if (brea==0)
			break;
	}
	i++;

	}
	return 0;
}

int read_line(int fd, char* block) {
	memset(block, '\0', strlen(block));
	int readInt; // assigns number of characters read from a file
	int maxchar = 1; // only go one character at a time
	char buffer[2];
	while (1) {
		readInt = read(fd, buffer, maxchar);
		buffer[1] = '\0';
		if (readInt==0)
			return 0;
		if (strcmp(buffer, "\n")==0) {
			return 1;
		}
		strcat(block, buffer);
	}
	printf("line is %s\n", block);

}


int set_socket(char* arg, int tcp, struct addrinfo **p_ptr) { // tcp = 0, udp = 1
	int yes=1;

        // SET UP TCP SOCKET
        int sockfd, new_fd; // both socket descriptors
        struct addrinfo hints, *servinfo, *p;
        unsigned int addr_len;

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
	if (tcp ==0)
		hints.ai_socktype = SOCK_STREAM;
        else if (tcp == 1)
		hints.ai_socktype = SOCK_DGRAM;
	else
		return -1; // some sort of error
	hints.ai_flags = AI_PASSIVE;
        int rv;

        if ((rv = getaddrinfo(NULL, arg, &hints, &servinfo)) != 0) {
                fprintf(stderr, "Error setting structs for socket info\n");
        }

        for (p=servinfo; p!=NULL; p=p->ai_next) {
                if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))== -1) {
                        perror ("error creating socket\n");
                        continue;
                }
                //make the port reusable
                if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))==-1) {
                        perror ("setsockopt");
                        exit(1);
                }
                // bind the socket
                if (bind(sockfd, p->ai_addr, p->ai_addrlen)==-1) {
                        close(sockfd);
												printf(" we are using %d\n", tcp);
                        perror("error binding to socket");
                        continue;
                }
                break;
        }
        if (p==NULL) {
                fprintf(stderr, "Failed to bind to the socket\n");
                exit(1);
        }
	*p_ptr = p;
        freeaddrinfo(servinfo);
	printf("waiting for connections\n");
	if (tcp==1) {
		return sockfd;
	}
	if (listen(sockfd, BACKLOG) == -1) {
                perror ("listen");
                exit(1);
        }
        struct sockaddr_storage client_address;
        socklen_t sin_size = sizeof client_address;
        sockfd = accept(sockfd, (struct sockaddr *)&client_address, &sin_size);
	return sockfd;
}
