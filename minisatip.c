/*
 * Copyright (C) 2014-2020 Catalin Toda <catalinii@yahoo.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 *
 */

#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <sys/ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "minisatip.h"
#include "socketworks.h"
#include "stream.h"
#include "adapter.h"
#include "dvb.h"

struct struct_opts opts;
int rtp,
rtpc;
extern sockets s[MAX_SOCKS];

void
usage ()
{
	printf ("minisatip [-f] [-r remote_rtp_host] [-d device_id] [-w http_server[:port]] [-p public_host] [-s rtp_port] [-a no] [-m mac] [-l] [-a X:Y:Z] [-e X-Y,Z]\n \
		-f foreground, otherwise run in background\n\
		-r remote_rtp_host: send remote rtp to remote_rtp_host\n \
		-d specify the device id (in case there are multiple SAT>IP servers in the network)\n \
		-w http_server[:port]: specify the host and the port where the xml file can be downloaded from \n\
		-x port: port for listening on http\n\
		-s force to get signal from the DVB hardware every 200ms (use with care, onle when needed)\n\
		-a x:y:z simulate x DVB-S2, y DVB-T2 and z DVB-C adapters on this box (0 means autodetect)\n\
		-m xx: simulate xx as local mac address, generates UUID based on mac\n\
		-e list_of_enabled adapters: enable only specified adapters, example 0-2,5,7 (no spaces between parameters)\n\
		-c X: bandwidth capping for the output to the network (default: unlimited)\n\
		-b X: set the DVR buffer to X KB (default: %dKB)\n\
		-l increases the verbosity (you can use multiple -l), logging to stdout in foreground mode or in /tmp/log when a daemon\n\
		-p url: specify playlist url using X_SATIPM3U header \n\
		",
		DVR_BUFFER / 1024);
	exit (1);
}


void
set_options (int argc, char *argv[])
{
	int opt;
	int sethost = 0;
	int index;
	char *lip;

	opts.log = 1;
	opts.rrtp = NULL;
	opts.disc_host = "239.255.255.250";
	opts.start_rtp = 5500;
	opts.http_port = 8080;
	opts.http_host = NULL;
	opts.log = 0;
	opts.timeout_sec = 30000;
	opts.force_sadapter = 0;
	opts.force_tadapter = 0;
	opts.force_cadapter = 0;
	opts.mac[0] = 0;
	opts.daemon = 1;
	opts.bw = 0;
	opts.device_id = 1;
	opts.force_scan = 0;
	opts.dvr = DVR_BUFFER;
	memset(opts.playlist, sizeof(opts.playlist), 0);
	
	while ((opt = getopt (argc, argv, "flr:a:t:d:w:p:shc:b:m:p:e:x:")) != -1)
	{
		//              printf("options %d %c %s\n",opt,opt,optarg);
		switch (opt)
		{
			case FOREGROUND_OPT:
			{
				opts.daemon = 0;
				break;
			}
			case MAC_OPT:
			{
				strncpy (opts.mac, optarg, 12);
				opts.mac[12] = 0;
				break;
			}
			case RRTP_OPT:
			{
				opts.rrtp = optarg;
				break;
			}

			case DEVICEID_OPT:
			{
				opts.device_id = atoi (optarg);
				break;
			}

			case HTTPSERVER_OPT:
			{
				//                              int i=0;
				opts.http_host = optarg;
				sethost = 1;
				break;
			}

			case LOG_OPT:
			{
				opts.log++;
				break;
			}

			case HELP_OPT:
			{
				usage ();
				exit (0);
			}

			case HTTPPORT_OPT:
			{
				opts.http_port = atoi (optarg);
				break;
			}

			case BW_OPT:
			{
				opts.bw = atoi (optarg) * 1024;
				break;
			}

			case DVRBUFFER_OPT:
			{
				opts.dvr = atoi (optarg) * 1024;
				break;
			}

			case DVBS2_ADAPTERS_OPT:
			{
				sscanf(optarg,"%d:%d:%d", &opts.force_sadapter, &opts.force_tadapter, &opts.force_cadapter) ;
				break;
			}

			case DVBT2_ADAPTERS_OPT:
			{
				opts.force_tadapter = atoi (optarg);
				break;
			}
			
			case SCAN_OPT:
			{
				opts.force_scan = 1;
				break;
			}

			case PLAYLIST_OPT:
			{
				snprintf(opts.playlist, sizeof(opts.playlist), "<satip:X_SATIPM3U xmlns:satip=\"urn:ses-com:satip\">%s</satip:X_SATIPM3U>\r\n",optarg);
				break;
			}
			
			case ENABLE_ADAPTERS_OPT:
			{
				enable_adapters(optarg);
				break;
			}
		}
	}
	
	lip = getlocalip ();
	if (!opts.http_host)
	{
		opts.http_host = (char *) malloc (MAX_HOST);
		sprintf (opts.http_host, "%s:%d", lip, opts.http_port);
	}
}


#ifdef __mips__

void
hexDump (char *desc, void *addr, int len)
{
	int i;
	unsigned char buff[17];
	unsigned char *pc = (unsigned char *) addr;

	// Output description if given.
	if (desc != NULL)
		printf ("%s:\n", desc);

	// Process every byte in the data.
	for (i = -len; i < len; i++)
	{
		// Multiple of 16 means new line (with line offset).

		if ((i % 16) == 0)
		{
			// Just don't print ASCII for the zeroth line.
			if (i != 0)
				printf ("  %s\n", buff);

			// Output the offset.
			printf ("  %08x ", ((unsigned int) addr) + i);
		}

		// Now the hex code for the specific character.
		printf (" %02x", pc[i]);

		// And store a printable ASCII character for later.
		if ((pc[i] < 0x20) || (pc[i] > 0x7e))
			buff[i % 16] = '.';
		else
			buff[i % 16] = pc[i];
		buff[(i % 16) + 1] = '\0';
	}

	// Pad out last line if not exactly 16 characters.
	while ((i % 16) != 0)
	{
		printf ("   ");
		i++;
	}

	// And print the final ASCII bit.
	printf ("  %s\n", buff);
}
#endif

void posix_signal_handler (int sig, siginfo_t * siginfo, ucontext_t * ctx);
void
set_signal_handler ()
{
	struct sigaction sig_action = { };
	sig_action.sa_sigaction =
		(void (*)(int, siginfo_t *, void *)) posix_signal_handler;
	sigemptyset (&sig_action.sa_mask);

	sig_action.sa_flags = SA_SIGINFO | SA_ONSTACK;

	#ifndef __mips__
    if (sigaction(SIGBUS, &sig_action, NULL) != 0) { LOG("Could not set signal SIGBUS"); }
	if (sigaction(SIGSEGV, &sig_action, NULL) != 0) { LOG("Could not set signal SIGSEGV"); }
    if (sigaction(SIGABRT, &sig_action, NULL) != 0) { LOG("Could not set signal SIGABRT"); }
    if (sigaction(SIGFPE,  &sig_action, NULL) != 0) { LOG("Could not set signal SIGFPE"); }
    if (sigaction(SIGILL,  &sig_action, NULL) != 0) { LOG("Could not set signal SIGILL"); }
	#endif
	if (sigaction (SIGINT, &sig_action, NULL) != 0) { LOG("Could not set signal SIGINT");}
	
	//    if (sigaction(SIGTERM, &sig_action, NULL) != 0) { err(1, "sigaction"); }
	if (signal (SIGHUP, SIG_IGN) != 0) { LOG("Could not ignore signal SIGHUP"); }
	if (signal (SIGPIPE, SIG_IGN) != 0) { LOG("Could not ignore signal SIGPIPE"); }
}


int
split (char **rv, char *s, int lrv, char sep)
{
	int i = 0,
		j = 0;

	if (!s)
		return 0;
	for (i = 0; s[i] && s[i] == sep && s[i] < 32; i++);

	rv[j++] = &s[i];
	//      LOG("start %d %d\n",i,j);
	while (j < lrv)
	{
		if (s[i] == 0 || s[i + 1] == 0)
			break;
		if (s[i] == sep || s[i] < 33)
		{
			s[i] = 0;
			if (s[i + 1] != sep && s[i + 1] > 32)
				rv[j++] = &s[i + 1];
		}
		else if (s[i] < 14)
			s[i] = 0;
		//              LOG("i=%d j=%d %d %c \n",i,j,s[i],s[i]);
		i++;
	}
	if (s[i] == sep)
		s[i] = 0;
	rv[j] = NULL;
	return j;
}


#define LR(s) {LOG("map_int returns %d",s);return s;}
int map_int (char *s, char ** v)
{
	int i, n = 0;

	if (v == NULL)
		return atoi (s);
	for (i = 0; v[i]; i++)
		if (!strncasecmp (s, v[i], strlen (v[i])))
			n = i;
	return n;
}


int
map_float (char *s, int mul)
{
	float f;
	int r;

	if (s[0] < '0' || s[0] > '9')
		return 0;
	f = atof (s);
	r = (int) (f * mul);
	//      LOG("atof returned %.1f, mul = %d, result=%d",f,mul,r);
	return r;

}

char public[] = "Public: OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN";

char *
http_response (sockets *s, int rc, char *ah, char *desc, int cseq, int lr)
{
	char *reply =
		"%s/1.0 %d %s\r\nCSeq: %d\r\nDate: %s\r\n%s\r\nContent-Length: %d\r\n\r\n%s";
	char *reply0 = 
		"%s/1.0 %d %s\r\nCseq: %d\r\nDate: %s\r\n%s\r\n\r\n";
	char *d;
	char *proto;
	
	if( s->type == TYPE_HTTP)
		proto = "HTTP";
	else 
		proto = "RTSP";
		
	if (!ah)
		ah = public;
	if (!desc)
		desc = "";
	if (rc == 200)
		d = "OK";
	else if (rc == 400)
		d = "Bad Request";
	else if (rc == 403)
		d = "Forbidden";
	else if (rc == 404)
		d = "Not Found";
	else if (rc == 500)
		d = "Internal Server Error";
	else if (rc == 501)
		d = "Not Implemented";
	else if (rc == 405)
		d = "Method Not Allowed";
	else 
	{
		d = "Service Unavailable";
		rc = 503;
	}
	static char resp[5000];
	if(!lr)
		lr = strlen (desc);
	if (lr)
		sprintf (resp, reply, proto, rc, d, cseq, get_current_timestamp (), ah, lr, desc);
	else
		sprintf (resp, reply0, proto, rc, d, cseq, get_current_timestamp (), ah);
	LOG ("reply -> %d (%s:%d) CL:%d :\n%s", s->sock, inet_ntoa(s->sa.sin_addr), ntohs(s->sa.sin_port), lr, resp);
	send (s->sock, resp, strlen (resp), MSG_NOSIGNAL);
	return resp;
}

#define RBUF 4000

int
read_rtsp (sockets * s)
{
	char *arg[50];
	int cseq, la, i, rlen;
	char *proto;
	char buf[2000];
	streams *sid = NULL;

	if(s->buf[0]==0x24 && s->buf[1]<2)
	{
		int rtsp_len = s->buf[2]*256+s->buf[3];
		LOG("Received RTSP over tcp packet (sock_id %d, stream %d, rlen %d) packet len: %d, type %02X %02X discarding %s...", 
			s->sock_id, s->sid, s->rlen, rtsp_len , s->buf[4], s->buf[5], (s->rlen == rtsp_len+4)?"complete":"fragment" );		
		if(s->rlen == rtsp_len+4){ // we did not receive the entire packet
			s->rlen = 0;			
			return 0;
		}
	}
	
	if (s->rlen < 4
		|| (htonl (*(uint32_t *) & s->buf[s->rlen - 4]) != 0x0D0A0D0A))
	{
		if( s->rlen > RBUF - 10 )
		{
			LOG("Discarding %d bytes from the socket buffer, request > %d, consider increasing  RBUF", s->rlen, RBUF);
			s->rlen = 0;
		}
		if ( s->flags & 1 ) 
			return 0;		
		unsigned char *new_alloc = malloc1 (RBUF);
		memcpy(new_alloc, s->buf, s->rlen);
		s->buf = new_alloc;
		s->flags = s->flags | 1;
		return 0;
	}

	rlen = s->rlen;
	s->rlen = 0;

	LOG ("read RTSP (from handle %d sock_id %d, ts: %d, len: %d):\n%s", s->sock, s->sock_id, s->rtime, s->rlen, s->buf);

	if( (s->type != TYPE_HTTP ) && (strncasecmp(s->buf, "GET", 3) == 0))
	{
		http_response (s , 404, NULL, NULL, 0, 0);
		return 0;
	}
	
	la = split (arg, s->buf, 50, ' ');
	cseq = 0;	
	if (la<2)
		LOG_AND_RETURN(0, "Most likely not an RTSP packet sock_id: %d sid: %d rlen: %d, dropping ....", s->sock_id, s->sid, rlen); 
	
//	if(strstr(arg[1], "freq") || strstr(arg[1], "pids"))
	sid = (streams *) setup_stream (arg[1], s);
	
	for (i = 0; i < la; i++)
		if (strncasecmp ("CSeq:", arg[i], 5) == 0)
			cseq = atoi (arg[i + 1]);
	else if (strncasecmp ("Transport:", arg[i], 9) == 0){
		char *rtp_avp = strstr(arg[i], "RTP/AVP");
		int rv;
		if(!rtp_avp)
			rtp_avp = strstr(arg[i+1], "RTP/AVP");
		if(!rtp_avp)
			rtp_avp = strstr(arg[i+2], "RTP/AVP");		
		if(rtp_avp)
			rv = decode_transport (s, rtp_avp, opts.rrtp, opts.start_rtp);
		
		if(rv == -1)
		{
			http_response (s, 400, NULL, NULL, cseq, 0);
			return 0;
		}
	}else if (strstr (arg[i], "LIVE555"))
		if(sid)
		{
			LOG("VLC detected, setting stream timeout to unlimited for sid %d", sid->sid);
			sid->timeout = 0; // ignore timeout for VLC
		}
	
	if((strncasecmp (arg[0], "PLAY", 4) == 0) || (strncasecmp (arg[0], "GET", 3) == 0) || (strncasecmp (arg[0], "SETUP", 5) == 0)) 
	{
		char ra[100];
		int rv;
			
		if (!( sid = get_sid(s->sid)))
		{
			http_response (s, 503, NULL, NULL, cseq, 0);
			return 0;
		}

		if ((strncasecmp (arg[0], "PLAY", 3) == 0) || (strncasecmp (arg[0], "GET", 3) == 0))
			if ((rv = start_play (sid, s)) < 0)
			{
				http_response (s, -rv , NULL, NULL, cseq, 0);
				return 0;
			}
		strcpy(ra, inet_ntoa (sid->sa.sin_addr));
		if(sid->type == STREAM_RTSP_UDP)
			if (atoi (ra) < 239)
				snprintf (buf, sizeof(buf),
					"Transport: RTP/AVP;unicast;source=%s;client_port=%d-%d;server_port=%d-%d\r\nSession: %010d;timeout=%d\r\ncom.ses.streamID: %d",
					get_sock_host (s->sock), ntohs (sid->sa.sin_port), ntohs (sid->sa.sin_port) + 1,
					get_sock_port (sid->rsock), get_sock_port (sid->rsock) + 1, get_session_id (s->sid),
					sid->timeout?sid->timeout / 1000:opts.timeout_sec / 1000, sid->sid + 1);
			else
				snprintf (buf, sizeof(buf),
					"Transport: RTP/AVP;multicast;destination=%s;port=%d-%d\r\nSession: %010d;timeout=%d\r\ncom.ses.streamID: %d",
					ra, ntohs (sid->sa.sin_port), ntohs (sid->sa.sin_port) + 1,
					get_session_id (s->sid), sid->timeout?sid->timeout / 1000:opts.timeout_sec / 1000 , sid->sid + 1);
		else if(sid->type == STREAM_HTTP)
			snprintf(buf, sizeof(buf), "Content-Type: video/mp2t");
		else 
			snprintf(buf, sizeof(buf), "Transport: RTP/AVP/TCP;interleaved=0-1\r\nSession: %010d;timeout=%d\r\ncom.ses.streamID: %d", get_session_id (s->sid), sid->timeout?sid->timeout / 1000:opts.timeout_sec / 1000, sid->sid + 1);

		if (strncasecmp(arg[0], "PLAY", 4) == 0)
			snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf) - 1,  "\r\nRTP-Info: url=%s;seq=%d", arg[1], 0);
		http_response (s, 200, buf, NULL, cseq, 0);
	}
	else if (strncmp (arg[0], "TEARDOWN", 8) == 0)
	{
		close_stream (s->sid);
		http_response (s, 200, NULL, NULL, cseq, 0);
	}
	else
	{
		if (strncmp (arg[0], "DESCRIBE", 8) == 0)
		{
			char sbuf[1000];
			describe_streams(s, arg[1], sbuf, sizeof(sbuf));
			snprintf(buf, sizeof(buf), "Content-type: application/sdp\r\nContent-Base: rtsp://%s/", get_sock_host(s->sock));
			http_response (s, 200, buf, sbuf, cseq, 0);
				
		}
		else if (strncmp (arg[0], "OPTIONS", 8) == 0)
		{		
			if(cseq<3)         // fix for tivizen
			{
				for (i = 0; i < la; i++)
					if (strncasecmp ("Session:", arg[i], 5) == 0)      
						set_session_id(sid->sid, map_int(arg[i+1], NULL));
						
			}
			snprintf(buf, sizeof(buf), "Session:%010d\r\n%s", get_session_id(s->sid), public);
			http_response (s, 200, buf, NULL, cseq, 0);
		}
	}
	return 0;
}

#define REPLY_AND_RETURN(c) {http_response (s, c, NULL, NULL, 0, 0);return 0;}

char uuid[100];
int bootid, uuidi;
struct sockaddr_in ssdp_sa;

int
read_http (sockets * s)
{
	char buf[2000];
	char *arg[50];
	int la, rlen;
	char *xml =
		"<?xml version=\"1.0\"?>\r\n"
		"<root xmlns=\"urn:schemas-upnp-org:device-1-0\" configId=\"0\">\r\n"
		"<specVersion>\r\n"
		"<major>0</major>\r\n"
		"<minor>1</minor>\r\n"
		"</specVersion>\r\n"
		"<device>\r\n"
		"<deviceType>urn:ses-com:device:SatIPServer:1</deviceType>\r\n"
		"<friendlyName>minisatip</friendlyName>\r\n"
		"<manufacturer>cata</manufacturer>\r\n"
		"<manufacturerURL>http://github.com/catalinii/minisatip</manufacturerURL>\r\n"
		"<modelDescription>minisatip for Linux</modelDescription>\r\n"
		"<modelName>minisatip</modelName>\r\n"
		"<modelNumber>0001</modelNumber>\r\n"
		"<modelURL>http://github.com/catalinii/minisatip</modelURL>\r\n"
		"<serialNumber>0001</serialNumber>\r\n"
		"<UDN>uuid:%s</UDN>\r\n"
		"<UPC>Universal Product Code</UPC>\r\n"
		"<iconList>\r\n"
 		"<icon>\r\n"
 		"<mimetype>image/png</mimetype>\r\n"
 		"<width>48</width>\r\n"
 		"<height>48</height>\r\n"
 		"<depth>24</depth>\r\n"
 		"<url>/icons/sm.png</url>\r\n"
 		"</icon><icon>\r\n"
 		"<mimetype>image/png</mimetype>\r\n"
 		"<width>120</width>\r\n"
 		"<height>120</height>\r\n"
 		"<depth>24</depth>\r\n"
 		"<url>/icons/lr.png</url>\r\n"
 		"</icon>\r\n"
 		"<icon>\r\n"
 		"<mimetype>image/jpeg</mimetype>\r\n"
 		"<width>48</width>\r\n"
 		"<height>48</height>\r\n"
 		"<depth>24</depth>\r\n"
 		"<url>/icons/sm.jpg</url>\r\n"
 		"</icon>\r\n"
 		"<icon>\r\n"
 		"<mimetype>image/jpeg</mimetype>\r\n"
 		"<width>120</width>\r\n"
 		"<height>120</height>\r\n"
 		"<depth>24</depth>\r\n"
 		"<url>/icons/lr.jpg</url>\r\n"
 		"</icon>\r\n"
 		"</iconList>\r\n"
		"<presentationURL>http://github.com/catalinii/minisatip</presentationURL>\r\n"
		"<satip:X_SATIPCAP xmlns:satip=\"urn:ses-com:satip\">%s</satip:X_SATIPCAP>\r\n"
		"%s"
		"</device>\r\n"
		"</root>\r\n";

	if (s->rlen < 5
		|| (htonl (*(uint32_t *) & s->buf[s->rlen - 4]) != 0x0D0A0D0A))
	{
		if( s->rlen > RBUF - 10 )
		{
			LOG("Discarding %d bytes from the socket buffer, request > %d, consider increasing  RBUF", s->rlen, RBUF);
			s->rlen = 0;
		}
		if ( s->flags & 1 ) 
			return 0;		
		unsigned char *new_alloc = malloc1 (RBUF);
		memcpy(new_alloc, s->buf, s->rlen);
		s->buf = new_alloc;
		s->flags = s->flags | 1;
		return 0;
	}

	if (strncasecmp(s->buf,"GET ",4)==0 && strstr(s->buf, "/?" ))
	{
		read_rtsp(s);
		return 0;
	}

	rlen = s->rlen;
	s->rlen = 0;
	
	LOG ("read HTTP from %d sid: %d: %s", s->sock, s->sid, s->buf);
	
	la = split (arg, s->buf, 50, ' ');
	//      LOG("args: %s -> %s -> %s",arg[0],arg[1],arg[2]);
	if (strncmp (arg[0], "GET", 3) != 0)
		REPLY_AND_RETURN(503);
	if (uuidi == 0)
		ssdp_discovery (s);

	if (strncmp (arg[1], "/desc.xml", 9) == 0)
	{
		int tuner_s2, tuner_t, tuner_c;
		char adapters[50];
		tuner_s2 = getS2Adapters ();
		tuner_t = getTAdapters ();
		tuner_c = getCAdapters ();
		
		memset(adapters, 0, sizeof(adapters));
		if(tuner_s2)sprintf(adapters, "DVBS2-%d,", tuner_s2);
		if(tuner_t)sprintf(adapters + strlen(adapters), "DVBT2-%d,", tuner_t);
		if(tuner_c)sprintf(adapters + strlen(adapters), "DVBC-%d,", tuner_c);
		if(tuner_s2 + tuner_t + tuner_c == 0)
			strcpy(adapters,"DVBS2-0,");
		adapters[strlen(adapters)-1] = 0;
		sprintf (buf, xml, uuid, adapters, opts.playlist);
		http_response (s, 200, "Content-type: text/xml\r\nConnection: close", buf, 0, 0);
		return 0;
	}

	if (strncmp (arg[1], "/icons/", 7) == 0)
	{
		char *ctype = NULL;
		int nl = sizeof(buf);
		if(strlen (arg[1]) != 13 )
			REPLY_AND_RETURN(404);		
		if( arg[1][strlen(arg[1])-2] == 'n' )
			ctype = "Content-type: image/png\r\nConnection: close";
		else if (arg[1][strlen(arg[1])-2] == 'p')			
			ctype = "Content-type: image/jpeg\r\nConnection: close";
		else REPLY_AND_RETURN(404); 
		
		readfile(arg[1], buf, &nl);
		if(nl == 0)
			http_response (s, 404, NULL, NULL, 0, 0);
		else
			http_response (s, 200, ctype, buf, 0, nl);
		return 0;
	}	
	
	http_response (s, 404, NULL, NULL, 0, 0);
	return 0;
}


int
close_http (sockets * s)
{
	if (s->flags & 1  && s->buf)
		free1 (s->buf);
	s->flags = 0;
	s->buf = NULL;
	LOG ("Closing stream %d", s->sid);
	close_stream (s->sid);
	return 0;
}


int
ssdp_discovery (sockets * s)
{
	char *reply = "NOTIFY * HTTP/1.1\r\n"
		"HOST: %s:1900\r\n"
		"CACHE-CONTROL: max-age=1800\r\n"
		"LOCATION: http://%s/desc.xml\r\n"
		"NT: upnp:rootdevice\r\n"
		"NTS: ssdp:alive \r\n"
		"SERVER: Linux/1.0 UPnP/1.1 minisatip/%s\r\n"
		"USN: uuid:%s::upnp:rootdevice\r\n"
		"BOOTID.UPNP.ORG: %d\r\n"
		"CONFIGID.UPNP.ORG: 0\r\n" "DEVICEID.SES.COM: %d\r\n\r\n\0";
	char buf[500],
		mac[15] = "00000000000000";
	char uuid1[] = "11223344-9999-0000-b7ae";
	socklen_t salen;
	int i;

	if (uuidi == 0)
	{
		uuidi = 1;
		get_mac (mac);
		sprintf (uuid, "%s-%s", uuid1, mac);
		fill_sockaddr (&ssdp_sa, opts.disc_host, 1900);
	}

	if(s->type != TYPE_UDP) return 0;
	sprintf (buf, reply, opts.disc_host, opts.http_host, VERSION, uuid, bootid, opts.device_id);
	salen = sizeof (ssdp_sa);
	LOG ("ssdp_discovery fd: %d -> %s", s->sock, buf);
	for (i = 0; i < 3; i++)
		sendto (s->sock, buf, strlen (buf), MSG_NOSIGNAL,
			(const struct sockaddr *) &ssdp_sa, salen);
	s->rtime = getTick ();
	return 0;
}


int ssdp;
int
ssdp_reply (sockets * s)
{
	char *reply = "HTTP/1.1 200 OK\r\n"
		"CACHE-CONTROL: max-age=1800\r\n"
		"DATE: %s\r\n"
		"EXT:\r\n"
		"LOCATION: http://%s/desc.xml\r\n"
		"SERVER: Linux/1.0 UPnP/1.1 minisatip/%s\r\n"
		"ST: urn:ses-com:device:SatIPServer:1\r\n"
		"USN: uuid:%s::urn:ses-com:device:SatIPServer:1\r\n"
		"BOOTID.UPNP.ORG: %d\r\n"
		"CONFIGID.UPNP.ORG: 0\r\n" "DEVICEID.SES.COM: %d\r\n\0";
	socklen_t salen;

	if (strncmp (s->buf, "M-SEARCH", 8) != 0)
	{
		//      LOG("not an M-SEARCH, ignoring");
		return 0;
	}
	if (uuidi == 0)
		ssdp_discovery (s);
	char buf[500];

	sprintf (buf, reply, get_current_timestamp (), opts.http_host, VERSION, uuid, bootid, opts.device_id);
	salen = sizeof (s->sa);
	LOG ("ssdp_reply fd: %d -> %s\n%s", ssdp, inet_ntoa (s->sa.sin_addr), buf);
								 //use ssdp (unicast) even if received to multicast address
	sendto (ssdp, buf, strlen (buf), MSG_NOSIGNAL, (const struct sockaddr *) &s->sa, salen);
	return 0;
}


int
new_rtsp (sockets * s)
{
	s->type = TYPE_RTSP;
	s->action = (socket_action) read_rtsp;
	s->close = (socket_action) close_http;
	return 0;
}


int
new_http (sockets * s)
{
	s->type = TYPE_HTTP;
	s->action = (socket_action) read_http;
	s->close = (socket_action) close_http;
	return 0;
}


char pn[200];

int
main (int argc, char *argv[])
{
	int rtsp, http, si, si1, ssdp1;
	strncpy(pn,argv[0],sizeof(pn));
	set_signal_handler ();
	set_options (argc, argv);
	if (opts.daemon)
		becomeDaemon ();
	bootid = readBootID();
	printf("Starting minisatip version %s, dvbapi version: %04X\n",VERSION, DVBAPIVERSION);
	if ((ssdp = udp_bind (NULL, 1900)) < 1)
		FAIL ("SSDP: Could not bind on udp port 1900");
	if ((ssdp1 = udp_bind (opts.disc_host, 1900)) < 1)
		FAIL ("SSDP: Could not bind on %s udp port 1900", opts.disc_host);
	if ((rtsp = tcp_listen (NULL, 554)) < 1)
		FAIL ("RTSP: Could not listen on port 554");
	if ((http = tcp_listen (NULL, opts.http_port)) < 1)
		FAIL ("Could not listen on http port %d", opts.http_port);
	if ((rtp = udp_bind (NULL, opts.start_rtp)) < 1)
		FAIL ("SSDP: Could not bind on rtp port %d", opts.start_rtp);
	if ((rtpc = udp_bind (NULL, opts.start_rtp + 1)) < 1)
		FAIL ("SSDP: Could not listen on rtpc port %d", opts.start_rtp + 1);

	si =
		sockets_add (ssdp, NULL, -1, TYPE_UDP, (socket_action) ssdp_reply, NULL,
		(socket_action) ssdp_discovery);
	si1 =
		sockets_add (ssdp1, NULL, -1, TYPE_UDP, (socket_action) ssdp_reply, NULL,
		(socket_action) ssdp_discovery);
	if (si < 0 || si1 < 0)
		FAIL ("sockets_add failed for ssdp");

	s[si].close_sec = 1800 * 1000;
	s[si].rtime = -s[si].close_sec;
	if (0 >
		sockets_add (rtsp, NULL, -1, TYPE_SERVER, (socket_action) new_rtsp,
		NULL, (socket_action) close_http))
		FAIL ("sockets_add failed for rtsp");
	if (0 >
		sockets_add (http, NULL, -1, TYPE_SERVER, (socket_action) new_http,
		NULL, (socket_action) close_http))
		FAIL ("sockets_add failed for http");
	printf ("Initializing with %d devices\n", init_hw ());
	select_and_execute ();
	free_all ();
	return 0;
}


int addr2line(char const * const program_name, void const * const addr)
{
	char addr2line_cmd[512] = {0};

	sprintf(addr2line_cmd,"addr2line -f -p -e %.256s %p", program_name, addr);
	return system(addr2line_cmd);
}


void
print_trace (void)
{
	void *array[10];
	size_t size;
	char **strings;
	size_t i;
	#ifndef __mips__

	size = backtrace (array, 10);

	printf ("Obtained %zd stack frames.\n", size);

	for (i = 0; i < size; i++)
	{
		printf("%p : ", array[i]);
		fflush(stdout);
		if(addr2line(pn, array[i])) printf("\n");
	}
	#else
	printf( " No backtrace defined\n");
	#endif
}


extern int run_loop;
void
posix_signal_handler (int sig, siginfo_t * siginfo, ucontext_t * ctx)
{
	int sp = 0, ip = 0;

	if (sig == SIGINT)
	{
		run_loop = 0;
		return;
	}
	#ifdef __mips__
	sp = ctx->uc_mcontext.gregs[29];
	ip = ctx->uc_mcontext.pc;
	#endif
	printf
		("RECEIVED SIGNAL %d - SP=%lX IP=%lX main=%lX read_dmx=%lX clock_gettime=%lX\n",
		sig, (long unsigned int) sp, (long unsigned int) ip,
		(long unsigned int) main, (long unsigned int) read_dmx,
		(long unsigned int) clock_gettime);
	fflush(stdout);
	#ifdef __mips__
	hexDump ("Stack dump: ", (void *)sp, 128);
	#endif
	print_trace();
	exit (1);
}


int								 /* Returns 0 on success, -1 on error */
becomeDaemon ()
{
	int maxfd, fd;
	struct stat sb;
	
	switch (fork ())
	{							 /* Become background process */
		case -1:
			return -1;
		case 0:
			break;				 /* Child falls through... */
		default:
			_exit (EXIT_SUCCESS);/* while parent terminates */
	}

	if (setsid () == -1)		 /* Become leader of new session */
		return -1;

	switch (fork ())
	{							 /* Ensure we are not session leader */
		case -1:
			return -1;
		case 0:
			break;
		default:
			_exit (EXIT_SUCCESS);
	}

	umask (0);					 /* Clear file mode creation mask */

	maxfd = sysconf (_SC_OPEN_MAX);
	if (maxfd == -1)			 /* Limit is indeterminate... */
		maxfd = 1024;			 /* so take a guess */

	for (fd = 0; fd < maxfd; fd++)
		close (fd);

	close (STDIN_FILENO);		 /* Reopen standard fd's to /dev/null */
	//	chdir ("/tmp");				 /* Change to root directory */
	
	fd = open ("/dev/null", O_RDWR);

	if (fd != STDIN_FILENO)		 /* 'fd' should be 0 */
		return -1;
	if ( stat ("/tmp/log", &sb ) == -1)
		fd = open ("/tmp/log", O_RDWR | O_CREAT, 0666);
	else
		fd = open ("/tmp/log", O_RDWR | O_APPEND);
	if (fd != STDOUT_FILENO)	 /* 'fd' should be 1 */
		return -1;
	if (dup2 (STDOUT_FILENO, STDERR_FILENO) != STDERR_FILENO)
		return -1;

	return 0;
}


void *
mymalloc (int a, char *f, int l)
{
	void *x = malloc (a);
	printf ("%s:%d allocation_wrapper malloc allocated %d bytes at %p\n", f, l, a, x);
	fflush (stdout);
	return x;
}


void
myfree (void *x, char *f, int l)
{
	printf ("%s:%d allocation_wrapper free called with argument %p", f, l, x);
	fflush (stdout);
	free (x);
	puts (" - done free");
	fflush (stdout);
}


int readBootID()
{
	int bid=0;
	char buf[20];
	FILE *f=fopen("bootid","rt");
	if(f)
	{
		fgets(buf,sizeof(buf),f);
		fclose(f);
		buf[sizeof(f)-1]=0;
		bid = map_int(buf,NULL);
	}
	bid++;
	f=fopen("bootid","wt");
	if(f)
	{
		fprintf(f,"%d",bid);
		fclose(f);
	}
	return bid;
}


int readfile(char *fn,char *buf,int *len)
{
	char ffn[500];
	char *path[]={".","/usr/share/minisatip",NULL};
	int fd,i,nl=0;
	for( i=0; path[i]; i++)
	{
		strcpy(ffn, path[i]);
		strncat(ffn, fn, sizeof(ffn)-strlen(path[i])-1);
		ffn[sizeof(ffn)-1] = 0;
		if((fd = open(ffn,O_RDONLY))<0)
			continue;
		
		nl = read(fd, buf, *len);
		close(fd);
		LOG("opened %s fd %d and read %d bytes from it", ffn, fd, nl);
	}
	*len = nl;
	buf[nl] = 0;
	
	return nl;
}
