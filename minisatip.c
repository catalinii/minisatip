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

#include "minisatip.h"
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <sys/ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
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
#include <syslog.h>
#include "socketworks.h"
#include "stream.h"
#include "adapter.h"
#include "dvb.h"
#include "tables.h"

struct struct_opts opts;

#define DESC_XML "desc.xml"
#define PID_NAME "/var/run/%s.pid"
char version[] = VERSION;
char app_name[] = "minisatip";
char pid_file[50];
extern sockets s[MAX_SOCKS];
char public[] = "Public: OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN";
int rtsp, http, si, si1, ssdp1;

static const struct option long_options[] =
{
{ "remote-rtp", required_argument, NULL, 'r' },
{ "device-id", required_argument, NULL, 'D' },
{ "check-signal", no_argument, NULL, 'z' },
{ "clean-psi", no_argument, NULL, 't' },
{ "log", no_argument, NULL, 'l' },
{ "buffer", required_argument, NULL, 'b' },
{ "enable-adapters", required_argument, NULL, 'e' },
{ "unicable", required_argument, NULL, 'u' },
{ "jess", required_argument, NULL, 'j' },
{ "diseqc", required_argument, NULL, 'd' },
{ "diseqc-timing", required_argument, NULL, 'q' },
#ifndef DISABLE_DVBAPI
		{ "dvbapi", required_argument, NULL, 'o' },
#endif
#ifndef DISABLE_SATIPCLIENT
		{ "satip-servers", required_argument, NULL, 's' },
#endif
#ifndef DISABLE_NETCVCLIENT
		{ "netceiver", required_argument, NULL, 'n' },
#endif
		{ "rtsp-port", required_argument, NULL, 'y' },
		{ "http-port", required_argument, NULL, 'x' },
		{ "http-host", required_argument, NULL, 'w' },
		{ "slave", required_argument, NULL, 'S' },
		{ "delsys", required_argument, NULL, 'Y' },
		{ "priority", required_argument, NULL, 'i' },
		{ "document-root", required_argument, NULL, 'R' },
		{ "threads", no_argument, NULL, 'T' },
		{ "dmx-source", required_argument, NULL, '9' },
		{ "xml", required_argument, NULL, 'X' },
		{ "help", no_argument, NULL, 'h' },
		{ "version", no_argument, NULL, 'V' },
		{ 0, 0, 0, 0 } };

#define RRTP_OPT 'r'
#define DEVICEID_OPT 'D'
#define HTTPSERVER_OPT 'w'
#define HTTPPORT_OPT 'x'
#define LOG_OPT 'l'
#define HELP_OPT 'h'
#define PLAYLIST_OPT 'p'
#define DVBS2_ADAPTERS_OPT 'a'
#define CLEANPSI_OPT 't'
#define MAC_OPT 'm'
#define FOREGROUND_OPT 'f'
#define BW_OPT 'c'
#define DVRBUFFER_OPT 'b'
#define ENABLE_ADAPTERS_OPT 'e'
#define UNICABLE_OPT 'u'
#define JESS_OPT 'j'
#define DISEQC_OPT 'd'
#define DISEQC_TIMING_OPT 'q'
#define SLAVE_OPT 'S'
#define DELSYS_OPT 'Y'
#define DVBAPI_OPT 'o'
#define SYSLOG_OPT 'g'
#define RTSPPORT_OPT 'y'
#define SATIPCLIENT_OPT 's'
#define NETCVCLIENT_OPT 'n'
#define PRIORITY_OPT 'i'
#define PRIORITY_OPT 'i'
#define DOCUMENTROOT_OPT 'R'
#define XML_OPT 'X'
#define THREADS_OPT 'T'
#define DMXSOURCE_OPT '9'

char *built_info[] =
{
#ifdef DISABLE_DVBCSA
		"Built without dvbcsa",
#else 
		"Built with dvbcsa",
#endif
#ifdef DISABLE_DVBAPI
		"Built without dvbapi",
#else 
		"Built with dvbapi",
#endif
#ifdef DISABLE_DVBAES
		"Built without AES (OpenSSL)",
#else 
		"Built with AES (OpenSSL)",
#endif
#ifdef DISABLE_TABLES
		"Built without tables processing",
#else 
		"Built with tables processing",
#endif
#ifdef DISABLE_SATIPCLIENT
		"Built without satip client",
#else 
		"Built with satip client",
#endif
#ifdef DISABLE_LINUXDVB
		"Built without linux dvb client",
#else 
		"Built with linux dvb client",
#endif
#ifdef NO_BACKTRACE
		"Built without backtrace",
#else 
		"Built with backtrace",
#endif
#ifdef DISABLE_NETCVCLIENT
		"Built without netceiver",
#else 
		"Built with netceiver",
#endif
		NULL };

void print_version(int use_log)
{
	char buf[200];
	int i, len = 0;
	memset(buf, 0, sizeof(buf));
	len += sprintf(buf, "%s version %s, compiled with s2api version: %04X",
			app_name, version, LOGDVBAPIVERSION);
	if (!use_log)
		puts(buf);
	else
		LOGL(0, buf);
	for (i = 0; built_info[i]; i++)
		LOGL(0, "%s", built_info[i]);
}

void usage()
{
	print_version(0);
	printf(
			"\n\t./%s [-[fgltz]] [-a x:y:z] [-b X:Y] [-c X] [-d A:C-U ] [-D device_id] [-e X-Y,Z] [-i prio] \n\
	\t[-[uj] A1:S1-F1[-PIN]] [-m mac]"
#ifndef DISABLE_DVBAPI
					"[-o oscam_host:dvbapi_port] "
#endif
			"[-p public_host] [-r remote_rtp_host] \n\
	\t[-R document_root] "
#ifndef DISABLE_SATIPCLIENT
			"[-s [DELSYS:]host[:port] "
#endif
			"[-u A1:S1-F1[-PIN]] [-w http_server[:port]] \n\
	\t[-x http_port] [-X xml_path] [-y rtsp_port] \n\n\
Help\n\
-------\n\
\n\
* -a x:y:z simulate x DVB-S2, y DVB-T2 and z DVB-C adapters on this box (0 means auto-detect)\n\
	* eg: -a 1:2:3  \n\
	- it will report 1 dvb-s2 device, 2 dvb-t2 devices and 3 dvb-c devices \n\
\n\
* -b --buffers X:Y : set the app adapter buffer to X Bytes (default: %d) and set the kernel DVB buffer to Y Bytes (default: %d) - both multiple of 188\n\
	* eg: -b 18800:18988\n\
\n\
* -c X: bandwidth capping for the output to the network [default: unlimited]\n\
	* eg: -c 2048  (does not allow minisatip to send more than 2048KB/s to all remote servers)\n\
\n\
* -d --diseqc ADAPTER1:COMMITTED1-UNCOMMITTED1[,ADAPTER2:COMMITTED2-UNCOMMITTED2[,...]\n\
\t* The first argument is the adapter number, second is the number of committed packets to send to a Diseqc 1.0 switch, third the number of uncommitted commands to sent to a Diseqc 1.1 switch\n\
\tThe higher number between the committed and uncommitted will be sent first.\n\
	* eg: -d 0:1-0  (which is the default for each adapter).\n\
	- note: * as adapter means apply to all adapters\n\
	- note: * before committed number enables fast-switch (only voltage/tone)\n\
\n\
* -q --diseqc-timing ADAPTER1:BEFORE_CMD1-AFTER_CMD1-AFTER_REPEATED_CMD1-AFTER_SWITCH1-AFTER_BURST1-AFTER_TONE1[,...]\n\
\t* All timing values are in ms, default adapter values are: 15-54-15-15-15-0\n\
	- note: * as adapter means apply to all adapters\n\
\n\
* -D --device-id DVC_ID: specify the device id (in case there are multiple SAT>IP servers in the network)\n \
	* eg: -D 4 \n\
\n\
* -Y --delsys ADAPTER1:DELIVERY_SYSTEM1[,ADAPTER2:DELIVERY_SYSTEM2[,..]] - specify the delivery system of the adapters	\n\
	* eg: --delsys 1:dvbt,2:dvbs\n\
	- specifies adapter 1 as a DVBT device, adapter 2 as DVB-S, which overrides the system detection of the adapter\n\
\n\
* --dmx-source ADAPTER1:FRONTENDX - specifies the frontend number specified as argument for DMX_SET_SOURCE \n\
	* eg: --dmx-source 0:1 - enables DMX_SET_SOURCE ioctl call with parameter 1 for adapter 0\n\
\n\
* -e --enable-adapters list_of_enabled adapters: enable only specified adapters\n\
	* eg: -e 0-2,5,7 (no spaces between parameters)\n\
	- keep in mind that the first adapters are the local ones starting with 0 after that are the satip adapters \n\
	if you have 3 local dvb cards 0-2 will be the local adapters, 3,4, ... will be the satip servers specified with argument -s\n\
\n\
* -f  foreground, otherwise run in background\n\
\n\
* -g use syslog instead stdout for logging, multiple -g - print to stderr as well\n\
\n\
* -i --priority prio: set the DVR thread priority to prio \n\
\n\
* -l increases the verbosity (you can use multiple -l), logging to stdout in foreground mode or in /tmp/log when a daemon\n\
	* eg: -l -l -l\n\
\n\
* -m xx: simulate xx as local mac address, generates UUID based on mac\n\
	* eg: -m 001122334455 \n\
\n\
"
#ifndef DISABLE_NETCVCLIENT
			"\
* -n --netceiver if:count: use network interface <if> (default vlan4) and look for <count> netceivers\n\
	* eg: -n vlan4:2 \n\
\n\
"
#endif
#ifndef DISABLE_DVBAPI
			"\
* -o --dvbapi host:port - specify the hostname and port for the dvbapi server (oscam) \n\
	* eg: -o 192.168.9.9:9000 \n\
	192.168.9.9 is the host where oscam is running and 9000 is the port configured in dvbapi section in oscam.conf\n\
\n\
"
#endif
			"\
* -p url: specify playlist url using X_SATIPM3U header \n\
	* eg: -p http://192.168.2.3:8080/playlist\n\
	- this will add X_SATIPM3U tag into the satip description xml\n\
\n\
* -r --remote-rtp  remote_rtp_host: send the rtp stream to remote_rtp_host instead of the ip the connection comes from\n \
	* eg: -r 192.168.7.9\n \
\n\
* -R --document-root directory: document root for the minisatip web page and images\n\
\n\
"
#ifndef DISABLE_SATIPCLIENT
			"\
* -s --satip-servers DELSYS:host:port - specify the remote satip host and port with delivery system DELSYS, it is possible to use multiple -s \n\
	* DELSYS - can be one of: dvbs, dvbs2, dvbt, dvbt2, dvbc, dvbc2, isdbt, atsc, dvbcb ( - DVBC_ANNEX_B ) [default: dvbs2]\n\
	host - the server of the satip server\n\
	port - rtsp port for the satip server [default: 554]\n\
	eg: -s 192.168.1.2 -s dvbt:192.168.1.3:554 -s dvbc:192.168.1.4\n\
	- specifies 1 dvbs2 (and dvbs)satip server with address 192.168.1.2:554\n\
	- specifies 1 dvbt satip server  with address 192.168.1.3:554\n\
	- specifies 1 dvbc satip server  with address 192.168.1.4:554\n\
\n\
"
#endif
			"\
* -S --slave ADAPTER1,ADAPTER2-ADAPTER4[,..] - specify slave adapters	\n\
	* Allows specifying bonded adapters (multiple adapters connected with a splitter to the same LNB)\n\
	Only one adapter needs to be master all others needs to have this parameter specified\n\
	eg: -S 1-2\n\
	- specifies adapter 1 to 2 as slave, in this case adapter 0 can be the master that controls the LNB\n\
	- the slave adapter will not control the LNB polarity or band, but it will just change the internal frequency to tune to a different transponder\n\
	- in this way the master will be responsible for changing the LNB polarity and band\n\
\n\
* -t --cleanpsi clean the PSI from all CA information, the client will see the channel as clear if decrypted successfully\n\
\n\
* -T --threads: enables/disable multiple threads (reduces memory consumptions) (default: %s)\n\
\n\
* -u --unicable unicable_string: defines the unicable adapters (A) and their slot (S), frequency (F) and optionally the PIN for the switch:\n\
\t* The format is: A1:S1-F1[-PIN][,A2:S2-F2[-PIN][,...]]\n\
	eg: 2:0-1284[-1111]\n\
\t* When * character is used before frequency, force 13V only for setup\n\
\n\
* -j --jess jess_string - same format as -u \n\
\n\
* -w --http-host http_server[:port]: specify the host and the port (if not 80) where the xml file can be downloaded from [default: default_local_ip_address:8080] \n\
	* eg: -w 192.168.1.1:8080 \n\
\n\
* -x --http-port port: port for listening on http [default: 8080]\n\
	* eg: -x 9090 \n\
\n\
* -X --xml PATH: the path to the xml that is provided as part of the satip protocol	\n\
	* by default desc.xml is provided by minisatip without needing an additional file, \n\
	however satip.xml is included if it needs to be customized\n\
\n\
* -y --rtsp-port rtsp_port: port for listening for rtsp requests [default: 554]\n\
	* eg: -y 5544 \n\
	- changing this to a port > 1024 removes the requirement for minisatip to run as root\n\
\n\
",
			app_name,
			ADAPTER_BUFFER,
			DVR_BUFFER, opts.no_threads ? "DISABLED" : "ENABLED");
	exit(1);
}

void set_options(int argc, char *argv[])
{
	int opt;
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
	opts.device_id = 0;
	opts.bootid = 0;
	opts.dvr_buffer = DVR_BUFFER;
	opts.adapter_buffer = ADAPTER_BUFFER;
	opts.file_line = 0;
	opts.dvbapi_port = 0;
	opts.dvbapi_host = NULL;
	opts.drop_encrypted = 1;
	opts.rtsp_port = 554;
	opts.clean_psi = 0;
	opts.satip_addpids = 1;
	opts.satip_setup_pids = 0;
	opts.output_buffer = 512 * 1024;
	opts.satip_servers[0] = 0;
	opts.document_root = "html";
	opts.xml_path = DESC_XML;
	opts.no_threads = 0;
	opts.th_priority = -1;
	opts.diseqc_before_cmd = 15;
	opts.diseqc_after_cmd = 54;
	opts.diseqc_after_repeated_cmd = 15;
	opts.diseqc_after_switch = 15;
	opts.diseqc_after_burst = 15;
	opts.diseqc_after_tone = 0;

#ifdef NO_BACKTRACE
	opts.no_threads = 1;
#endif
	memset(opts.playlist, 0, sizeof(opts.playlist));

	while ((opt = getopt_long(argc, argv,
			"flr:a:td:w:p:s:n:hc:b:m:p:e:x:u:j:o:gy:i:q:D:VR:S:TX:Y:",
			long_options, NULL)) != -1)
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
			strncpy(opts.mac, optarg, 12);
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
			opts.device_id = atoi(optarg);
			break;
		}

		case HTTPSERVER_OPT:
		{
			//                              int i=0;
			opts.http_host = optarg;
			break;
		}

		case LOG_OPT:
		{
			opts.log++;
			break;
		}

		case SYSLOG_OPT:
		{
			opts.slog++;
			break;
		}

		case HELP_OPT:
		{
			usage();
			exit(0);
		}

		case HTTPPORT_OPT:
		{
			opts.http_port = atoi(optarg);
			break;
		}

		case BW_OPT:
		{
			opts.bw = atoi(optarg) * 1024;
			break;
		}

		case DVRBUFFER_OPT:
		{
			sscanf(optarg, "%d:%d", &opts.adapter_buffer, &opts.dvr_buffer);
			opts.adapter_buffer = (opts.adapter_buffer / 188) * 188;
			if (opts.adapter_buffer < ADAPTER_BUFFER)
				opts.adapter_buffer = ADAPTER_BUFFER;
			if (opts.dvr_buffer == 0)
				opts.dvr_buffer = DVR_BUFFER;

			break;
		}

		case DVBS2_ADAPTERS_OPT:
		{
			sscanf(optarg, "%d:%d:%d", &opts.force_sadapter,
					&opts.force_tadapter, &opts.force_cadapter);
			break;
		}

		case CLEANPSI_OPT:
		{
			opts.clean_psi = 1;
			break;
		}

		case PLAYLIST_OPT:
		{
			snprintf(opts.playlist, sizeof(opts.playlist),
					"<satip:X_SATIPM3U xmlns:satip=\"urn:ses-com:satip\">%s</satip:X_SATIPM3U>\r\n",
					optarg);
			break;
		}

		case ENABLE_ADAPTERS_OPT:
		{
			enable_adapters(optarg);
			break;
		}

		case DMXSOURCE_OPT:
		{
			set_adapter_dmxsource(optarg);
			break;
		}

		case UNICABLE_OPT:
		{
			set_unicable_adapters(optarg, SWITCH_UNICABLE);
			break;
		}

		case JESS_OPT:
		{
			set_unicable_adapters(optarg, SWITCH_JESS);
			break;
		}

		case DISEQC_OPT:
		{
			set_diseqc_adapters(optarg);
			break;
		}

		case DISEQC_TIMING_OPT:
		{
			set_diseqc_timing(optarg);
			break;
		}

		case SLAVE_OPT:
		{
			set_slave_adapters(optarg);
			break;
		}

		case DELSYS_OPT:
		{
			set_adapters_delsys(optarg);
			break;
		}

		case DVBAPI_OPT:
		{
#ifdef DISABLE_DVBAPI
			LOGL(0, "%s was not compiled with DVBCSA support, please install libdvbcsa (libdvbcsa-dev in Ubuntu) and change the Makefile", app_name);
			exit (0);
#else
			char* sep1 = strchr(optarg, ':');
			if (sep1 != NULL)
			{
				*sep1 = 0;
				opts.dvbapi_host = optarg;
				opts.dvbapi_port = map_int(sep1 + 1, NULL);
			}
#endif
			break;
		}

		case RTSPPORT_OPT:
		{
			opts.rtsp_port = atoi(optarg);
			break;
		}

		case SATIPCLIENT_OPT:
#ifdef DISABLE_SATIPCLIENT
			LOGL(0, "%s was not compiled with satip client support, please change the Makefile", app_name);
			exit (0);

#endif
			if (strlen(optarg) + strlen(opts.satip_servers)
					> sizeof(opts.satip_servers))
				break;

			if (opts.satip_servers[0])
				sprintf(opts.satip_servers + strlen(opts.satip_servers), ",%s",
						optarg);
			else
				sprintf(opts.satip_servers, "%s", optarg);

			break;

		case NETCVCLIENT_OPT:
		{
#ifdef DISABLE_NETCVCLIENT
			LOGL(0, "%s was not compiled with netceiver client support, please change the Makefile", app_name);
			exit (0);
#else
			// parse network interface name and number of netceivers
			char* sep1 = strchr(optarg, ':');
			if (sep1 != NULL)
			{
				*sep1 = 0;
				opts.netcv_if = optarg;
				opts.netcv_count = map_int(sep1 + 1, NULL);
				break;
			}

			// default interface is vlan4 as it is used on the REEL
			opts.netcv_if = "vlan4";
			opts.netcv_count = map_int(optarg, NULL);
#endif
			break;
		}

		case PRIORITY_OPT:

			opts.th_priority = map_int(optarg, NULL);
			break;

		case DOCUMENTROOT_OPT:
			opts.document_root = optarg;
			break;

		case THREADS_OPT:
			opts.no_threads = 1 - opts.no_threads;
			break;

		case XML_OPT:
			while (*optarg > 0 && *optarg == '/')
				optarg++;
			if (*optarg > 0)
				opts.xml_path = optarg;
			else
				LOGL(0, "Not a valid path for the xml file")
			;
			break;
		}

	}

	if (opts.bw && (opts.bw < opts.adapter_buffer))
		opts.adapter_buffer = (opts.bw / 188) * 188;

	lip = getlocalip();
	if (!opts.http_host)
	{
		opts.http_host = (char *) malloc(MAX_HOST);
		sprintf(opts.http_host, "%s:%d", lip, opts.http_port);
	}
}

#define RBUF 4000

int read_rtsp(sockets * s)
{
	char *arg[50];
	int cseq, la, i, rlen;
	char *transport = NULL, *useragent = NULL;
	int sess_id = 0;
	char buf[2000];
	char tmp_ra[50];
	streams *sid = get_sid(s->sid);

	if (s->buf[0] == 0x24 && s->buf[1] < 2)
	{
		if (sid)
			sid->rtime = s->rtime;

		int rtsp_len = s->buf[2] * 256 + s->buf[3];
		LOG(
				"Received RTSP over tcp packet (sock_id %d, stream %d, rlen %d) packet len: %d, type %02X %02X discarding %s...",
				s->id, s->sid, s->rlen, rtsp_len, s->buf[4], s->buf[5],
				(s->rlen == rtsp_len + 4) ? "complete" : "fragment");
		if (s->rlen == rtsp_len + 4)
		{ // we did not receive the entire packet
			s->rlen = 0;
			return 0;
		}
	}

	if (s->rlen < 4 || !end_of_header(s->buf + s->rlen - 4))
	{
		if (s->rlen > RBUF - 10)
		{
			LOG(
					"Discarding %d bytes from the socket buffer, request > %d, consider increasing  RBUF",
					s->rlen, RBUF);
			s->rlen = 0;
		}
		LOG(
				"read_rtsp: read %d bytes from handle %d, sock_id %d, flags %d not ending with \\r\\n\\r\\n",
				s->rlen, s->sock, s->id, s->flags);
		if (s->flags & 1)
			return 0;
		unsigned char *new_alloc = malloc1(RBUF);
		memcpy(new_alloc, s->buf, s->rlen);
		s->buf = new_alloc;
		s->flags = s->flags | 1;
		return 0;
	}

	rlen = s->rlen;
	s->rlen = 0;

	LOG("read RTSP (from handle %d sock_id %d, len: %d, sid %d):", s->sock,
			s->id, s->rlen, s->sid);
	LOGL(3, "%s", s->buf);

	if ((s->type != TYPE_HTTP)
			&& (strncasecmp((const char*) s->buf, "GET", 3) == 0))
	{
		http_response(s, 404, NULL, NULL, 0, 0);
		return 0;
	}

	la = split(arg, (char*) s->buf, 50, ' ');
	cseq = 0;
	if (la < 2)
		LOG_AND_RETURN(0,
				"Most likely not an RTSP packet sock_id: %d sid: %d rlen: %d, dropping ....",
				s->id, s->sid, rlen);

	if (s->sid < 0)
		for (i = 0; i < la; i++)
			if (strncasecmp("Session:", arg[i], 8) == 0)
			{
				sess_id = map_int(header_parameter(arg, i), NULL);
				s->sid = find_session_id(sess_id);
			}

	if (strstr(arg[1], "freq") || strstr(arg[1], "pids"))
	{
		sid = (streams *) setup_stream(arg[1], s);
	}

	//setup empty stream, removing this breaks satip tests
	if (!get_sid(s->sid)
			&& ((strncasecmp(arg[0], "PLAY", 4) == 0)
					|| (strncasecmp(arg[0], "GET", 3) == 0)
					|| (strncasecmp(arg[0], "SETUP", 5) == 0)))
		sid = (streams *) setup_stream(arg[1], s);

	sid = get_sid(s->sid);
	if (sid)
		sid->rtime = s->rtime;

	if (sess_id)
		set_session_id(s->sid, sess_id);

	for (i = 0; i < la; i++)
		if (strncasecmp("CSeq:", arg[i], 5) == 0)
			cseq = map_int(header_parameter(arg, i), NULL);
		else if (strncasecmp("Transport:", arg[i], 9) == 0)
		{
			transport = header_parameter(arg, i);

			if (-1 == decode_transport(s, transport, opts.rrtp, opts.start_rtp))
			{
				http_response(s, 400, NULL, NULL, cseq, 0);
				return 0;
			}
		}
		else if (strstr(arg[i], "LIVE555"))
		{
			if (sid)
				sid->timeout = 0;
		}
		else if (strstr(arg[i], "Lavf"))
		{
			if (sid)
				sid->timeout = 0;
		}
		else if (strncasecmp("User-Agent:", arg[i], 10) == 0)
			useragent = header_parameter(arg, i);
		else if (!useragent && strncasecmp("Server:", arg[i], 10) == 0)
			useragent = header_parameter(arg, i);

	if ((strncasecmp(arg[0], "PLAY", 4) == 0)
			|| (strncasecmp(arg[0], "GET", 3) == 0)
			|| (strncasecmp(arg[0], "SETUP", 5) == 0))
	{
		char ra[100];
		int rv;

		if (!(sid = get_sid(s->sid)))
		{
			http_response(s, 454, NULL, NULL, cseq, 0);
			return 0;
		}

		if (useragent)
			strncpy(sid->useragent, useragent, sizeof(sid->useragent) - 1);

		if ((strncasecmp(arg[0], "PLAY", 3) == 0)
				|| (strncasecmp(arg[0], "GET", 3) == 0))
			if ((rv = start_play(sid, s)) < 0)
			{
				http_response(s, -rv, NULL, NULL, cseq, 0);
				return 0;
			}
		buf[0] = 0;
		if (transport)
		{
			int s_timeout;

			if (sid->timeout == 1)
				sid->timeout = opts.timeout_sec;

			s_timeout = (
					(sid->timeout > 20000) ? sid->timeout : opts.timeout_sec)
					/ 1000;
			get_stream_rhost(sid->sid, ra, sizeof(ra));

			switch (sid->type)
			{
			case STREAM_RTSP_UDP:
				if (atoi(ra) < 224)
					snprintf(buf, sizeof(buf),
							"Transport: RTP/AVP;unicast;destination=%s;source=%s;client_port=%d-%d;server_port=%d-%d\r\nSession: %010d;timeout=%d\r\ncom.ses.streamID: %d",
							ra, get_sock_shost(s->sock),
							get_stream_rport(sid->sid),
							get_stream_rport(sid->sid) + 1,
//							opts.start_rtp, opts.start_rtp + 1,
							get_sock_sport(sid->rsock),
							get_sock_sport(sid->rtcp), get_session_id(s->sid),
							s_timeout, sid->sid + 1);
				else
					snprintf(buf, sizeof(buf),
							"Transport: RTP/AVP;multicast;destination=%s;port=%d-%d\r\nSession: %010d;timeout=%d\r\ncom.ses.streamID: %d",
							ra, get_stream_rport(sid->sid),
							ntohs (sid->sa.sin_port) + 1,
							get_session_id(s->sid), s_timeout, sid->sid + 1);
				break;
			case STREAM_RTSP_TCP:
				snprintf(buf, sizeof(buf),
						"Transport: RTP/AVP/TCP;interleaved=0-1\r\nSession: %010d;timeout=%d\r\ncom.ses.streamID: %d",
						get_session_id(s->sid), s_timeout, sid->sid + 1);
				break;
			}
		}

		if (strncasecmp(arg[0], "PLAY", 4) == 0)
		{
			char *qm = strchr(arg[1], '?');
			if (qm)
				*qm = 0;
			if (buf[0])
				strcat(buf, "\r\n");

			snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf) - 1,
					"RTP-Info: url=%s;seq=%jd;rtptime=%jd\r\nRange: npt=0.000-",
					arg[1], getTick(), (getTickUs() / 1000000));
		}
		if (buf[0] == 0 && sid->type == STREAM_HTTP)
			snprintf(buf, sizeof(buf), "Content-Type: video/mp2t");
		http_response(s, 200, buf, NULL, cseq, 0);
	}
	else if (strncmp(arg[0], "TEARDOWN", 8) == 0)
	{
		buf[0] = 0;
		if (get_sid(s->sid))
			sprintf(buf, "Session: %010d", get_session_id(s->sid));
		close_stream(s->sid);
		http_response(s, 200, buf, NULL, cseq, 0);
	}
	else
	{
		if (strncmp(arg[0], "DESCRIBE", 8) == 0)
		{
			char sbuf[1000];
			char *rv = NULL;
			rv = describe_streams(s, arg[1], sbuf, sizeof(sbuf));
			if (!rv)
			{
				http_response(s, 404, NULL, NULL, cseq, 0);
				return 0;
			}
			snprintf(buf, sizeof(buf),
					"Content-type: application/sdp\r\nContent-Base: rtsp://%s/",
					get_sock_shost(s->sock));
			http_response(s, 200, buf, sbuf, cseq, 0);

		}
		else if (strncmp(arg[0], "OPTIONS", 8) == 0)
		{
//			if(!get_sid(s->sid))
//				http_response(s, 454, public, NULL, cseq, 0);
//			else
			http_response(s, 200, public, NULL, cseq, 0);
		}
	}
	return 0;
}

#define REPLY_AND_RETURN(c) {http_response (s, c, NULL, NULL, 0, 0);return 0;}

char uuid[100];
int uuidi;
struct sockaddr_in ssdp_sa;

int read_http(sockets * s)
{
	char *arg[50];
	char buf[2000]; // the XML should not be larger than 1400 as it will create problems
	static char *xml =
			"<?xml version=\"1.0\"?>"
					"<root xmlns=\"urn:schemas-upnp-org:device-1-0\" configId=\"0\">"
					"<specVersion><major>1</major><minor>1</minor></specVersion>"
					"<device><deviceType>urn:ses-com:device:SatIPServer:1</deviceType>"
					"<friendlyName>%s</friendlyName><manufacturer>cata</manufacturer>"
					"<manufacturerURL>http://github.com/catalinii/minisatip</manufacturerURL>"
					"<modelDescription>%s for Linux</modelDescription><modelName>%s</modelName>"
					"<modelNumber>1.1</modelNumber><modelURL></modelURL><serialNumber>1</serialNumber><UDN>uuid:%s</UDN>"
					"<iconList>"
					"<icon><mimetype>image/png</mimetype><width>48</width><height>48</height><depth>24</depth><url>/sm.png</url></icon>"
					"<icon><mimetype>image/png</mimetype><width>120</width><height>120</height><depth>24</depth><url>/lr.png</url></icon>"
					"<icon><mimetype>image/jpeg</mimetype><width>48</width><height>48</height><depth>24</depth><url>/sm.jpg</url></icon>"
					"<icon><mimetype>image/jpeg</mimetype><width>120</width><height>120</height><depth>24</depth><url>/lr.jpg</url></icon>"
					"</iconList>"
					"<presentationURL>http://%s/</presentationURL>\r\n"
					"<satip:X_SATIPCAP xmlns:satip=\"urn:ses-com:satip\">%s</satip:X_SATIPCAP>"
					"%s"
					"</device></root>";
	if (s->rlen < 5 || !end_of_header(s->buf + s->rlen - 4))
	{
		if (s->rlen > RBUF - 10)
		{
			LOG(
					"Discarding %d bytes from the socket buffer, request > %d, consider increasing  RBUF",
					s->rlen, RBUF);
			s->rlen = 0;
		}
		if (s->flags & 1)
			return 0;
		unsigned char *new_alloc = malloc1(RBUF);
		memcpy(new_alloc, s->buf, s->rlen);
		set_socket_buffer(s->id, new_alloc, RBUF);
		s->flags = s->flags | 1;
		return 0;
	}

	if (strncasecmp((const char*) s->buf, "GET ", 4) == 0
			&& strstr((const char*) s->buf, "/?"))
	{
		read_rtsp(s);
		return 0;
	}

	s->rlen = 0;

	LOG("read HTTP from %d sid: %d: ", s->sock, s->sid);
	LOGL(3, "%s", s->buf);

	split(arg, (char*) s->buf, 50, ' ');
//      LOG("args: %s -> %s -> %s",arg[0],arg[1],arg[2]);
	if (strncmp(arg[0], "GET", 3) != 0)
		REPLY_AND_RETURN(503);
	if (uuidi == 0)
		ssdp_discovery(s);

	sockets_timeout(s->id, 1); //close the connection 

	if (strcmp(arg[1], "/"DESC_XML) == 0)
	{
		extern int tuner_s2, tuner_t, tuner_c, tuner_t2, tuner_c2;
		char adapters[400];
		char headers[500];

		memset(adapters, 0, sizeof(adapters));
		if (tuner_s2)
			sprintf(adapters, "DVBS2-%d,", tuner_s2);
		if (tuner_t)
			sprintf(adapters + strlen(adapters), "DVBT-%d,", tuner_t);
		if (tuner_c)
			sprintf(adapters + strlen(adapters), "DVBC-%d,", tuner_c);
		if (tuner_t2)
			sprintf(adapters + strlen(adapters), "DVBT2-%d,", tuner_t2);
		if (tuner_c2)
			sprintf(adapters + strlen(adapters), "DVBC2-%d,", tuner_c2);
		if (tuner_s2 + tuner_t + tuner_c + tuner_t2 + tuner_c2 == 0)
			strcpy(adapters, "DVBS2-0,");
		adapters[strlen(adapters) - 1] = 0;
		snprintf(buf, sizeof(buf), xml, app_name, app_name, app_name, uuid,
				opts.http_host, adapters, opts.playlist);
		sprintf(headers,
				"CACHE-CONTROL: no-cache\r\nContent-type: text/xml\r\nX-SATIP-RTSP-Port: %d",
				opts.rtsp_port);
		http_response(s, 200, headers, buf, 0, 0);
		return 0;
	}
// process file from html directory, the images are just sent back

	if (!strcmp(arg[1], "/"))
		arg[1] = "/status.html";

	if (!strchr(arg[1] + 1, '/') && !strstr(arg[1], ".."))
	{
		char ctype[500];
		char *f;
		int nl;

		f = readfile(arg[1], ctype, &nl);
		if (!f)
		{
			http_response(s, 404, NULL, NULL, 0, 0);
			return 0;
		}
		if (strstr(ctype, "image") || strstr(ctype, "css")
				|| strstr(ctype, "javascript"))
		{
			http_response(s, 200, ctype, f, 0, nl);
			closefile(f, nl);
			return 0;
		}

		process_file(s, f, nl, ctype);
		closefile(f, nl);
		return 0;
	}

	http_response(s, 404, NULL, NULL, 0, 0);
	return 0;
}

int close_http(sockets * s)
{
	streams *sid = get_sid(s->sid);
	if ((s->flags & 1) && s->buf)
		free1(s->buf);
	s->flags = 0;
	s->buf = NULL;
	LOG("Requested stream close %d timeout %d type %d", s->sid,
			sid ? sid->timeout : -1, sid ? sid->type : -1);
	if (sid
			&& ((sid->type == STREAM_RTSP_UDP && sid->timeout != 0)
					|| (sid->type == 0 && sid->timeout != 0)))
// Do not close rtsp udp as most likely there was no TEARDOWN at this point
		return 0;
	close_stream(s->sid);
	return 0;
}

int ssdp_discovery(sockets * s)
{
	char *reply = "NOTIFY * HTTP/1.1\r\n"
			"HOST: %s:1900\r\n"
			"CACHE-CONTROL: max-age=1800\r\n"
			"LOCATION: http://%s/%s\r\n"
			"NT: %s\r\n"
			"NTS: ssdp:alive \r\n"
			"SERVER: Linux/1.0 UPnP/1.1 %s/%s\r\n"
			"USN: uuid:%s%s\r\n"
			"BOOTID.UPNP.ORG: %d\r\n"
			"CONFIGID.UPNP.ORG: 0\r\n" "DEVICEID.SES.COM: %d\r\n\r\n\0";
	char buf[500], mac[15] = "00000000000000";
	char nt[3][50];

	char uuid1[] = "11223344-9999-0000-b7ae";
	socklen_t salen;
	int i;
	s->wtime = getTick();
	if (uuidi == 0)
	{
		uuidi = 1;
		get_mac(mac);
		sprintf(uuid, "%s-%s", uuid1, mac);
		fill_sockaddr(&ssdp_sa, opts.disc_host, 1900);
	}
	strcpy(nt[0], "::upnp:rootdevice");
	sprintf(nt[1], "::uuid:%s", uuid);
	strcpy(nt[2], "::urn:ses-com:device:SatIPServer:1");

	if (s->type != TYPE_UDP)
		return 0;

	LOG("ssdp_discovery: bootid: %d deviceid: %d http: %s", opts.bootid,
			opts.device_id, opts.http_host);

	for (i = 0; i < 3; i++)
	{
		sprintf(buf, reply, opts.disc_host, opts.http_host, opts.xml_path,
				nt[i] + 2, app_name, version, uuid, i == 1 ? "" : nt[i],
				opts.bootid, opts.device_id);
		salen = sizeof(ssdp_sa);
		LOGL(3, "Discovery packet %d:\n%s", i + 1, buf);
		sendto(s->sock, buf, strlen(buf), MSG_NOSIGNAL,
				(const struct sockaddr *) &ssdp_sa, salen);
	}
	s->rtime = getTick();
	return 0;
}

int ssdp;
int ssdp_reply(sockets * s)
{
	char *reply = "HTTP/1.1 200 OK\r\n"
			"CACHE-CONTROL: max-age=1800\r\n"
			"DATE: %s\r\n"
			"EXT:\r\n"
			"LOCATION: http://%s/%s\r\n"
			"SERVER: Linux/1.0 UPnP/1.1 %s/%s\r\n"
			"ST: urn:ses-com:device:SatIPServer:1\r\n"
			"USN: uuid:%s::urn:ses-com:device:SatIPServer:1\r\n"
			"BOOTID.UPNP.ORG: %d\r\n"
			"CONFIGID.UPNP.ORG: 0\r\n" "DEVICEID.SES.COM: %d\r\n\r\n\0";
	char *device_id_conflict = "M-SEARCH * HTTP/1.1\r\n"
			"HOST: %s:1900\r\n"
			"MAN: \"ssdp:discover\"\r\n"
			"ST: urn:ses-com:device:SatIPServer:1\r\n"
			"USER-AGENT: Linux/1.0 UPnP/1.1 %s/%s\r\n"
			"DEVICEID.SES.COM: %d\r\n\r\n\0";
	socklen_t salen;
	char *man, *man_sd, *didsescom, *ruuid, *rdid;
	char buf[500];
	char ra[50];
	int did = 0;

	if (uuidi == 0)
		ssdp_discovery(s);

	s->rtime = s->wtime; // consider the timeout of the discovery operation

	salen = sizeof(s->sa);
	ruuid = strcasestr((const char *) s->buf, "uuid:");
	if (ruuid && strncmp(uuid, strip(ruuid + 5), strlen(uuid)) == 0)
	{
		LOGL(3, "Dropping packet from the same UUID as mine (from %s:%d)",
				get_socket_rhost(s->id, ra, sizeof(ra)),
				get_socket_rport(s->id));
		return 0;
	}

// not my uuid
	LOG("Received SSDP packet from %s:%d -> handle %d",
			get_socket_rhost(s->id, ra, sizeof(ra)), get_socket_rport(s->id),
			s->sock);
	LOGL(3, "%s", s->buf);

	if (strncasecmp((const char *) s->buf, "NOTIFY", 6) == 0)
	{
		rdid = strcasestr((const char*) s->buf, "DEVICEID.SES.COM:");
		if (rdid && opts.device_id == map_int(strip(rdid + 17), NULL))
		{
			snprintf(buf, sizeof(buf), device_id_conflict, getlocalip(),
					app_name, version, opts.device_id);
			LOG(
					"A new device joined the network with the same Device ID:  %s, asking to change DEVICEID.SES.COM",
					get_socket_rhost(s->id, ra, sizeof(ra)));
			sendto(ssdp, buf, strlen(buf), MSG_NOSIGNAL,
					(const struct sockaddr *) &s->sa, salen);
		}

		return 0;
	}

	man = strcasestr((const char*) s->buf, "MAN");
	man_sd = strcasestr((const char*) s->buf, "ssdp:discover");
	if ((didsescom = strcasestr((const char*) s->buf, "DEVICEID.SES.COM:")))
		did = map_int(didsescom + 17, NULL);

	if (man && man_sd && didsescom && (s->rtime < 15000)
			&& did == opts.device_id) // SSDP Device ID clash, only first 5 seconds after the announcement
	{
		opts.device_id++;
		s[si].timeout_ms = 1800 * 1000;
		s[si].rtime = -s[si].timeout_ms;
		LOG(
				"Device ID conflict, changing our device id to %d, destination SAT>IP server %s",
				opts.device_id, get_socket_rhost(s->id, ra, sizeof(ra)));
		readBootID();
	}
	else
		did = opts.device_id;

	if (strncmp((const char*) s->buf, "HTTP/1", 6) == 0)
		LOG_AND_RETURN(0, "ssdp_reply: the message is a reply, ignoring....");

	sprintf(buf, reply, get_current_timestamp(), opts.http_host, opts.xml_path,
			app_name, version, uuid, opts.bootid, did);

	LOG("ssdp_reply fd: %d -> %s:%d, bootid: %d deviceid: %d http: %s", ssdp,
			get_socket_rhost(s->id, ra, sizeof(ra)), get_socket_rport(s->id),
			opts.bootid, did, opts.http_host);
//use ssdp (unicast) even if received to multicast address
	LOGL(3, "%s", buf);
	sendto(ssdp, buf, strlen(buf), MSG_NOSIGNAL,
			(const struct sockaddr *) &s->sa, salen);
	return 0;
}

int new_rtsp(sockets * s)
{
	s->type = TYPE_RTSP;
	s->action = (socket_action) read_rtsp;
	s->close = (socket_action) close_http;
	return 0;
}

int new_http(sockets * s)
{
	s->type = TYPE_HTTP;
	s->action = (socket_action) read_http;
	s->close = (socket_action) close_http;
	return 0;
}

void write_pid_file()
{
	FILE *f;
	sprintf(pid_file, PID_NAME, app_name);
	if ((f = fopen(pid_file, "wt")))
	{
		fprintf(f, "%d", getpid());
		fclose(f);
	}
}

pthread_t main_tid;
extern int sock_signal;
int main(int argc, char *argv[])
{
	int sock_st, sock_bw;
	main_tid = get_tid();
	thread_name = "main";
	set_signal_handler(argv[0]);
	set_options(argc, argv);
	if (opts.daemon)
		becomeDaemon();
	if (opts.slog)
		openlog(app_name,
		LOG_NDELAY | LOG_NOWAIT | LOG_PID | (opts.slog > 1 ? LOG_PERROR : 0),
		LOG_DAEMON);

	print_version(1);

	readBootID();
	if ((ssdp = udp_bind(NULL, 1900)) < 1)
		FAIL("SSDP: Could not bind on udp port 1900");
	if ((ssdp1 = udp_bind(opts.disc_host, 1900)) < 1)
		FAIL("SSDP: Could not bind on %s udp port 1900", opts.disc_host);
	if ((rtsp = tcp_listen(NULL, opts.rtsp_port)) < 1)
		FAIL("RTSP: Could not listen on port %d", opts.rtsp_port);
	if ((http = tcp_listen(NULL, opts.http_port)) < 1)
		FAIL("Could not listen on http port %d", opts.http_port);

	si = sockets_add(ssdp, NULL, -1, TYPE_UDP, (socket_action) ssdp_reply, NULL,
			(socket_action) ssdp_discovery);
	si1 = sockets_add(ssdp1, NULL, -1, TYPE_UDP, (socket_action) ssdp_reply,
	NULL, (socket_action) ssdp_discovery);
	if (si < 0 || si1 < 0)
		FAIL("sockets_add failed for ssdp");

	sockets_timeout(si, 60 * 1000);
	set_sockets_rtime(si, -60 * 1000);
	if (0 > sockets_add(rtsp, NULL, -1, TYPE_SERVER, (socket_action) new_rtsp,
	NULL, (socket_action) close_http))
		FAIL("sockets_add failed for rtsp");
	if (0 > sockets_add(http, NULL, -1, TYPE_SERVER, (socket_action) new_http,
	NULL, (socket_action) close_http))
		FAIL("sockets_add failed for http");

	if (0 > (sock_signal = sockets_add(SOCK_TIMEOUT, NULL, -1, TYPE_UDP, NULL,
	NULL, (socket_action) signal_thread)))
		FAIL("sockets_add failed for signal thread");

	if (!opts.no_threads)
		set_socket_thread(sock_signal, start_new_thread("signal"));
	sockets_timeout(sock_signal, 1000);

	if (0 > (sock_bw = sockets_add(SOCK_TIMEOUT, NULL, -1, TYPE_UDP, NULL,
	NULL, (socket_action) calculate_bw)))
		FAIL("sockets_add failed for BW calculation");

	set_socket_thread(sock_bw, get_socket_thread(sock_signal));
	sockets_timeout(sock_bw, 1000);

#ifndef DISABLE_TABLES
	tables_init();
#endif
	LOGL(0, "Initializing with %d devices", init_all_hw());

	write_pid_file();
	select_and_execute(NULL);
	unlink(pid_file);
	free_all();
	if (opts.slog)
		closelog();
	return 0;
}

int readBootID()
{
	int did = 0;
	opts.bootid = 0;
	FILE *f = fopen("bootid", "rt");
	__attribute__((unused)) int rv;
	if (f)
	{
		rv = fscanf(f, "%d %d", &opts.bootid, &did);
		fclose(f);
		if (opts.device_id < 1)
			opts.device_id = did;
	}
	opts.bootid++;
	if (opts.device_id < 1)
		opts.device_id = 1;
	f = fopen("bootid", "wt");
	if (f)
	{
		fprintf(f, "%d %d", opts.bootid, opts.device_id);
		fclose(f);
	}
	return opts.bootid;
}

void http_response(sockets *s, int rc, char *ah, char *desc, int cseq, int lr)
{
	int binary = 0;
	char *desc1;
	char ra[50];
	char server[50];
	char *reply =
			"%s/1.0 %d %s\r\nDate: %s%s%s\r\n%s%s\r\nContent-Length: %d\r\n\r\n%s";
	char *reply0 = "%s/1.0 %d %s\r\nDate: %s%s%s\r\n%s%s\r\n\r\n";
	char *d;
	char *proto;
	char sess_id[100], scseq[100];

	if (s->type == TYPE_HTTP)
		proto = "HTTP";
	else
		proto = "RTSP";

	if (!ah || !ah[0])
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
	else if (rc == 454)
		d = "Session Not Found";
	else
	{
		d = "Service Unavailable";
		rc = 503;
	}
	char resp[10000];
	int lresp;
	desc1 = desc;
	resp[sizeof(resp) - 1] = 0;
	if (!lr)
		lr = strlen(desc);
	else
	{
		binary = 1;
		desc1 = "";
	}

	sess_id[0] = 0;
	scseq[0] = 0;
	server[0] = 0;

	sprintf(server, "\r\nServer: %s/%s", app_name, version);

	if (s->type != TYPE_HTTP && get_sid(s->sid) && ah && !strstr(ah, "Session")
			&& rc != 454)
		sprintf(sess_id, "\r\nSession: %010d", get_session_id(s->sid));
	if (s->type != TYPE_HTTP && cseq > 0)
		sprintf(scseq, "\r\nCseq: %d", cseq);

	if (lr > 0)
		lresp = snprintf(resp, sizeof(resp) - 1, reply, proto, rc, d,
				get_current_timestamp(), sess_id, scseq, ah, server, lr, desc1);
	else
		lresp = snprintf(resp, sizeof(resp) - 1, reply0, proto, rc, d,
				get_current_timestamp(), sess_id, scseq, ah, server);
	LOG("reply %s-> %d (%s:%d) CL:%d [sock_id %d]:",
			(lresp == sizeof(resp) - 1) ? "message truncated" : "", s->sock,
			get_socket_rhost(s->id, ra, sizeof(ra)), get_socket_rport(s->id),
			lr, s->id);
	LOGL(3, "%s", resp);
	send(s->sock, resp, strlen(resp), MSG_NOSIGNAL);
	if (binary)
		send(s->sock, desc, lr, MSG_NOSIGNAL);
}

_symbols minisatip_sym[] =
{
{ "http_host", VAR_PSTRING, &opts.http_host, 0, 0, 0 },
{ "uuid", VAR_STRING, uuid, 0, 0, 0 },
{ "http_port", VAR_INT, &opts.http_port, 1, 0, 0 },
{ "version", VAR_STRING, &version, 1, 0, 0 },
{ NULL, 0, NULL, 0, 0 } };

