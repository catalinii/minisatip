#ifndef SRT_H
#define SRT_H

#ifndef DISABLE_SRT
#include <srt/srt.h>
#include <string>

// SRT listener initialization and shutdown
int srt_listener_init();
void srt_listener_close();

// Helper function to retrieve pending SRT sockets by streamid
SRTSOCKET srt_pending_take(const std::string &streamid);

// Return a connected SRT socket to the pending pool for reuse
void srt_pending_return(SRTSOCKET sock, const std::string &streamid);

// Check if SRT listener is initialized
int srt_listener_is_init();

// Check if SRT socket is in connected state
int srt_socket_is_connected(SRTSOCKET sock);

#endif // DISABLE_SRT

#endif // SRT_H
