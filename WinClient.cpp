#include <winsock2.h>
#include "StdAfx.h"
#include <windows.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <conio.h>

int getsocket()
{
	int hsock;
	int * p_int ;
	hsock = socket(AF_INET, SOCK_STREAM, 0);
	if(hsock == -1){
		printf("Error initializing socket %d\n",WSAGetLastError());
		return -1;
	}
	
	p_int = (int*)malloc(sizeof(int));
	*p_int = 1;
	if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
		(setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
		printf("Error setting options %d\n", WSAGetLastError());
		free(p_int);
		return -1;
	}
	free(p_int);

	return hsock;
}

void socket_client()
{

	//The port and address you want to connect to
	int host_port= 1101;
	char* host_name="127.0.0.1";

	//Initialize socket support WINDOWS ONLY!
	unsigned short wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 2, 2 );
 	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 || ( LOBYTE( wsaData.wVersion ) != 2 ||
		    HIBYTE( wsaData.wVersion ) != 2 )) {
	    fprintf(stderr, "Could not find sock dll %d\n",WSAGetLastError());
		goto FINISH;
	}

	//Initialize sockets and set any options

	//Connect to the server
	struct sockaddr_in my_addr;

	my_addr.sin_family = AF_INET ;
	my_addr.sin_port = htons(host_port);
	
	memset(&(my_addr.sin_zero), 0, 8);
	my_addr.sin_addr.s_addr = inet_addr(host_name);

	//if( connect( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == SOCKET_ERROR ){
	//	fprintf(stderr, "Error connecting socket %d\n", WSAGetLastError());
	//	goto FINISH;
	//}

	//Now lets do the client related stuff
	char *buffer;
	int buffer_len = 1048576;
	int bytecount;
	int c;

	while(true) {

		int hsock = getsocket();
		//add error checking on hsock...
		if( connect(hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == SOCKET_ERROR ){
			fprintf(stderr, "Error connecting socket %d\n", WSAGetLastError());
			goto FINISH;
		}
		buffer = (char *)malloc(1048576 * sizeof(char));
		memset(buffer, '\0', buffer_len);
		
		char name[10];
		printf("Enter ur user name");
		scanf("%s", name);
		printf("1.upload file\n2.view file\n3.download file\n4.delete file\nenter choice:");
		int choice, i, j;
		FILE *fp;
		scanf("%d", &choice);
		char fname[15];
		switch (choice)
		{
		case 1:
			memset(buffer, '\0', sizeof(buffer));
			printf("Enter path name:");
			scanf("%s", buffer);
			printf("Enter file name:");
			
			scanf("%s", fname);
			fp = fopen(buffer, "r");
			if (!fp)
			{
				perror("can't open file:");
			}
			else
			{
				printf("opened successfully\n");
				memset(buffer, '\0', 1048576);
				long int size;
				fseek(fp, 0, SEEK_END);
				size = ftell(fp);
				fseek(fp, 0, SEEK_SET);
				fread(buffer, size, 1, fp);
				printf("bufferis\n%s\n\n size::%ld\n%d", buffer, size, strlen(buffer));
			}
			j = strlen(buffer);
			buffer[j] = '$';
			for (i = 0, j = j + 1; name[i] != '\0'; i++, j++)
			{
				buffer[j] = name[i];
			}
			buffer[j] = '$';
			for (i = 0, j = j + 1; fname[i] != '\0'; i++, j++)
			{
				buffer[j] = fname[i];
			}

			buffer[j] = '1';
			j++;
			buffer[j] = '\0';
		//	printf("%s\n", buffer);
			if ((bytecount = send(hsock, buffer, strlen(buffer), 0)) == SOCKET_ERROR){
				fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
				goto FINISH;
			}
			printf("Sent bytes %d\n", bytecount);
			for (int i = 0; i < bytecount; i++)
				buffer[i] = '\0';
			if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR){
				fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
				goto FINISH;
			}
			printf("Recieved bytes %d\nReceived string \"%s\"\n", bytecount, buffer);
			printf("%s\n", buffer);
			break;
		case 2:
			memset(buffer, '\0', sizeof(buffer));
			//char fname[15];
			printf("Enter file name to view:");
			scanf("%s", fname);
			buffer[0] = '$';
			buffer[1] = '\0';
			j = strlen(buffer);
			//buffer[j] = '2';
			for (i = 0; name[i] != '\0'; i++, j++)
			{
				buffer[j] = name[i];
			}
			buffer[j] = '$';
			for (i = 0, j = j + 1; fname[i] != '\0'; i++, j++)
			{
				buffer[j] = fname[i];
			}
			buffer[j] = '2';
			j++;
			buffer[j] = '\0';
			printf("%s\n", buffer);
			if ((bytecount = send(hsock, buffer, strlen(buffer), 0)) == SOCKET_ERROR){
				fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
				goto FINISH;
			}
			printf("Sent bytes %d\n", bytecount);
			for (int i = 0; i < bytecount; i++)
				buffer[i] = '\0';
			if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR){
				fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
				goto FINISH;
			}
			printf("Recieved bytes %d\nReceived string \"%s\"\n", bytecount, buffer);
			printf("%s\n", buffer);
			break;
		case 3:
			memset(fname, '\0', sizeof(fname));
			memset(buffer, '\0', sizeof(buffer));
			printf("Enter file name to be downloaded:");
			scanf("%s", fname);
			buffer[0] = '$';
			buffer[1] = '\0';
			j = strlen(buffer);
			//buffer[j] = '3';
			for (i = 0; name[i] != '\0'; i++, j++)
			{
				buffer[j] = name[i];
			}
			buffer[j] = '$';
			for (i = 0, j = j + 1; fname[i] != '\0'; i++, j++)
			{
				buffer[j] = fname[i];
			}
			buffer[j] = '3';
			j++;
			buffer[j] = '\0';
			printf("%s\n", buffer);
			if ((bytecount = send(hsock, buffer, strlen(buffer), 0)) == SOCKET_ERROR){
				fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
				goto FINISH;
			}
			printf("Sent bytes %d\n", bytecount);
			for (int i = 0; i < bytecount; i++)
				buffer[i] = '\0';
			if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR){
				fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
				goto FINISH;
			}
			printf("Recieved bytes %d\nReceived string \"%s\"\n", bytecount, buffer);
			printf("%s\n", buffer);
			FILE *download;
			download = fopen(fname, "w");
			fwrite(buffer, bytecount, 1, download);
			fclose(download);
			break;
		case 4:
			memset(buffer, '\0', sizeof(buffer));
			printf("Enter file name to be deleted:");
			scanf("%s", fname);
			buffer[0] = '$';
			buffer[1] = '\0';
			j = strlen(buffer);
			for (i = 0; name[i] != '\0'; i++, j++)
			{
				buffer[j] = name[i];
			}
			buffer[j] = '$';
			for (i = 0, j = j + 1; fname[i] != '\0'; i++, j++)
			{
				buffer[j] = fname[i];
			}
			buffer[j] = '4';
			j++;
			buffer[j] = '\0';
			if ((bytecount = send(hsock, buffer, strlen(buffer), 0)) == SOCKET_ERROR){
				fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
				goto FINISH;
			}
			printf("Sent bytes %d\n", bytecount);
			for (int i = 0; i < bytecount; i++)
				buffer[i] = '\0';
			if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR){
				fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
				goto FINISH;
			}
			printf("Recieved bytes %d\nReceived string \"%s\"\n", bytecount, buffer);
			printf("%s\n", buffer);
			break;
		default:
			break;
		}
		/*fflush(stdin);*/
		//buffer[0] = '\0';


		closesocket(hsock);
	}

	//closesocket(hsock);
FINISH:
;
}