/* 
	Author: Katherine Weng
			Song Xu
			Huanzhi Zhu
	CSCI-4220, Assignment 1
*/

/* To compile: gcc -o hw1.out hw1.c lib/libunp.a */
#include "lib/unp.h"
#include <string.h>

/* Change the #include for submission
	#include "unp.h"
*/

/*-------------------------- Struct Declaration -----------------------------*/
/*  	
		OP # 		TYPE
		01 			RRQ
		02 			WRQ
		03 			DATA
		04 			ACK
		05 			ERROR

 */

typedef struct Packet {
	
	unsigned short int OpCode = -1; /* ALL, 2 bytes */

	char FileName[MAXLINE]; /* RRQ or WRQ, string*/

	unsigned short int BlockNo = -1; /* DATA or ACK, 2 bytes */
	char Data[513]; /* DATA, n <= 512 bytes*/

	unsigned short int ErrorCode = -1; /* ERROR, 2 bytes*/
	char ErrMsg[MAXLINE]; /* ERROR, n bytes */
} Packet;

/*---------------------------------------------------------------------------*/

/*---------------------------- GLOBAL VAIRABLES -----------------------------*/

/* Sender retransmits its last packet upon NOT receiving data for 1 second */
#define RETRANSMIT_TIME_LIMIT 1 

/* Abort the connection if NOT heard from the other party for 10 seconds */
#define ABORT_TIME_LIMIT 10

int MAX_TID_COUNT;

int COUNT = 0;
/*---------------------------------------------------------------------------*/

/*-------------------------- Function Declaration ---------------------------*/

/* Create socket and bind to given port to the socket */
int UDPsetup(int port);

/* Fill struct Packet with given msg */
void Parse_msg(char* msg, Packet* packet);

/*---------------------------------------------------------------------------*/

int main( int argc, char **argv ) {

	/*setbuf(stdout, NULL);*/

	/*-------------------------- CHECK COMMAND LINE ARGUMENTS --------------------------*/
	if ( argc != 3 ) {
		fprintf( stderr, "ERROR: Invalid argument(s)\n"
						 "USAGE: a.out <start of port range>"
						 " <end of port range>\n" );
		return EXIT_FAILURE;
	}

	int start = atoi( *(argv+1) );	/* start of port range */
	int end	  = atoi( *(argv+2) );	/* end of port range*/

	MAX_TID_COUNT = end - start;

	/* printf("start: %d, end: %d\n", start, end); */

	/*----------------------------------------------------------------------------------*/

	/*----------------------------------- UDP SETUP ------------------------------------*/

	int sockfd = UDPsetup(start);

	socklen_t	len = sizeof(cliaddr);
	char		msg[MAXLINE];
	/*----------------------------------------------------------------------------------*/

/*	Signal(SIGINT, recvfrom_int);*/

	/*----------------------------- START RECEIVING REQUEST ----------------------------*/
	for ( ; ; ) {

		memset(packet, 0, MAXLINE);

		Recvfrom(sockfd, msg, MAXLINE, 0, (SA *) &cliaddr, &len);

		/* start+COUNT will give the port number to use */
		COUNT++;

#if DEBUG
		printf("Parent: fd %d received: %s\n", sockfd, msg);
#endif

		pid_t pid = fork(); /* create a child process */

		/* Error checking */
		if ( pid == -1 ) {
		    perror( "fork() failed" );
    		return EXIT_FAILURE;
		}

		/* Let child process handle the request */
		if ( pid == 0 ) {
			close(sockfd);

			/* interact with client using a different port */
			sockfd = UDPsetup(start+COUNT);


		}

		/* Let parent process go back to the top of the loop waiting for next packet */
		else {
#if DEBUG
			printf("Parent: BACK TO TOP OF LOOP\n");
#endif
		}


	}

	
	return EXIT_SUCCESS;
}

int UDPsetup(int port) {
	int					sockfd;
	struct sockaddr_in	servaddr, cliaddr;

	sockfd = Socket(AF_INET, SOCK_DGRAM, 0);


	/* server info */
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(port);

	Bind(sockfd, (SA *) &servaddr, sizeof(servaddr));

	return sockfd;
}

void Parse_msg(char* msg, Packet* packet) {

	bzero(packet, sizeof(Packet));

	/* Get OPCODE */
	unsigned short int* opcode_ptr = (unsigned short int*) msg;
	packet->OpCode = ntohs(*opcode_ptr);

	/* RRQ or WRQ */
	if (packet->OpCode == 1 ||packet->OpCode == 2) {

		/* Get Filename */
		char* tmp = msg+2; /* skip first 2 bytes */
		strcpy(packet->FileName, tmp);
	} 
	/* DATA or ACK */
	else if (packet->OpCode == 3 || packet->OpCode == 4) {
		/* bytes 3 and 4 */
		packet->BlockNo = ntohs(*(opcode_ptr+1));

	} 
	/* ERROR */
	else if (packet->OpCode == 5) {

		packet->ErrorCode = ntohs(*(opcode_ptr+1));

		/* Get ErrMsg */
		char* tmp = msg+2; /* skip first 2 bytes */
		strcpy(packet->ErrMsg, tmp);
	}
}



