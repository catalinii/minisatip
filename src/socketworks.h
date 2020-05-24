#ifndef SOCKETWORKS_H
#define SOCKETWORKS_H
#define MAX_SOCKS 300
#include <netinet/in.h>
#include "utils.h"

typedef int (*socket_action)(void *s);
typedef int (*read_action)(int, void *, size_t, void *, int *);

typedef struct struct_spacket
{
	int len;
	int size;
	uint8_t *buf;
} SNPacket;

typedef union union_sockaddr {
	struct sockaddr sa;
	struct sockaddr_in sin;
	struct sockaddr_in6 sin6;
} USockAddr;

typedef struct struct_sockets
{
	char enabled;
	SMutex mutex;
	char is_enabled, force_close;
	int sock;	 // socket - <0 for invalid/not used, 0 for end of the list
	int nonblock; // non-blocking i/o mode
	USockAddr sa; //remote address - set on accept or recvfrom on udp sockets
	socket_action action;
	socket_action close;
	socket_action timeout;
	read_action read;
	int type;	  //0 - udp; 1 -> tcp(client); 2 -> server ; 3 -> http; 4-> rtsp
	int sid;	   //stream_id if set >=0 or adapter_id for dvb handles
	int64_t rtime; // read time
	int64_t wtime;
	unsigned char *buf;
	void *opaque, *opaque2, *opaque3;
	int lbuf;
	int rlen;
	int timeout_ms;
	int id; // socket id
	int iteration;
	int err;
	int flags; // 1 - buf is allocated dynamically
	int events;
	int64_t last_poll;
	pthread_t tid;
	SMutex *lock;
	int sock_err;
	SNPacket *pack;
	SNPacket prio_pack; // http_responses have higher priority
	int flush_enqued_data;
	int spos, wmax, wpos;
	int overflow, buf_alloc, buf_used;
	// if != -1 points to the master socket which holds the buffer and the action function.
	//Useful when the DVR buffer comes from different file handles
	int master;
} sockets;

#define IPTOS_DSCP_EF 0xb8
#define IPTOS_DSCP_MASK_VALUE 0xfc

#define TYPE_UDP 0
#define TYPE_TCP 1
#define TYPE_SERVER 2
#define TYPE_HTTP 3
#define TYPE_RTSP 4
#define TYPE_DVR 5
#define TYPE_RTCP 6
#define TYPE_NONBLOCK 256 // support for non blocking i/o mode
#define TYPE_CONNECT 512  // support for non blocking connect -> when it is connected call write with s->rlen 0
#define TYPE_IPV6 1024	// IPv6 support

#define MAX_HOST 50
#define SOCK_TIMEOUT -2
#define SELECT_TIMEOUT 100

char *setlocalip();
char *getlocalip();
int udp_connect(char *addr, int port, USockAddr *serv);
int udp_bind_connect(char *src, int sport, char *dest, int dport,
					 USockAddr *serv);
int udp_bind(char *addr, int port, int ipv4_only);
int tcp_connect(char *addr, int port, USockAddr *serv, int blocking);
int tcp_connect_src(char *addr, int port, USockAddr *serv, int blocking, char *src);
char *get_sock_shost(int fd, char *dest, int ld);
int get_sock_sport(int fd);
int get_sockaddr_port(USockAddr s);
char *get_sockaddr_host(USockAddr s, char *dest, int ld);
int sockets_add(int sock, USockAddr *sa, int sid, int type,
				socket_action a, socket_action c, socket_action t);
int sockets_del(int sock);
int no_action(int s);
void *select_and_execute(void *arg);
int get_mac_address(char *mac);
int fill_sockaddr(USockAddr *serv, char *host, int port, int ipv4_only);
int sockets_del_for_sid(int sid);
void set_socket_buffer(int sid, unsigned char *buf, int len);
void sockets_timeout(int i, int t);
void set_sockets_rtime(int i, int r);
void free_all();
void free_pack(SNPacket *p);
void sockets_setread(int i, void *r);
void sockets_setclose(int i, void *r);
void set_socket_send_buffer(int sock, int len);
void set_socket_receive_buffer(int sock, int len);
void set_socket_pos(int sock, int pos);
void set_sock_lock(int i, SMutex *m);
void set_socket_thread(int s_id, pthread_t tid);
pthread_t get_socket_thread(int s_id);
int tcp_listen(char *addr, int port, int ipv4_only);
int connect_local_socket(char *file, int blocking);
int set_linux_socket_nonblock(int sockfd);
int set_linux_socket_timeout(int sockfd);
int socket_enque_highprio(sockets *s, struct iovec *iov, int iovcnt);
int sockets_writev_prio(int sock_id, struct iovec *iov, int iovcnt, int high_prio);
int sockets_write(int sock_id, void *buf, int len);
int flush_socket(sockets *s);
void get_socket_iteration(int s_id, int it);
void set_sockets_sid(int id, int sid);
void set_socket_dscp(int id, int dscp, int prio);
void sockets_set_opaque(int id, void *opaque, void *opaque2, void *opaque3);
void sockets_force_close(int id);
void sockets_set_master(int slave, int master);
extern __thread char thread_name[];
extern __thread pthread_t tid;
extern __thread int select_timeout;

static inline sockets *get_sockets(int i)
{
	extern sockets *s[];
	if (i < 0 || i >= MAX_SOCKS || !s[i] || !s[i]->enabled || !s[i]->is_enabled)
		return NULL;
	return s[i];
}
#define sockets_writev(sock_id, iov, iovcnt) sockets_writev_prio(sock_id, iov, iovcnt, 0)
#define SOCKADDR_SIZE(a) ((a).sa.sa_family == AF_INET ? sizeof((a).sin) : sizeof((a).sin6))

#endif
