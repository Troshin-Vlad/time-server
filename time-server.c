#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define CLR_ERR "\033[31m"
#define CLR_OK	"\033[32m"
#define CLR_DEF "\033[0m"

#define BUF_TM 64

int sock_d;
int connect_d;

void error(const char*);
void closed_socket();
void socket_bind(int);
void setlisten(int, int);
void server_shutdown(int);
void retcurs();
int open_socket();

void gettm(const char *, char *, int);
void gettime(char*,int);
void getdate(char*,int);

int main(){

	if(signal(SIGINT, server_shutdown) == SIG_ERR)
		error("cat't catch signal(SIGINT)");

	sock_d = open_socket();
	socket_bind(sock_d);
	setlisten(sock_d, 25);

	struct sockaddr_storage user;
	unsigned int user_size = sizeof(user);

	char date_time[BUF_TM];
	char date_date[BUF_TM];

	while(1){

		connect_d = accept(sock_d, (struct sockaddr*)&user, &user_size);
		if(connect_d == -1)
			error("connecing");

		if( !fork() ){

			char send_date[128] = "";
			while(1){

				gettime(date_time, BUF_TM-1);
				getdate(date_date, BUF_TM-1);

				snprintf(send_date, 127, "seconds: %li\r\ntime: [%s]\r\ndate: [%s]\r\n\033[3A", time(NULL), date_time, date_date);
				send(connect_d, send_date, 127, 0);

				sleep(1);
			}
			close(connect_d);
			exit(0);

		}
		close(connect_d);
	}

	closed_socket();
	return 0;
}

void server_shutdown(int sig){

	retcurs();
	
	closed_socket();
	fprintf(stderr, "\033[2Dserver:[%sshutdown%s]\n", CLR_OK, CLR_DEF);
	exit(0);
}

void retcurs(){
	send(connect_d, "\033[3B", 5, 0);
}

void gettm(const char *tfmt, char *dst, int size){
	long int s_time;
	struct tm *m_time;

	s_time = time(NULL);
	m_time = localtime(&s_time);

	strftime(dst, size, tfmt, m_time);
}

void gettime(char *dst, int len){
	gettm("%H:%M:%S", dst, len);
}

void getdate(char *dst, int len){
	gettm("%m.%d.%Y", dst, len);
}

void closed_socket(){
	if(sock_d)
		close(sock_d);
	if(connect_d)
		close(connect_d);
}

void socket_bind(int sock){
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(12200);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1)
		error("bind socket with port");
}

void setlisten(int sock, int count){
	if(listen(sock, count) == -1)
		error("setup queue listen on socket");
}

int open_socket(){
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock == -1)
		error("open socket");

	int reuse = -1;
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(int)) == -1)
		error("setup option reuse socket");

	return sock;
}

void error(const char *msg){
	fprintf(stderr, "[%serror%s]: %s\n",CLR_ERR, CLR_DEF, msg);
	exit(1);
}