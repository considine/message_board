// message board server
//   John Considine
//     jconsidi
//       November 11, 2016

#include "utility.h"
int set_socket(char* arg, int, struct addrinfo **p);
int read_line(int fd, char* block);
int get_lines(char* filename);
int deleteLine(char* board, int line);
int check_users(int fd, char * req_user);
int check_pass(int, int, char *);
int client_loop (int udpfd, int tcpfd);
int extractLineNum(char* line);
void fixLines(char* filename);
void fixParens (char* block, int line);
int op_loop(int tcpsockfd, int udpsockfd, struct sockaddr_storage* src_addr, socklen_t src_addr_len, char* username);
int sign_in();
void appendLineNumber(char* message, int linenumber);




int main (int argc, char *argv[]) {
	if (argc != 2) {
		printf("please enter prot name as argument 1\n");
		exit(1);
	}
	struct addrinfo *p;
	int udpfd = set_socket(argv[1], 1, &p);
	int tcpfd = set_socket(argv[1], 0, &p);

	int cont = 1;
	while (cont == 1) {
		// listen for connection

    struct sockaddr_storage client_address;
    socklen_t sin_size = sizeof client_address;
    tcpfd = accept(tcpfd, (struct sockaddr *)&client_address, &sin_size);

		cont = client_loop(udpfd, tcpfd);
	}

	return 0;
}


int client_loop (int udpfd, int tcpfd) {

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
		memset(password, '\0', sizeof password);
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
			char tempCommand[40 + strlen(username) + strlen(password)];
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
	char message[50];
	char board[30];
	while (1) {
		memset(op, '\0', sizeof op);
		getStringUDP(udpsockfd, op, 5);
		if (strcmp(op, "CRT")==0) {
			memset(board, '\0', 30);
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
				//get board name
				memset(board, '\0', 30);
				getStringUDP(udpsockfd, board, 30);
				char boardPath[strlen(board)+10];
				memset(boardPath, '\0', sizeof boardPath);
				strcpy(boardPath, "boards/");
				strcat (boardPath, board);
				memset(message, '\0', sizeof message);
				getStringUDP(udpsockfd, message, 50);
				if (fileExists(boardPath)==1) {
					// printf("such board, and the message is %s\n", message);
					//get lines in board
					int lines = get_lines(boardPath);
					printf("there are %d lines\n", lines);
					char tempCommand[strlen(boardPath) + strlen(message) + 15];
					appendLineNumber(message, lines);

					memset(tempCommand, '\0', sizeof tempCommand);
					strcpy(tempCommand, "echo '");
					strcat(tempCommand, message);
					strcat(tempCommand, "' >> ");
					strcat(tempCommand, boardPath);
					int res = system(tempCommand);
					sendCodeUDP(res, udpsockfd, src_addr, src_addr_len);
				}
				else {
					// does not exists
					sendCodeUDP(-1, udpsockfd, src_addr, src_addr_len);
				}
				// see if it exists

		}else if (strcmp(op, "DLT")==0) {
			//get board name
			memset(board, '\0', 30);
			getStringUDP(udpsockfd, board, 30);

			char boardPath[strlen(board)+10];
			memset(boardPath, '\0', sizeof boardPath);
			strcpy(boardPath, "boards/");
			strcat (boardPath, board);
			if (fileExists(boardPath)==1) {
				sendCodeUDP(1, udpsockfd, src_addr, src_addr_len);
			}
			else {
				sendCodeUDP(-1, udpsockfd, src_addr, src_addr_len);
				continue;
			}
			//getLine to deleteLine
			int line = getCodeUDP(udpsockfd);
			printf("the line to delete is %d\n", line);
			int res = deleteLine(boardPath, line);
			if (res < 0)  {
				sendCodeUDP(-1, udpsockfd, src_addr, src_addr_len);
				continue;
			}// the linde doesnt exist
			else {
				sendCodeUDP(1, udpsockfd, src_addr, src_addr_len);
			}

		}else if (strcmp(op, "RDB")==0) {

		}else if (strcmp(op, "APN")==0) {

		}else if (strcmp(op, "DWN")==0) {

		}else if (strcmp(op, "DST")==0) {

		}else if (strcmp(op, "XIT")==0) {

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

int get_lines(char* filename) {
	int fd = open(filename, O_RDONLY, 0);
	int line_ct;
	char throwawayBlock[40];
	while (read_line(fd, throwawayBlock)!=0)
		line_ct++;
	return line_ct;
	close(fd);
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
	if (tcp==1)
		return sockfd;
	if (listen(sockfd, BACKLOG) == -1) {
		perror ("listen");
		exit(1);
	}
	return sockfd;


}

void appendLineNumber(char* message, int lines) {
	char * tempMessage = malloc (strlen(message)*sizeof(char) + 5 *sizeof(char));
	strcpy(tempMessage, message);
	sprintf(tempMessage, "%s (%d)", message, lines);
	memset(message, '\0', strlen(message));
	strcpy(message, tempMessage);
	free(tempMessage);
}


int deleteLine(char* board, int line) {
	//first maek temp file of same naem
	// get lines of board
	int total_lines = get_lines(board);
	if (line > total_lines) {
		return -1;
	}
	int fd = open(board, O_RDONLY, 0);
	char * block = malloc(50 * sizeof(char));
	char * tempCommand = malloc (30 * sizeof(char) + strlen(board) * sizeof(char));
	strcpy(tempCommand, "cp ");
	strcat(tempCommand, board);
	strcat(tempCommand, " .temp_board");

	system(tempCommand);

	int found = 0; // so we know if we found the line o rnot

	read_line(fd, block);
	// int line_track =0;
	// while (1) {
	// 	line_track++;
	// 	int brea= read_line(fd, block);
	// 	if (brea==0)
	// 		break;
	// 	if (extractLineNum(block)==line) {
	// 		found = 1;
	// 		break; // we've found the line
	// 	}
	// }
	// if (found == 0) {
	// 	//todo send message
	//
	// }



	//remove file... we will be rewriting to it
	memset(tempCommand, '\0', strlen(tempCommand));
	strcpy(tempCommand, "rm ");
	strcat(tempCommand, board);
	system(tempCommand); // remove the board;
	close (fd);
	int fd2 = open(".temp_board", O_RDONLY, 0);

	int cLine = 0;
	while (1) {
		int brea = read_line(fd2, block);
		if (brea==0) break;

		// set tempcommand
		if (cLine != line)  {
			memset(tempCommand, '\0', strlen(tempCommand));
			strcpy(tempCommand, "echo '");
			strcat(tempCommand, block);
			strcat(tempCommand, "' >> ");
			strcat(tempCommand, board);

			system(tempCommand);
		}

		cLine++;

	}


	memset(tempCommand, '\0', strlen(tempCommand));
	strcpy(tempCommand, "rm .temp_board");
	system(tempCommand);
	free  (tempCommand); free(block);

	close(fd2);
	fixLines(board); //replaces the line handles on hte end
	
	return 1;
}


int extractLineNum(char* line) {
	// parse until (, then we want everything after
	int i;
	char newStr[5]; // where we will store the number
  for (i=0; i<strlen(line); i++) {
          if (line[i] == '(') break;
  }


  int spot = i+1;
  for (i=0; i<strlen(line)- spot; i++) {
          if (line[i+spot] == ')') break;
          newStr[i] = line[i+spot];
  }
  newStr[i] = '\0';
	return atoi(newStr);
}

// this function makes sure the handle of each line stays up to date
void fixLines(char* filename) {
	// cp file into temp file first:
	char * command = malloc(50 * sizeof(char) + 25*sizeof(char));
	memset(command, '\0', 50+25);
	sprintf(command, "cp %s .temp_block", filename);
	system(command);
	memset(command, '\0', 50+25);
	sprintf(command, "rm %s", filename); // so we can write back out to this file!
	system(command);
	int fd_t = open(".temp_block", O_RDONLY, 0);
	// int fd = open(filename, O_RDONLY, 0);
	// close (fd);
	char * block = malloc(50 * sizeof(char));
	int brea = read_line(fd_t, block); // once to get rid of username:
	memset(command, '\0', 50+25);
	sprintf(command, "echo '%s' >> %s", block, filename);
	system(command);

	int line = 1;
	while (1) {
		brea = read_line(fd_t, block);
		if (brea==0) break;
		// printf("before fix: %s\n", block);
		fixParens(block, line);
		memset(command, '\0', 50+25);
		sprintf(command, "echo '%s' >> %s", block, filename);
		system(command);
		// printf("the block is now: %s\n", block);
		// printf("fix lines: %s\n", block);
		line ++;
	}
	memset(command, '\0', 50+25);
	strcpy(command, "rm .temp_block");
	system(command);
	free (command);
	close(fd_t);
}

// for actually logic behind editing number in parens at end of each line
void fixParens (char* block, int line) {
				int i;
        for (i=0; i<strlen(block); i++) {
                if (block[i] == '(') break;
								// if (block[i] == '\n') {
								// 	printf("new\n");
								// 	i--;
								// 	break;}
        }
        i++;
        char line_s[5];
        sprintf(line_s, "%d", line);
				int j;
        for (j=0; j<strlen(line_s); j++) {
                block[i+j] = line_s[j];
        }
				j--;
        block[i+j+1] = ')';
        block[i+j+2] = '\0';
}
