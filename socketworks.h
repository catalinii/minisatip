#ifndef SOCKETWORKS_H
#define SOCKETWORKS_H
#define MAX_SOCKS 100
#include <netinet/in.h>
#include "utils.h"

typedef int (*socket_action)(void *s);
typedef int (*read_action)(int, void *, size_t, void *, int *);

typedef struct struct_spacket {
	uint16_t len;
	uint16_t size;
	uint8_t *buf;
} SNPacket;

typedef struct struct_sockets {
	char enabled;
	SMutex mutex;
	int sock;		// socket - <0 for invalid/not used, 0 for end of the list
	int nonblock;		// non-blocking i/o mode
	struct sockaddr_in sa;//remote address - set on accept or recvfrom on udp sockets
	socket_action action;
	socket_action close;
	socket_action timeout;
	read_action read;
	int type;	//0 - udp; 1 -> tcp(client); 2 -> server ; 3 -> http; 4-> rtsp
	int sid;				//stream_id if set >=0 or adapter_id for dvb handles
	int64_t rtime;					 // read time
	int64_t wtime;
	unsigned char *buf;
	int lbuf;
	int rlen;
	int timeout_ms;
	int id;				 // socket id
	int iteration;
	int err;
	int flags; // 1 - buf is allocated dynamically
	int events;
	int64_t last_poll;
	pthread_t tid;
	SMutex *lock;
	int sock_err;
	SNPacket *pack;
	int spos, wmax, wpos;
	int overflow, buf_alloc, buf_used;
} sockets;

#define TYPE_UDP 0
#define TYPE_TCP 1
#define TYPE_SERVER 2
#define TYPE_HTTP 3
#define TYPE_RTSP 4
#define TYPE_DVR 5
#define TYPE_RTCP 6
#define TYPE_NONBLOCK 256 // support for non blocking i/o mode
#define TYPE_CONNECT 512 // support for non blocking connect -> when it is connected call write with s->rlen 0

#define MAX_HOST 50
#define SOCK_TIMEOUT -2
char *setlocalip();
char *getlocalip();
int udp_connect(char *addr, int port, struct sockaddr_in *serv);
int udp_bind_connect(char *src, int sport, char *dest, int dport,
		struct sockaddr_in *serv);
int udp_bind(char *addr, int port);
int tcp_connect(char *addr, int port, struct sockaddr_in *serv, int blocking);
char *get_sock_shost(int fd);
int get_sock_sport(int fd);
int sockets_add(int sock, struct sockaddr_in *sa, int sid, int type,
		socket_action a, socket_action c, socket_action t);
int sockets_del(int sock);
int no_action(int s);
void *select_and_execute(void *args);
int get_mac(char *mac);
int fill_sockaddr(struct sockaddr_in *serv, char *host, int port);
int sockets_del_for_sid(int ad);
char *get_current_timestamp();
char *get_current_timestamp_log();
void set_socket_buffer(int sid, unsigned char *buf, int len);
void sockets_timeout(int i, int t);
void set_sockets_rtime(int s, int r);
void free_all();
void sockets_setread(int i, void *r);
void set_socket_send_buffer(int sock, int len);
void set_socket_receive_buffer(int sock, int len);
sockets *get_sockets(int i);
void set_socket_pos(int sock, int pos);
char *get_socket_rhost(int s_id, char *dest, int ld);
int get_socket_rport(int s_id);
void set_sock_lock(int i, SMutex *m);
void set_socket_thread(int s_id, pthread_t tid);
pthread_t get_socket_thread(int s_id);
int tcp_listen(char *addr, int port);
int connect_local_socket(char *file, int blocking);
int set_linux_socket_nonblock(int sockfd);
int set_linux_socket_timeout(int sockfd);
int sockets_writev(int sock_id, struct iovec *iov, int iovcnt);
int sockets_write(int sock_id, void *buf, int len);
int flush_socket(sockets *s);
void get_socket_iteration(int s_id, int it);
extern __thread char *thread_name;
extern __thread pthread_t tid;


#endif
