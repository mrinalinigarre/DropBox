#include "stdafx.h"
#include <winsock2.h>
#include <windows.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
struct node
{
	char msg[128];
	int msg_id;
	node *next;
}*flist,*alist,*printid;

struct bufserv{
	
		int userId;
		int forumId;
		int msgId;
		int commentId;
		int choice;
		char *forumname;
		char msg[128];
}buf1;

struct files{
	int bit;
	long int size;
	char user[20];
	char filename[16];
	//char *content;
};

struct content{
	char *buffer;
};

bool flag=true;
int mid = 0;
int count1 =0;
char *Data[100];
int count=1;
int values[100];
DWORD WINAPI SocketHandler(void*);
void replyto_client(char *buf, int *csock);

void socket_server() {

	//The port you want the server to listen on
	int host_port= 1101;

	unsigned short wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD( 2, 2 );
 	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 || ( LOBYTE( wsaData.wVersion ) != 2 ||
		    HIBYTE( wsaData.wVersion ) != 2 )) {
	    fprintf(stderr, "No sock dll %d\n",WSAGetLastError());
		goto FINISH;
	}

	//Initialize sockets and set options
	int hsock;
	int * p_int ;
	hsock = socket(AF_INET, SOCK_STREAM, 0);
	if(hsock == -1){
		printf("Error initializing socket %d\n",WSAGetLastError());
		goto FINISH;
	}
	
	p_int = (int*)malloc(sizeof(int));
	*p_int = 1;
	if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
		(setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
		printf("Error setting options %d\n", WSAGetLastError());
		free(p_int);
		goto FINISH;
	}
	free(p_int);

	//Bind and listen
	struct sockaddr_in my_addr;
	my_addr.sin_family = AF_INET ;
	my_addr.sin_port = htons(host_port);
	
	memset(&(my_addr.sin_zero), 0, 8);
	my_addr.sin_addr.s_addr = INADDR_ANY ;
	
	/* if you get error in bind 
	make sure nothing else is listening on that port */
	if( bind( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
		fprintf(stderr,"Error binding to socket %d\n",WSAGetLastError());
		goto FINISH;
	}
	if(listen( hsock, 10) == -1 ){
		fprintf(stderr, "Error listening %d\n",WSAGetLastError());
		goto FINISH;
	}
	
	//Now lets do the actual server stuff

	int* csock;
	sockaddr_in sadr;
	int	addr_size = sizeof(SOCKADDR);
	
	while(true){
		printf("waiting for a connection\n");
		csock = (int*)malloc(sizeof(int));
		
		if((*csock = accept( hsock, (SOCKADDR*)&sadr, &addr_size))!= INVALID_SOCKET ){
			printf("Received connection from %s",inet_ntoa(sadr.sin_addr));
			CreateThread(0,0,&SocketHandler, (void*)csock , 0,0);
		}
		else{
			fprintf(stderr, "Error accepting %d\n",WSAGetLastError());
		}
	}

FINISH:
;
}


void process_input(char *recvbuf, int recv_buf_cnt, int* csock) 
{

	char replybuf[1024];// = { 'n', 'a', 'v', 'y', 'a' };
	//printf("%s",recvbuf);
	printf("Enter some replay to client:");
	fflush(stdin);
	gets(replybuf);
	/*char c;
	for (char* p = replybuf; (c = getchar()) != 13; p++){
		printf("%c", c);
		*p = c;
	}*/
	replyto_client(replybuf, csock);
	replybuf[0] = '\0';
}

void replyto_client(char *buf, int *csock) {
	int bytecount;
	
	if((bytecount = send(*csock, buf, strlen(buf), 0))==SOCKET_ERROR){
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		free (csock);
	}
	printf("replied to client: %s\n",buf);
}

void get_user_file(char *buffer, char *user, char *filename){
	int i, j = -1;
	i = strlen(buffer)-1;
	//printf("\n\n%s\n", buffer);
	for (; buffer[i] != '$'; i--)
	{
		filename[++j] = buffer[i];
	}
	filename[++j] = '\0';
	filename=strrev(filename);
	i--;
	for (j = -1; buffer[i] != '$' || i < 0; i--)
	{
		user[++j] = buffer[i];
	}
	user[++j] = '\0';
	user = strrev(user);
	if (i > 0)
		buffer[i] = '\0';
}

void upload(char *buffer, FILE *f){
	struct files s;
	//buffer[strlen(buffer) - 1] = '\0';
	get_user_file(buffer, s.user, s.filename);
	//printf("user name is %s\nFile name is %s\n", s.user, s.filename);
	//printf("%s", buffer);
	s.size = strlen(buffer);
	s.bit = 1;
	//s.content = (char *)malloc(s.size*sizeof(char));
	//for (int i = 0; buffer[i] != '\0';i++)
	//strcpy(s.content, buffer);
	printf("user name is %s\nFile name is %s\nbit is %d\nsize is %ld\nlength of buffer %ld\ncontent\n%s\n", s.user, s.filename,s.bit,s.size,strlen(buffer),buffer);
	fseek(f, 0, SEEK_END);
	if (ftell(f) < 1048576){
		printf("\nGng to write %ld\n",ftell(f));
		fwrite(&s, sizeof(s), 1, f);
		fwrite(buffer, strlen(buffer), 1, f);
	}
}

char * viewRequestedFile(char *buffer)
{
	struct files s,p;
	FILE *f;
	char *content;
	int found = 0;
	f = fopen("D:\\blob.bin", "rb");
	get_user_file(buffer, s.user, s.filename);
	printf("User is %s\nfile requested is %s\n", s.user, s.filename);
	fseek(f, 0, SEEK_SET);
	while (fread(&p,sizeof(files),1,f)!=0)
	{
		printf("p.usser::%s\np.filename::%s\n", p.user, p.filename);
		if ((strcmp(p.user, s.user) == 0) && (strcmp(p.filename, s.filename) == 0))
		{
			found = 1;
			break;
		}
		else
		{
			fseek(f, p.size, SEEK_CUR);
		}
	}
	if (found == 1)
	{
		content = (char *)malloc(p.size*(sizeof(char)));
		memset(content, '\0', p.size);
		fread(content, p.size, 1, f);
		return content;
	}
	else
		return "file not found";
}

char * downloadRequestedFile(char *buffer)
{
	struct files s, p;
	FILE *f;
	char *content;
	int found = 0;
	f = fopen("D:\\blob.bin", "rb");
	get_user_file(buffer, s.user, s.filename);
	printf("User is %s\nfile requested is %s\n", s.user, s.filename);
	FILE *fp;
	fp = fopen(s.filename,"w");
	fseek(f, 0, SEEK_SET);
	while (fread(&p, sizeof(files), 1, f) != 0)
	{
		printf("p.usser::%s\np.filename::%s\n", p.user, p.filename);
		if ((strcmp(p.user, s.user) == 0) && (strcmp(p.filename, s.filename) == 0))
		{
			found = 1;
			break;
		}
		else
		{
			fseek(f, p.size, SEEK_CUR);
		}
	}
	if (found == 1)
	{
		content = (char *)malloc(p.size*(sizeof(char)));
		memset(content, '\0', p.size);
		fread(content, p.size, 1, f);
		printf("%s\n", connect);
		//fwrite(content, p.size, 1, fp);
		return content;
	}
	else
		return "downloading error";
}

char * deleteRequestedFile(char *buffer)
{
	struct files s, p;
	FILE *f;
	//char *content;
	int found = 0;
	long int offset=0;
	f = fopen("D:\\blob.bin", "rb+");
	get_user_file(buffer, s.user, s.filename);
	printf("User is %s\nfile requested is %s\n", s.user, s.filename);
	fseek(f, 0, SEEK_SET);
	while (fread(&p, sizeof(files), 1, f) != 0)
	{
		printf("p.usser::%s\np.filename::%s\n", p.user, p.filename);
		if ((strcmp(p.user, s.user) == 0) && (strcmp(p.filename, s.filename) == 0))
		{
			found = 1;
			break;
		}
		else
		{
			fseek(f, p.size, SEEK_CUR);
			offset = ftell(f);
		}
	}
	if (found == 1)
	{
		memset(p.user, '\0', sizeof(p.user));
		fseek(f, offset, SEEK_SET);
		fwrite(&p, sizeof(files), 1, f);
		return "File deleted successfully";
	}
	else
		return "file not exist";
}

DWORD WINAPI SocketHandler(void* lp){
    int *csock = (int*)lp;

	char *recvbuf;
	int recvbuf_len = 1048576;
	int recv_byte_cnt;
	recvbuf = (char *)malloc(1048576 * sizeof(char));
	memset(recvbuf, '\0', recvbuf_len);
	if((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0))==SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		free (csock);
		return 0;
	}
	int result;
	//printf("Received bytes %d\nReceived string \"%s\"\n", recv_byte_cnt, recvbuf);
	char ch = recvbuf[strlen(recvbuf)-1];
	FILE *f = fopen("D:\\blob.bin", "rb+");
	switch (ch)
	{
	case '1':
		printf("File ready to be uploaded\n");
		recvbuf[strlen(recvbuf) - 1] = '\0';
		upload(recvbuf,f);
		replyto_client("uploaded successfull....", csock);
		break;
	case '2':
		recvbuf[strlen(recvbuf) - 1] = '\0';
		replyto_client(viewRequestedFile(recvbuf), csock);
		break;
	case '3':
		recvbuf[strlen(recvbuf) - 1] = '\0';
		replyto_client(downloadRequestedFile(recvbuf), csock);
		break;
	case '4':
		recvbuf[strlen(recvbuf) - 1] = '\0';
		replyto_client(deleteRequestedFile(recvbuf), csock);
		break;
	default:
		break;
	}
	//process_input(recvbuf, recv_byte_cnt, csock);
	fclose(f);
    return 0;
}