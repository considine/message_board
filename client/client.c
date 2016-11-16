// JACK CONSIDINE
//    jconsidi
//      Message Board- PRG ASSGNMNT 4


#include "utility.h"
#define TCP 0
#define UDP 1
int client_tcp_setup (char * arg1, char * arg2, struct addrinfo **p, int sock_type);
int operation_state(int tcpsockfd, int udpsockfd, struct sockaddr_storage* src_addr, socklen_t src_addr_len);
int main (int argc, char* argv[]) {

	if (argc != 3) {
		printf("please enter hostname, followed by port!\n");
		exit(1);
	}
	struct sockaddr_storage src_addr;   // wil store the address for UDP, important to continue
	socklen_t src_addr_len=sizeof(src_addr);  //passing these two parameters
	struct addrinfo *p;
	int tcpsockfd = client_tcp_setup(argv[1], argv[2], &p, TCP);
	struct addrinfo *p2;
	int udpsockfd = client_tcp_setup(argv[1], argv[2], &p2, UDP);
	int cond = 0;
	while (cond==0) {
		printf("What username would you like to send\n");
		char username[50];
		scanf("%s", username);
		if (sendto(udpsockfd, username, strlen(username), 0, p2->ai_addr, p2->ai_addrlen) == -1)
			perror("send");
		getCodeUDP(udpsockfd);
		int numbytes;
		char password[40];
		printf("PASSWORD:\n");
		scanf("%s", password);
		sendStringUDP (password, udpsockfd, (struct sockaddr_storage *) p2->ai_addr, p2->ai_addrlen);


		cond = getCodeUDP(udpsockfd);
		if (cond==0)
			printf("\nwrong password\n");
	}

	operation_state(tcpsockfd, udpsockfd, (struct sockaddr_storage *) p2->ai_addr, p2->ai_addrlen);

	// NOW EVENT LOOP. Need to pass the udpsockfd, tcp sockfd, and p2.

	return 0;
}




int operation_state(int tcpsockfd, int udpsockfd, struct sockaddr_storage* src_addr, socklen_t src_addr_len) {
	int cond = 0;
	while (cond==0) {
		printf("\nChoose an operation...\n (CRT: Create Board, LIS: List Boards, MSG: Leave Message, DLT: Delete Message, RDB: Read Board, EDT: Edit Message, APN: Append File, DWN: Download File, DST: Destroy Board, XIT: Exit, SHT: Shutdown Server\n");
		char op[10];
		scanf("%s", op);
		if (strcmp(op, "CRT") == 0) {
			sendStringUDP(op, udpsockfd, src_addr, src_addr_len);
			printf("What board would you like to create?\n");
			char board[30];
			scanf("%s", board);
			sendStringUDP(board, udpsockfd, src_addr, src_addr_len);
			if (getCodeUDP(udpsockfd)<0) {
				printf("there was an error creating the board! It might already exist..\n");
			}
			else {
				printf("board created");
			}

		}
		else if (strcmp(op, "LIS") == 0) {

		}else if (strcmp(op, "MSG") == 0) {

		}else if (strcmp(op, "DLT") == 0) {

		}else if (strcmp(op, "RDB") == 0) {

		}else if (strcmp(op, "APN") == 0) {

		}else if (strcmp(op, "DWN") == 0) {

		}else if (strcmp(op, "DST") == 0) {

		}else if (strcmp(op, "XIT") == 0) {
			sendStringUDP(op, udpsockfd, src_addr, src_addr_len);
			cond = 1;

		}else if (strcmp(op, "SHT") == 0) {
			sendStringUDP(op, udpsockfd, src_addr, src_addr_len);
			cond = 1;
		}
		else {
			printf ("that is not an operation\n");
		}
	}
	return 0;
}

int client_tcp_setup(char* arg1, char* arg2, struct addrinfo **p_ptr, int sock_type) {
	struct addrinfo hints, *servinfo;//, *p;
	struct addrinfo *p;

	//connect to tcp
	memset (&hints, 0, sizeof hints);
				hints.ai_family = AF_UNSPEC;
				hints.ai_flags = AI_PASSIVE;
				if (sock_type==TCP) {
					hints.ai_socktype = SOCK_STREAM;
				} else {
					hints.ai_socktype = SOCK_DGRAM;
				}
	struct sockaddr_storage their_addr;
				int rv, sockfd;
	if ((rv = getaddrinfo(arg1, arg2, &hints, &servinfo)) != 0) {
								perror("server: socket");
								return 1;
				}

	//iterate through each of the nodes in serviceinfo. should just be one, this contains all the relevant info like
				// ip address and protocol
				for (p=servinfo; p != NULL; p = p->ai_next) {
								if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
												perror("talker: socket");
												continue;
								}
								printf("socktype is %d and TCP is %d", sock_type, TCP);
								if (sock_type==TCP) {
									if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
													close(sockfd);
													perror("client: connect");
													continue;
									}
								}

								break;
				}
		printf("p2 ai_addrlen is %d\n", p->ai_addrlen);

	if (p==NULL) {
		fprintf(stderr, "Failed to bind to the socket\n");
		exit(1);
	}
	*p_ptr = p;

	//freeaddrinfo(servinfo);
	return sockfd;
}
