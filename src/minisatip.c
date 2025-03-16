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
#include "adapter.h"
#include "api/symbols.h"
#include "api/variables.h"
#include "dvb.h"
#include "pmt.h"
#include "socketworks.h"
#include "stream.h"
#include "utils/alloc.h"
#include "utils/ticks.h"
#include "utils/uuid.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#ifndef DISABLE_SATIPCLIENT
#include "satipc.h"
#endif

#ifdef AXE
#include "axe.h"
#endif

#ifndef DISABLE_DVBCA
#include "ca.h"
#endif

#ifndef DISABLE_DDCI
#include "ddci.h"
#endif

struct struct_opts opts;

#define DEFAULT_LOG LOG_HTTP

#define DESC_XML "desc.xml"
#define PID_NAME "/var/run/%s.pid"
char version[] = VERSION;
char app_name[] = "minisatip";
char pid_file[50];
char public_str[] = "Public: OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN";
int rtsp, http, si, si1, ssdp1;

#define RRTP_OPT 'r'
#define DISABLEDVB_OPT 'N'
#define DISABLESSDP_OPT 'G'
#define BIND_OPT 'V'
#define BIND_HTTP_OPT 'U'
#define BIND_DEV_OPT 'J'
#define HTTPSERVER_OPT 'w'
#define HTTPPORT_OPT 'x'
#define LOG_OPT 'l'
#define DEBUG_OPT 'v'
#define LOGFILE_OPT 'F'
#define HELP_OPT 'h'
#define PLAYLIST_OPT 'p'
#define ADAPTERS_OPT 'a'
#define FOREGROUND_OPT 'f'
#define DVRBUFFER_OPT 'b'
#define APPBUFFER_OPT 'B'
#define THRESHOLD_OPT 'H'
#define ENABLE_ADAPTERS_OPT 'e'
#define UNICABLE_OPT 'u'
#define JESS_OPT 'j'
#define DISEQC_OPT 'd'
#define DISEQC_TIMING_OPT 'q'
#define SLAVE_OPT 'S'
#define DELSYS_OPT 'Y'
#define DVBAPI_OPT 'o'
#define LEGACY_DVBAPI_OPT 'm'
#define SYSLOG_OPT 'g'
#define RTSPPORT_OPT 'y'
#define SATIPCLIENT_OPT 's'
#define NETCVCLIENT_OPT 'n'
#define NAME_APP_OPT 'I'
#define PRIORITY_OPT 'i'
#define SATIP_TCP_OPT 'O'
#define DOCUMENTROOT_OPT 'R'
#define XML_OPT 'X'
#define THREADS_OPT 'T'
#define LNB_OPT 'L'
#define DROP_ENCRYPTED_OPT 'E'
#define UDPPORT_OPT 'P'
#define ADAPTERTIMEOUT_OPT 'Z'
#define QUATTRO_OPT 'Q'
#define QUATTRO_HIBAND_OPT '8'
#define AXE_POWER 'W'
#define ABSOLUTE_SRC 'A'
#define SATIPXML_OPT '6'
#define DISEQC_MULTI '0'
#define SIGNALMULTIPLIER_OPT 'M'
#define DEVICEID_OPT 'D'
#define DEMUXDEV_OPT '1'
#define FORCE_CI_OPT 'C'
#define CA_PIN_OPT '3'
#define IPV4_OPT '4'
#define NO_PIDS_ALL_OPT 'k'
#define CA_CHANNELS_OPT 'c'
#define CACHE_DIR_OPT 'z'
#define DISABLE_CAT_OPT '5'
#define DISABLE_PMT_SCAN '9'
// Options that don't have a short option available
#define LONG_OPT_ONLY_START 1024
#define VERSION_OPT (LONG_OPT_ONLY_START + 1)
#define SENDALLECM_OPT (LONG_OPT_ONLY_START + 2)
#define SATIPC_RECV_BUFFER_OPT (LONG_OPT_ONLY_START + 3)
#define CLIENT_SEND_BUFFER_OPT (LONG_OPT_ONLY_START + 4)

static const struct option long_options[] = {
    {"adapters", required_argument, NULL, ADAPTERS_OPT},
    {"adapter-timeout", required_argument, NULL, ADAPTERTIMEOUT_OPT},
    {"app-buffer", required_argument, NULL, APPBUFFER_OPT},
    {"buffer", required_argument, NULL, DVRBUFFER_OPT},
    {"bind", required_argument, NULL, BIND_OPT},
    {"bind-http", required_argument, NULL, BIND_HTTP_OPT},
    {"bind-dev", required_argument, NULL, BIND_DEV_OPT},
    {"cache-dir", required_argument, NULL, CACHE_DIR_OPT},
    {"send-all-ecm", no_argument, NULL, SENDALLECM_OPT},
    {"client-send-buffer", required_argument, NULL, CLIENT_SEND_BUFFER_OPT},
    {"delsys", required_argument, NULL, DELSYS_OPT},
    {"debug", required_argument, NULL, DEBUG_OPT},
    {"device-id", required_argument, NULL, DEVICEID_OPT},
    {"demux-dev", required_argument, NULL, DEMUXDEV_OPT},
    {"disable-dvb", no_argument, NULL, DISABLEDVB_OPT},
    {"disable-ssdp", no_argument, NULL, DISABLESSDP_OPT},
    {"diseqc", required_argument, NULL, DISEQC_OPT},
    {"diseqc-timing", required_argument, NULL, DISEQC_TIMING_OPT},
    {"diseqc-multi", required_argument, NULL, DISEQC_MULTI},
    {"document-root", required_argument, NULL, DOCUMENTROOT_OPT},
    {"drop-encrypted", no_argument, NULL, DROP_ENCRYPTED_OPT},
    {"enable-adapters", required_argument, NULL, ENABLE_ADAPTERS_OPT},
    {"foreground", no_argument, NULL, FOREGROUND_OPT},
    {"name-app", required_argument, NULL, NAME_APP_OPT},
    {"http-port", required_argument, NULL, HTTPPORT_OPT},
    {"http-host", required_argument, NULL, HTTPSERVER_OPT},
    {"ipv6", no_argument, NULL, IPV4_OPT},
    {"jess", required_argument, NULL, JESS_OPT},
    {"lnb", required_argument, NULL, LNB_OPT},
    {"log", required_argument, NULL, LOG_OPT},
    {"logfile", required_argument, NULL, LOGFILE_OPT},
    {"multiplier", required_argument, NULL, SIGNALMULTIPLIER_OPT},
    {"no-all-pids", no_argument, NULL, NO_PIDS_ALL_OPT},
    {"priority", required_argument, NULL, PRIORITY_OPT},
    {"playlist", required_argument, NULL, PLAYLIST_OPT},
    {"remote-rtp", required_argument, NULL, RRTP_OPT},
    {"rtsp-port", required_argument, NULL, RTSPPORT_OPT},
    {"syslog", no_argument, NULL, SYSLOG_OPT},
    {"slave", required_argument, NULL, SLAVE_OPT},
    {"threads", no_argument, NULL, THREADS_OPT},
    {"threshold", required_argument, NULL, THRESHOLD_OPT},
    {"udp", required_argument, NULL, UDPPORT_OPT},
    {"unicable", required_argument, NULL, UNICABLE_OPT},
    {"xml", required_argument, NULL, XML_OPT},
    {"help", no_argument, NULL, HELP_OPT},
    {"virtual-diseqc", required_argument, NULL, ABSOLUTE_SRC},
    {"version", no_argument, NULL, VERSION_OPT},
    {"disable-pmt-scan", no_argument, NULL, DISABLE_PMT_SCAN},
#ifndef DISABLE_DDCI
    {"disable-cat", required_argument, NULL, DISABLE_CAT_OPT},
#endif
#ifndef DISABLE_DVBAPI
    {"dvbapi", required_argument, NULL, DVBAPI_OPT},
    {"legacy-dvbapi", required_argument, NULL, LEGACY_DVBAPI_OPT},
#endif
#ifndef DISABLE_SATIPCLIENT
    {"satip-servers", required_argument, NULL, SATIPCLIENT_OPT},
    {"satip-tcp", no_argument, NULL, SATIP_TCP_OPT},
    {"satip-xml", required_argument, NULL, SATIPXML_OPT},
    {"satip-receive-buffer", required_argument, NULL, SATIPC_RECV_BUFFER_OPT},
#endif
#ifndef DISABLE_NETCVCLIENT
    {"netceiver", required_argument, NULL, NETCVCLIENT_OPT},
#endif
#ifdef AXE
    {"quattro", no_argument, NULL, QUATTRO_OPT},
    {"quattro-hiband", required_argument, NULL, QUATTRO_HIBAND_OPT},
    {"power", required_argument, NULL, AXE_POWER},
#endif
#ifndef DISABLE_DVBCA
    {"ca-pin", required_argument, NULL, CA_PIN_OPT},
    {"ci", required_argument, NULL, FORCE_CI_OPT},
    {"ca-channels", required_argument, NULL, CA_CHANNELS_OPT},
#endif

    {0, 0, 0, 0}};

char *built_info[] = {
#ifdef DISABLE_DVBCSA
    "Built without dvbcsa",
#else
    "Built with dvbcsa",
#endif
#ifdef DISABLE_DVBCA
    "Built without CI",
#else
    "Built with CI",
#endif
#ifdef DISABLE_DVBAPI
    "Built without dvbapi",
#else
    "Built with dvbapi",
#endif
#ifdef DISABLE_TABLES
    "Built without tables processing",
#else
    "Built with tables processing",
#endif
#ifdef DISABLE_PMT
    "Built without pmt processing",
#else
    "Built with pmt processing",
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
#ifdef DISABLE_DDCI
    "Built without ddci",
#else
    "Built with ddci",
#endif
#ifdef DISABLE_T2MI
    "Built without t2mi",
#else
    "Built with t2mi",
#endif
#ifdef AXE
    "Built for satip-axe",
#endif
    NULL};

void print_version(int use_log, int print_built_info) {
    char buf[200];
    int i;
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%s version %s, compiled on %s %s, with s2api version: %04X",
            app_name, version, __DATE__, __TIME__, LOGDVBAPIVERSION);
    if (!use_log)
        puts(buf);
    else
        LOG0(buf);

    if (print_built_info) {
        for (i = 0; built_info[i]; i++)
            LOG("%s", built_info[i]);
    }
}

void usage() {
    char modules[1000];
    int len = 0, i;
    print_version(0, 1);

    for (i = 0; loglevels[i]; i++)
        len += sprintf(modules + len, "%s,", loglevels[i]);
    modules[len - 1] = 0;

    printf(
        "\n\t./%s [-[fgtzE]] [-a x:y:z] [-b X:Y] [-B X] [-H X:Y] [-d A:C-U ] [-D device_id] [-e X-Y,Z] [-i prio] \n\
\t[-[uj] A1:S1-F1[-PIN]] [-P port] [-l module1[,module2]] [-v module1[,module2]] \n\t"
#ifndef DISABLE_DVBAPI
        "[-o [~]oscam_host:dvbapi_port[,offset] [-m A] "
#endif
        "[-p public_host] [-r remote_rtp_host] [-R document_root] "
#ifndef DISABLE_SATIPCLIENT
        "[-s [*][DELSYS:][FE_ID@][source_ip/]host[:port] "
#endif
        "\n\t[-u A1:S1-F1[-PIN]] [-L A1:low-high-switch] [-w "
        "http_server[:port]] \n "
#ifdef AXE
        "[-7 M1:S1[,M2:S2]] [-A SRC1:INP1:DISEQC1[,SRC2:INP2:DISEQC2]]\n\n"
#endif
        "\t[-x http_port] [-X xml_path] [-y rtsp_port] [-I name_service]\n\n\
Help\n\
-------\n\
\n\
* -4 : Force TCP sockets to use IPv6\n\
\n\
* -a --adapters x:y:z simulate x DVB-S2, y DVB-T2 and z DVB-C adapters on this box (0 means auto-detect)\n\
	* eg: -a 1:2:3  \n\
	- it will report 1 dvb-s2 device, 2 dvb-t2 devices and 3 dvb-c devices \n\
\n\
* -G --disable-ssdp disable SSDP announcement\n \
\n\
* -b --buffer X:Y : set the app adapter buffer to X Bytes (default: %d) and set the kernel DVB buffer to Y Bytes (default: %d) - both multiple of 188\n\
	* eg: -b 18800:18988\n\
\n\
* -B X : set the app socket write buffer to X MB. \n\
	* eg: -B 10 - to set the socket buffer to 10MB (default value)\n\
\n\
* --client-send-buffer X : set the socket write buffer for client connections to X KB. \n\
	- The default value is 0, corresponding to use the kernel default value\n\
	* eg: --satip-receive-buffer  100 - to set the socket buffer to 100KB\n\
	* eg: --satip-receive-buffer 1024 - to set the socket buffer to 1MB\n\
\n\
* -z --cache-dir dir : set the app cache directory to dir. The directory will be created if it doesn't exist. \n\
	* defaults to /var/cache/minisatip \n\
\n\
* -d --diseqc ADAPTER1:COMMITTED1-UNCOMMITTED1[,ADAPTER2:COMMITTED2-UNCOMMITTED2[,...]\n\
\t* The first argument is the adapter number, second is the number of committed packets to send to a Diseqc 1.0 switch, third the number of uncommitted commands to sent to a Diseqc 1.1 switch\n\
\tThe higher number between the committed and uncommitted will be sent first.\n\
	* eg: -d 0:1-0  (which is the default for each adapter).\n\
	- note: * as adapter means apply to all adapters\n\
	- note: * before committed number enables fast-switch (only voltage/tone)\n\
	- note: @ before committed number sets 'Any Device' diseqc address (0x00)\n\
	- note: . before committed number sets 'LNB' diseqc address (0x11)\n\
\n\
* -q --diseqc-timing ADAPTER1:BEFORE_CMD1-AFTER_CMD1-AFTER_REPEATED_CMD1-AFTER_SWITCH1-AFTER_BURST1-AFTER_TONE1[,...]\n\
\t* All timing values are in ms, default adapter values are: 15-54-15-15-15-0\n\
	- note: * as adapter means apply to all adapters\n\
\n\
* -D --device-id DVC_ID: specify the device id (in case there are multiple SAT>IP servers in the network)\n \
	* eg: -D 4 \n\
\n\
* -0 --diseqc-multi ADAPTER1:DISEQC_POSITION[,...]\n\
\t* Send diseqc to selected position before other position is set.\n\
	- note: * as adapter means apply to all adapters\n\
\n\
* -E Allows encrypted stream to be sent to the client even if the decrypting is unsuccessful\n \
       Duplicating it (-E -E) all undecrypted packets are send as stuffing TS packets. Usefull for regular player clients.\n\
	- note: when pids=all is emulated this pass NULLs too\n\
\n\
* -Y --delsys ADAPTER1:DELIVERY_SYSTEM1[,ADAPTER2:DELIVERY_SYSTEM2[,..]] - specify the delivery system of the adapters (0 is the first adapter)	\n\
	* eg: --delsys 0:dvbt,1:dvbs\n\
	- specifies adapter 0 as a DVBT device, adapter 1 as DVB-S, which overrides the system detection of the adapter\n\
\n\
* -e --enable-adapters list_of_enabled adapters: enable only specified adapters\n\
	* eg: -e 0-2,5,7 (no spaces between parameters)\n\
	- keep in mind that the first adapters are the local ones starting with 0 after that are the satip adapters \n\
	if you have 3 local dvb cards 0-2 will be the local adapters, 3,4, ... will be the satip servers specified with argument -s\n\
\n\
* -f foreground: otherwise run in background\n\
\n\
* -F --logfile log_file: output the debug/log information to  log_file when running in background (option -f not used), default /tmp/minisatip.log\n\
\n\
* -g --syslog: use syslog instead stdout for logging, multiple -g - print to stderr as well\n\
\n\
* -H --threshold X:Y : set the write time threshold to X (UDP) / Y (TCP)  milliseconds. \n\
	* eg: -H 5:50 - set thresholds to 5ms (UDP) and 50ms (TCP)\n\
\n\
* -I --name-app specificies an alternative Service Name\n\
\n\
* -i --priority prio: set the DVR thread priority to prio \n\
\n\
* -k Emulate pids=all when the hardware does not support it, on enigma boxes is enabled by default \n\
\n\
* -l specifies the modules comma separated that will have increased verbosity, \n\
	logging to stdout in foreground mode or in /tmp/minisatip.log when a daemon\n\
	Possible modules: %s\n\
	* eg: -l http,pmt\n\
\n\
* -v specifies the modules comma separated that will have increased debug level (more verbose than -l), \n\
	* eg: -v http,pmt\n\
\n\
* -L --lnb specifies the adapter and LNB parameters (low, high and switch frequency)\n\
	* eg: -L *:9750-10600-11700 - sets all the adapters to use Universal LNB parameters (default)\n\
	* eg: -L *:10750-0-0 - sets all adapters to use 10750 MHz Ku-band LNB parameters (e.g. Sky NZ)\n\
	* eg: -L *:5150-0-0 - sets all adapters to use 5150 MHz C-band LNB parameters\n\
	* eg: -L 0:10750-0-0,1:9750-10600-11700 - adapter 0 has a SKY NZ LNB, adapter 1 has an Universal LNB\n\
	\n\
	For backward-compatibility reasons, linear LNB parameters may also be specified as *:5150-5150-5150 instead of *:5150-0-0\n\
\n\
* -M --multiplier: multiplies the strength and snr of the DVB adapter with the specified values\n\
	* If the snr or the strength multipliers are set to 0, minisatip will override the value received from the adapter and will report always full signal 100%% \n\
	* eg: -M 4-6:1.2-1.3 - multiplies the strength with 1.2 and the snr with 1.3 for adapter 4, 5 and 6\n\
	* eg: -M *:1.5-1.6 - multiplies the strength with 1.5 and the snr with 1.6 for all adapters\n\
	* [%%] This symbol forces to read values as percentage\n\
	* [#] This symbol forces to read values as decibels\n\
	* eg: -M *:%%1.0-#1.0 - not multiply the strength but force it as percentage and for the snr interpret it as decibels\n\
	* Zero values will disable frontend polling, and this may solve CC errors with some hardware/drivers (eg: -M *:0-0)\n\
\n\
* -N --disable-dvb disable DVB adapter detection\n \
\n\
* -Z --adapter-timeout ADAPTER1,ADAPTER2-ADAPTER4[,..]:TIMEOUT - specify the timeout for the adapters (0 enabled infinite timeout)	\n\
	eg: --adapter-timeout 1-2:30\n\
	- sets the timeouts for adapter 1 and 2 to 30 seconds \n\
	--adapter-timeout *:0\n\
	- turns off power management for all adapters (recommended instead of --adapter-timeout 0-32:0) \n\
	- required for some Unicable LNBs \n\
\n\
"
#ifndef DISABLE_DDCI
        "\
* -5 --disable-cat ADAPTER1,ADAPTER2-ADAPTER4\n\
	* eg: -5 1-3,4 \n\
	- disable passing the CAT to the DDCI device 1,2,3 and 4 \n\
\n\
"
#endif

#ifndef DISABLE_NETCVCLIENT
        "\
* -n --netceiver if:count: use network interface <if> (default vlan4) and look for <count> netceivers\n\
	* eg: -n vlan4:2 \n\
\n\
"
#endif
#ifndef DISABLE_DVBAPI
        "\
* -o --dvbapi [~]host:port,offset - specify the hostname and port for the dvbapi server (oscam). Port 9000 is set by default (if not specified) \n\
	* [~] This symbol at the beginning indicates that in all `pids=all` requests the filtering of unencrypted packets must be disabled (useful when not using -E).\n\
	* eg: -o 192.168.9.9:9000 or -o 192.168.9.9:9999,2  with offset if multiple dvbapi clients use the same server\n\
	192.168.9.9 is the host where oscam is running and 9000 is the port configured in dvbapi section in oscam.conf.\n\
	* eg: -o /tmp/camd.socket \n\
	/tmp/camd.socket is the local socket that can be used \n\
\n\
* --send-all-ecm Pass all received ECM to the DVBAPI server. Warning: This option may overload your server. Use with caution. Not necessary for regular use.\n\
\n\
* -m --legacy-dvbapi ADAPTER1[,ADAPTER2[,...]\n\
\t* Enable legacy (v3)  instead of recent (v5) DVBAPI for the specified (DVB hardware) adapter number.\n\
	- note: * as adapter means apply to all adapters (note that you might have to quote *)\n\
\n\
"
#endif
        "\
* -p --playlist url: specify playlist url using X_SATIPM3U header \n\
	* eg: -p http://192.168.2.3:8080/playlist\n\
	- this will add X_SATIPM3U tag into the satip description xml\n\
\n\
* -P  port: use port number to listen for UDP socket in the RTP communication. port + 1000 will be used to listen by the sat>ip client (option -s)\n \
	* eg: -P 5500 (default): will use for the sat>ip server 5500 + 2*A and 5500 + 2*A + 1, where A is the adapter number. \n\
				6500 + 2*A and 6500 + 2*A + 1 - will be used by the sat>ip client\n \
\n\
* -r --remote-rtp  remote_rtp_host: send the rtp stream to remote_rtp_host instead of the ip the connection comes from\n \
	* eg: -r 192.168.7.9\n \
\n\
* -R --document-root directory: document root for the minisatip web page and images\n\
\n\
"
#ifndef DISABLE_SATIPCLIENT
        "\
* -s --satip-servers [~][*][DELSYS:][FE_ID@][source_ip/]host[:port] - specify the remote satip host and port with delivery system DELSYS, it is possible to use multiple -s \n\
	* [~] When using this symbol at start the `pids=all` call is replaced with `pids=0-20`\n\
	* [*] Use TCP if -O is not specified and UDP if -O is specified\n\
	* DELSYS - can be one of: dvbs, dvbs2, dvbt, dvbt2, dvbc, dvbc2, isdbt, atsc, dvbcb ( - DVBC_ANNEX_B ) [default: dvbs2]\n\
	* host - the server of the satip server\n\
	* port - rtsp port for the satip server [default: 554]\n\
	* FE_ID - will be determined automatically\n\
	* eg: -s 192.168.1.2 -s dvbt:192.168.1.3:554 -s dvbc:192.168.1.4\n\
	- specifies 1 dvbs2 (and dvbs)satip server with address 192.168.1.2:554\n\
	- specifies 1 dvbt satip server  with address 192.168.1.3:554\n\
	- specifies 1 dvbc satip server  with address 192.168.1.4:554\n\
	* eg: -s dvbt:2@192.168.1.100/192.168.1.2:555\n\
	- specifies 1 dvbt adapter to satip server with address 192.168.1.2, port 555. The client will use fe=2 (indicating adapter 2 on the server) and will connect from IP address 192.168.1.100\n\
	address 192.168.1.100 needs to be assigned to an interface on the server running minisatip.\n\
	This feature is useful for AVM FRITZ!WLAN Repeater\n\
	\n\
* -6 --satip-xml <URL> Use the xml retrieved from a satip server to configure satip adapters \n\
	eg: --satip-xml http://localhost:8080/desc.xml \n\
\n\
* -O --satip-tcp Use RTSP over TCP instead of UDP for data transport \n\
\n\
* --satip-receive-buffer X : set the socket read buffer for connecting to SAT>IP servers to X KB. \n\
	- The default value is 0, corresponding to use the kernel default value\n\
	* eg: --satip-receive-buffer  350 - to set the socket buffer to 350KB\n\
	* eg: --satip-receive-buffer 1024 - to set the socket buffer to 1MB\n\
\n\
"
#endif
        "\
* -S --slave ADAPTER1,ADAPTER2-ADAPTER4[,..]:MASTER - specify slave adapters	\n\
	* Allows specifying bonded adapters (multiple adapters connected with a splitter to the same LNB)\n\
	* This feature is used by FBC receivers and AXE to specify the source input of the adapter\n\
	Only one adapter needs to be master all others needs to have this parameter specified\n\
	eg: -S 1-2:0\n\
	- specifies adapter 1 to 2 as slave, in this case adapter 0 is the master that controls the LNB\n\
	- the slave adapter will not control the LNB polarity or band, but it will just change the internal frequency to tune to a different transponder\n\
	- if there is no adapter using this link, the slave will use master adapters frontend to change the LNB polarity and band\n\
	eg: -S 2-7:0 (default for DVB-S2 FBC), adapter 0 and 1 are masters, 2-7 slave and linked to input 0 (A)\n\
	- all 8 adapters use physical input A to tune\n\
	eg: -S 2-4:0,5-7:1\n\
	- adapters 2,3,4 use physical input A to tune, while 1,5,6,7 uses input B to tune, adapter 0 and 1 are masters\n\
\n\
* -t --cleanpsi clean the PSI from all CA information, the client will see the channel as clear if decrypted successfully\n\
\n\
* -T --threads: enables/disable multiple threads (reduces memory consumption) (default: %s)\n\
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
* -1 --demux-dev [1|2|3]: the protocol used to get the data from demux\n\
	* 0 - use dvrX device \n\
	* 1 - use demuxX device \n\
	* 2 - use dvrX device and additionally capture PSI data from demuxX device \n\
	* 3 - use demuxX device and additionally capture PSI data from demuxX device \n\
* -V --bind address: address for listening (RTSP + SSDP) \n\
* -U --bind-http address: address for listening (HTTP)\n\
* -J --bind-dev device: device name for binding (all services)\n\
        * beware that only works with 1 device. loopback may not work!\n\
\n\
* -A --virtual-diseqc mapping_string: absolute source mapping for virtual diseqc mode\n\
\t* The format is: SRC1[-END1]:AD1:DISEQC1[,SRC2:INP2:DISEQC2]\n\
       * SRC: satellite position requested by clients (src argument for SAT>IP request minus 1, range: 0-63) (MAX_ADAPTERS 64)\n\
       * END: end of the range for the position requested by clients (end argument for SAT>IP request minus 1, range: 0-63) (MAX_ADAPTERS 64)\n\
       * ADAP: internal adapter (see -a and -e) (range: 0-63) (MAX_SOURCES 64)\n\
       * DISEQC: target diseqc position of the adapter (range: 0-15) (or more if your adapter supports it)\n\
	   * eg: 13E,19.2E on inputs 0&1 and 23.5E,28.2E on inputs 2&3:\n\
		-A 0:0:0,0:1:0,1:0:0,1:1:1,2:2:0,2:3:0,3:2:1,3:2:2\n\
	   * eg: 13E,19.2E on adapter 0 and 23.5E,28.2E on adapter 2&3 (source 0,1):\n\
		-A 0-1:0:0,2-3:1:0\n\
\t* Note: Option -A is not compatible with clients requesting fe= (tvheadend)\n\
\n\
* -9 --disable-pmt-scan: Disables scanning PMTs and only reads the PMTs that are requested by the client\n\
\t* Provides more reliable decrypting for channels included in multiple providers\n\
\n"
#ifdef AXE
        "\
* -W --power num: power to all inputs (0 = only active inputs, 1 = all inputs)\n\
\n\
* -Q --quattro  quattro LNB config (H/H,H/V,L/H,L/V)\n\
\n\
* -8 --quattro-hiband hiband\n\
	* if hiband is 0, do not allow hiband\n\
	* if hiband is 1, allow hiband\n\
\n\
"
#endif
#ifndef DISABLE_DVBCA
        "\
* -3 --ca-pin mapping_string: set the pin for CIs\n\
\t* The format is: ADAPTER1:PIN,ADAPTER2-ADAPTER4:PIN\n\
	* eg: 0:1234,2-3:4567 \n\
\n\
* -C --ci mapping_string: disable CI+ mode for specified adapters\n\
\t* The format is: ADAPTER1:PIN,ADAPTER2-ADAPTER4\n\
			* eg : 0,2-3\n\
\n\
* -c --ca-channels adapter_list:maximum_number_of_channels_supported: Specify the number of channels supported by the CAM and override the CAIDs\n\
\t* The format is: ADAPTER1:[*]MAX_CHANNELS[-CAID1[-CAID2]...,ADAPTER2:MAX_CHANNELS[-CAID3[-CAID4]...] \n\
\t\t * before the MAX_CHANNELS enable 2 PMTs inside of the same CAPMT to double the number of decrypted channels\n\
			* eg : 0:*1-100\n\
The DDCI adapters 0 will support maximum of 1 CAPMT (2 channels because of *) and will use CAID1. If CAID is not specified it will use CAMs CAIDs\n\
Official CAMs support 1 or 2 channels, with this option this is extended to 2 or 4\n\
By default every CAM supports 1 channels\n\
\n\
"
#endif
        ,
        app_name, ADAPTER_BUFFER, DVR_BUFFER, modules,
        opts.no_threads ? "DISABLED" : "ENABLED");
    exit(1);
}

char *get_command_line_string(int argc, char *argv[]) {
    int i;
    size_t len = argc;

    for (i = 0; i < argc; i++) {
        len += strlen(argv[i]) + 1;
    }

    char *dest = _malloc(len);
    memset(dest, 0, sizeof(len));
    for (i = 0; i < argc; i++) {
        strcat(dest, argv[i]);
        if (i != argc - 1) {
            strcat(dest, " ");
        }
    }

    return dest;
}

void get_short_opts(char *short_opts, const struct option *long_options) {
    int i = 0;
    while (long_options[i].name) {
        *short_opts++ = long_options[i].val;
        if (long_options[i].has_arg == required_argument)
            *short_opts++ = ':';
        else if (long_options[i].has_arg == optional_argument) {
            *short_opts++ = ':';
            *short_opts++ = ':';
        }
        i++;
    }
    *short_opts++ = 0;
}

void set_options(int argc, char *argv[]) {
    int opt;
    int is_log = 0;
    char *lip;
    memset(&opts, 0, sizeof(opts));
    if (access("/etc/enigma2/settings", F_OK) == 0) {
        opts.enigma = 1;
    }

    opts.command_line = get_command_line_string(argc, argv);
    opts.log = 1;
    opts.debug = 0;
    opts.file_line = 0;
    opts.log_file = "/tmp/minisatip.log";
    opts.disc_host = "239.255.255.250";
    opts.start_rtp = 5500;
    opts.http_port = 8080;
    opts.timeout_sec = 30000;
    opts.adapter_timeout = 30000;
    opts.daemon = 1;
    opts.device_id = 0;
    opts.dvr_buffer = DVR_BUFFER;
    opts.adapter_buffer = ADAPTER_BUFFER;
    opts.udp_threshold = 25;
    opts.tcp_threshold = 50;
    opts.dvbapi_port = 0;
    memset(opts.dvbapi_host, 0, sizeof(opts.dvbapi_host));
    opts.legacy_dvbapi = 0;
    opts.drop_encrypted = 1;
    opts.pids_all_no_dec = 0;
    opts.rtsp_port = 554;
    opts.use_ipv4_only = 1;
    opts.document_root = "html";
    opts.cache_dir = "/var/cache/minisatip";
    opts.xml_path = DESC_XML;
    opts.th_priority = -1;
    opts.diseqc_addr = 0x10;
    opts.diseqc_before_cmd = 15;
    opts.diseqc_after_cmd = 54;
    opts.diseqc_after_repeated_cmd = 15;
    opts.diseqc_after_switch = 15;
    opts.diseqc_after_burst = 15;
    opts.diseqc_after_tone = 0;
    opts.diseqc_committed_no = 1;
    opts.diseqc_multi = -1;
    opts.lnb_low = (9750 * 1000UL);
    opts.lnb_high = (10600 * 1000UL);
    opts.lnb_circular = (10750 * 1000UL);
    opts.lnb_switch = (11700 * 1000UL);
    opts.max_sbuf = 10;
    opts.pmt_scan = 1;
    opts.strength_multiplier = 1;
    opts.snr_multiplier = 1;

    opts.use_demux_device = USE_DVR;
    if (opts.enigma) {
        opts.use_demux_device = USE_PES_FILTERS_AND_DEMUX;
        opts.emulate_pids_all = 1;
    }
    opts.max_pids = 0;
    opts.dvbapi_offset =
        0; // offset for multiple dvbapi clients to the same server
#if defined(AXE)
    opts.max_pids = 32;
#elif defined(__sh__)
    opts.max_pids = 20; // allow oscam to use couple of pids as well
#endif

#ifdef NO_BACKTRACE
    opts.no_threads = 1;
#endif
#ifdef AXE
    opts.no_threads = 1;
    opts.document_root = "/usr/share/minisatip/html";
#endif

    opts.name_app = app_name;
    char short_opts[200];
    get_short_opts(short_opts, long_options);

    while ((opt = getopt_long_only(argc, argv, short_opts, long_options,
                                   NULL)) != -1) {
        switch (opt) {
        case FOREGROUND_OPT: {
            opts.daemon = 0;
            break;
        }
        case RRTP_OPT: {
            opts.rrtp = optarg;
            break;
        }

        case DISABLEDVB_OPT: {
            opts.disable_dvb = 1;
            break;
        }

        case DISABLESSDP_OPT: {
            opts.disable_ssdp = 1;
            break;
        }

        case DEVICEID_OPT: {
            opts.device_id = atoi(optarg);
            break;
        }

        case NAME_APP_OPT: {
            opts.name_app = optarg;
            break;
        }

        case HTTPSERVER_OPT: {
            //                              int i=0;
            opts.http_host = optarg;
            break;
        }

        case BIND_OPT: {
            opts.bind = optarg;
            break;
        }

        case BIND_HTTP_OPT: {
            opts.bind_http = optarg;
            break;
        }

        case BIND_DEV_OPT: {
            opts.bind_dev = optarg;
            break;
        }

        case DEBUG_OPT:
        case LOG_OPT: {
            is_log = 1;
            if (strstr(optarg, "all")) {
                opts.log = 0xFFFF;
                break;
            }
            char buf[200];
            char *arg[50];
            int i;
            memset(buf, 0, sizeof(buf));
            safe_strncpy(buf, optarg);
            int la = split(arg, buf, ARRAY_SIZE(arg), ',');
            for (i = 0; i < la; i++) {
                int level = check_strs(arg[i], loglevels, -1);
                if (level == -1) {
                    if (!strcmp(arg[i], "-l")) {
                        LOG("The LOG option has changed, please run "
                            "./minisatip -h");
                        level = 1;
                    } else {
                        LOG("module %s not recognised", arg[i]);
                        continue;
                    }
                }

                int log = (1 << level);
                if (opt == DEBUG_OPT)
                    opts.debug |= log;
                opts.log |= log;
            }
            break;
        }

        case SYSLOG_OPT: {
            opts.slog++;
            break;
        }

        case LOGFILE_OPT:
            opts.log_file = optarg;
            break;

        case HELP_OPT: {
            usage();
            exit(0);
        }

        case VERSION_OPT: {
            print_version(0, 0);
            exit(0);
        }

        case HTTPPORT_OPT: {
            opts.http_port = atoi(optarg);
            break;
        }

        case UDPPORT_OPT: {
            opts.start_rtp = atoi(optarg);
            break;
        }

        case IPV4_OPT: {
            opts.use_ipv4_only = 1 - opts.use_ipv4_only;
            if (!opts.use_ipv4_only)
                LOG0("IPv6 mode is enabled");
            break;
        }

        case DVRBUFFER_OPT: {
            sscanf(optarg, "%d:%d", &opts.adapter_buffer, &opts.dvr_buffer);
            opts.adapter_buffer = (opts.adapter_buffer / 188) * 188;
            if (opts.adapter_buffer < 1316)
                opts.adapter_buffer = 1316; // 188 * 7 = 1316
#ifdef AXE
            opts.dvr_buffer += 7 * 188 - 1;
            opts.dvr_buffer -= opts.dvr_buffer % (7 * 188);
#endif
            if (opts.dvr_buffer == 0)
                opts.dvr_buffer = DVR_BUFFER;

            break;
        }

        case APPBUFFER_OPT: {
            int val = atoi(optarg);
            opts.max_sbuf = val;
            break;
        }

        case CLIENT_SEND_BUFFER_OPT: {
            int val = atoi(optarg);
            opts.output_buffer = val * 1024;
            break;
        }

        case THRESHOLD_OPT: {
            sscanf(optarg, "%d:%d", &opts.udp_threshold, &opts.tcp_threshold);
            if (opts.udp_threshold < 0)
                opts.udp_threshold = 0;
            else if (opts.udp_threshold > 200)
                opts.udp_threshold = 200;
            if (opts.tcp_threshold < 0)
                opts.tcp_threshold = 0;
            else if (opts.tcp_threshold > 200)
                opts.tcp_threshold = 200;
            break;
        }

        case ADAPTERS_OPT: {
            sscanf(optarg, "%d:%d:%d", &opts.force_sadapter,
                   &opts.force_tadapter, &opts.force_cadapter);
            break;
        }

        case SENDALLECM_OPT: {
            opts.send_all_ecm = 1;
            break;
        }

        case NO_PIDS_ALL_OPT: {
            opts.emulate_pids_all = 1;
            break;
        }

        case PLAYLIST_OPT: {
            if (strlen(optarg) < 1000) {
                opts.playlist = _malloc(strlen(optarg) + 200);
                if (opts.playlist)
                    sprintf(opts.playlist,
                            "<satip:X_SATIPM3U "
                            "xmlns:satip=\"urn:ses-com:satip\">%s</"
                            "satip:X_SATIPM3U>\r\n",
                            optarg);
            } else
                LOG("playlist length is too big %zd bytes", strlen(optarg));
            break;
        }

        case ENABLE_ADAPTERS_OPT: {
            enable_adapters(optarg);
            break;
        }

        case UNICABLE_OPT: {
            set_unicable_adapters(optarg, SWITCH_UNICABLE);
            break;
        }

        case JESS_OPT: {
            set_unicable_adapters(optarg, SWITCH_JESS);
            break;
        }

        case DISEQC_OPT: {
            set_diseqc_adapters(optarg);
            break;
        }

        case DISEQC_TIMING_OPT: {
            set_diseqc_timing(optarg);
            break;
        }

        case DISEQC_MULTI: {
            set_diseqc_multi(optarg);
            break;
        }

        case LNB_OPT: {
            set_lnb_adapters(optarg);
            break;
        }

        case SLAVE_OPT: {
            set_slave_adapters(optarg);
            break;
        }
#ifndef DISABLE_DDCI
        case DISABLE_CAT_OPT: {
            disable_cat_adapters(optarg);
            break;
        }
#endif
        case ADAPTERTIMEOUT_OPT: {
            set_timeout_adapters(optarg);
            break;
        }

        case DELSYS_OPT: {
            set_adapters_delsys(optarg);
            break;
        }

        case SIGNALMULTIPLIER_OPT: {
            set_signal_multiplier(optarg);
            break;
        }

        case DVBAPI_OPT: {
#ifdef DISABLE_DVBAPI
            LOG("%s was not compiled with DVBAPI support, please install "
                "libdvbcsa "
                "(libdvbcsa-dev in Ubuntu) and change the Makefile",
                app_name);
            exit(0);
#else
            parse_dvbapi_opt(optarg, &opts);
#endif
            break;
        }

        case LEGACY_DVBAPI_OPT: {
            set_adapters_legacy_dvbapi(optarg);
            break;
        }

        case RTSPPORT_OPT: {
            opts.rtsp_port = atoi(optarg);
            break;
        }

#ifdef DISABLE_SATIPCLIENT
        case SATIPCLIENT_OPT:
        case SATIPXML_OPT:
        case SATIP_TCP_OPT:
        case SATIPC_RECV_BUFFER_OPT:

            LOG("%s was not compiled with satip client support, please change "
                "the "
                "Makefile",
                app_name);
            exit(0);

#else

        case SATIPCLIENT_OPT:
            if (!opts.satip_servers)
                opts.satip_servers = init_satip_pointer(SATIP_STR_LEN);
            if (!opts.satip_servers)
                break;
            if (strlen(optarg) + strlen(opts.satip_servers) > SATIP_STR_LEN)
                break;

            if (opts.satip_servers[0])
                sprintf(opts.satip_servers + strlen(opts.satip_servers), ",%s",
                        optarg);
            else
                sprintf(opts.satip_servers, "%s", optarg);

            break;

        case SATIPXML_OPT:
            if (!opts.satip_xml)
                opts.satip_xml = init_satip_pointer(SATIP_STR_LEN);
            if (!opts.satip_xml)
                break;
            if (strlen(optarg) + strlen(opts.satip_xml) > SATIP_STR_LEN)
                break;

            if (opts.satip_xml[0])
                sprintf(opts.satip_xml + strlen(opts.satip_xml), " %s", optarg);
            else
                sprintf(opts.satip_xml, "%s", optarg);
            break;

        case SATIP_TCP_OPT:
            opts.satip_rtsp_over_tcp = 1;
            break;

        case SATIPC_RECV_BUFFER_OPT: {
            int val = atoi(optarg);
            opts.satipc_buffer = val * 1024;
            break;
        }
#endif
        case NETCVCLIENT_OPT: {
#ifdef DISABLE_NETCVCLIENT
            LOG("%s was not compiled with netceiver client support, please "
                "change "
                "the Makefile",
                app_name);
            exit(0);
#else
            // parse network interface name and number of netceivers
            char *sep1 = strchr(optarg, ':');
            if (sep1 != NULL) {
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

        case CACHE_DIR_OPT:
            opts.cache_dir = optarg;
            break;

        case THREADS_OPT:
            opts.no_threads = 1 - opts.no_threads;
            break;

        case DROP_ENCRYPTED_OPT:
            opts.drop_encrypted =
                (opts.drop_encrypted == 0) ? 2 : (opts.drop_encrypted - 1);
            break;

        case XML_OPT:
            while (*optarg > 0 && *optarg == '/')
                optarg++;
            if (*optarg > 0)
                opts.xml_path = optarg;
            else
                LOG("Not a valid path for the xml file");
            break;

        case DEMUXDEV_OPT: {
            int o = atoi(optarg);
            if (o >= 0 && o < 4)
                opts.use_demux_device = o;
            else
                LOG("Demux device can be 0-3 and not %d", o);
        } break;

        case ABSOLUTE_SRC:
            set_absolute_src(optarg);
            break;

        case DISABLE_PMT_SCAN:
            opts.pmt_scan = 1 - opts.pmt_scan;
            break;

#ifdef AXE
        case QUATTRO_OPT:
            opts.quattro = 1;
            break;

        case QUATTRO_HIBAND_OPT:
            opts.quattro_hiband = atoi(optarg) + 1;
            break;

        case AXE_POWER:
            opts.axe_power = atoi(optarg) + 1;
            break;

#endif
#ifndef DISABLE_DVBCA
        case CA_PIN_OPT:
            set_ca_adapter_pin(optarg);
            break;

        case FORCE_CI_OPT:
            set_ca_adapter_force_ci(optarg);
            break;

        case CA_CHANNELS_OPT:
            set_ca_channels(optarg);
            break;

#endif
        }
    }

    if (!opts.bind)
        lip = getlocalip();
    else
        lip = opts.bind;

    if (!opts.http_host) {
        opts.http_host = (char *)_malloc(MAX_HOST);
        sprintf(opts.http_host, "%s:%u", lip, opts.http_port);
    }

    opts.rtsp_host = (char *)_malloc(MAX_HOST);
    sprintf(opts.rtsp_host, "%s:%d", lip, opts.rtsp_port);

    LOG("Listening configuration RTSP:%s (bind address: %s), HTTP:%s (bind address: %s)",
        opts.rtsp_host ? opts.rtsp_host : "(null)",
        opts.bind ? opts.bind : "(null)",
        opts.http_host ? opts.http_host : "(null)",
        opts.bind_http ? opts.bind_http : opts.bind ? opts.bind : "(null)");

    opts.datetime_compile = (char *)_malloc(64);
    sprintf(opts.datetime_compile, "%s | %s", __DATE__, __TIME__);

    time(&opts.start_time);
    struct tm *info;
    info = localtime(&opts.start_time);
    opts.datetime_start = (char *)_malloc(32);
    sprintf(opts.datetime_start, "%s ", asctime(info));
    opts.datetime_start[24] = ' ';
    opts.datetime_start[25] = '\0';

    opts.datetime_current = (char *)_malloc(32);
    sprintf(opts.datetime_current, "%s", " ");
    opts.time_running = (char *)_malloc(32);
    sprintf(opts.time_running, "%s", "0");

    opts.run_user = (int)getuid();
    opts.run_pid = (int)getpid();

    if (!is_log)
        opts.log = 0;

    // FBC setup
    if (!access("/proc/stb/frontend/0/fbc_connect", W_OK))
        set_slave_adapters("2-7:0");
}

#define RBUF 32000

int read_rtsp(sockets *s) {
    char *arg[50];
    int cseq, la, i, rlen;
    char *transport = NULL, *useragent = NULL;
    int sess_id = 0;
    char buf[2000];
    char ra[50];
    streams *sid = get_sid(s->sid);

    if (s->rlen > 3 && s->buf[0] == 0x24 && s->buf[1] < 2) {
        if (sid)
            sid->rtime = s->rtime;

        int rtsp_len = s->buf[2] * 256 + s->buf[3];
        LOG("Received RTSP over tcp packet (sock_id %d) sid %d, rlen %d, "
            "packet "
            "len: %d, type %02X %02X discarding %s...",
            s->id, s->sid, s->rlen, rtsp_len, s->buf[4], s->buf[5],
            (s->rlen == rtsp_len + 4) ? "complete" : "fragment");
        if (s->rlen >= rtsp_len + 4) { // we did not receive the entire packet
            memmove(s->buf, s->buf + rtsp_len + 4, s->rlen - (rtsp_len + 4));
            s->rlen -= rtsp_len + 4;
            if (s->rlen == 0)
                return 0;
        } else { // handle the remaining data as next fragment
            rtsp_len -= s->rlen - 4;
            s->rlen = 4;
            s->buf[2] = rtsp_len >> 8;
            s->buf[3] = rtsp_len & 0xFF;
            return 0;
        }
    }

    if ((s->rlen > 0) &&
        (s->rlen < 4 || !end_of_header((char *)s->buf + s->rlen - 4))) {
        if (s->rlen > RBUF - 10) {
            LOG("Discarding %d bytes from the socket buffer, request > %d, "
                "consider "
                "increasing  RBUF",
                s->rlen, RBUF);
            s->rlen = 0;
        }
        LOG("read_rtsp: read %d bytes from handle %d, sock_id %d, flags %d not "
            "ending with \\r\\n\\r\\n",
            s->rlen, s->sock, s->id, s->flags);
        hexdump("read_rtsp: ", s->buf, s->rlen);
        if (s->flags & 1)
            return 0;
        unsigned char *new_alloc = _malloc(RBUF);
        memcpy(new_alloc, s->buf, s->rlen);
        s->buf = new_alloc;
        s->flags = s->flags | 1;
        return 0;
    }

    rlen = s->rlen;
    s->rlen = 0;

    LOG("Read RTSP (sock %d, handle %d) [%s:%d] sid %d, len: %d", s->id,
        s->sock, get_sockaddr_host(s->sa, ra, sizeof(ra)),
        get_sockaddr_port(s->sa), s->sid, rlen);
    LOGM("MSG client >> process :\n%s", s->buf); //LOGM->LOG

    if ((s->type != TYPE_HTTP) &&
        (strncasecmp((const char *)s->buf, "GET", 3) == 0)) {
        http_response(s, 404, NULL, NULL, 0, 0);
        return 0;
    }

    la = split(arg, (char *)s->buf, ARRAY_SIZE(arg), ' ');
    cseq = 0;
    if (la < 2)
        LOG_AND_RETURN(
            0,
            "Most likely not an RTSP packet sock_id: %d sid: %d rlen: "
            "%d, dropping ....",
            s->id, s->sid, rlen);

    if (s->sid < 0)
        for (i = 0; i < la; i++)
            if (strncasecmp("Session:", arg[i], 8) == 0) {
                sess_id = map_int(header_parameter(arg, i), NULL);
                s->sid = find_session_id(sess_id);
            }

    if (strstr(arg[1], "freq") || strstr(arg[1], "pids")) {
        sid = (streams *)setup_stream(arg[1], s);
    }

    // setup empty stream, removing this breaks satip tests
    if (!get_sid(s->sid) && ((strncasecmp(arg[0], "PLAY", 4) == 0) ||
                             (strncasecmp(arg[0], "GET", 3) == 0) ||
                             (strncasecmp(arg[0], "SETUP", 5) == 0)))
        sid = (streams *)setup_stream(arg[1], s);

    sid = get_sid(s->sid);
    if (sid) {
        LOGM("Updating sid %d time to %jd, current time %jd", sid->sid,
             s->rtime, getTick());
        sid->rtime = s->rtime;
    }

    if (sess_id)
        set_session_id(s->sid, sess_id);

    for (i = 0; i < la; i++)
        if (strncasecmp("CSeq:", arg[i], 5) == 0)
            cseq = map_int(header_parameter(arg, i), NULL);
        else if (strncasecmp("Transport:", arg[i], 10) == 0) {
            transport = header_parameter(arg, i);

            if (-1 ==
                decode_transport(s, transport, opts.rrtp, opts.start_rtp)) {
                http_response(s, 400, NULL, NULL, cseq, 0);
                return 0;
            }
        } else if (strstr(arg[i], "LIVE555")) {
            if (sid)
                sid->timeout = 0;
        } else if (strstr(arg[i], "Lavf")) {
            if (sid)
                sid->timeout = 0;
        } else if (strncasecmp("User-Agent:", arg[i], 11) == 0)
            useragent = header_parameter(arg, i);
        else if (!useragent && strncasecmp("Server:", arg[i], 7) == 0)
            useragent = header_parameter(arg, i);

    if ((strncasecmp(arg[0], "PLAY", 4) == 0) ||
        (strncasecmp(arg[0], "GET", 3) == 0) ||
        (strncasecmp(arg[0], "SETUP", 5) == 0)) {
        char ra[100];
        int rv;

        if (!(sid = get_sid(s->sid))) {
            http_response(s, 454, NULL, NULL, cseq, 0);
            return 0;
        }

        if (useragent)
            safe_strncpy(sid->useragent, useragent);

        if ((strncasecmp(arg[0], "PLAY", 4) == 0) ||
            (strncasecmp(arg[0], "GET", 3) == 0))
            if ((rv = start_play(sid, s)) < 0) {
                http_response(s, -rv, NULL, NULL, cseq, 0);
                return 0;
            }
        buf[0] = 0;
        if (transport) {
            int s_timeout;
            char localhost[100];

            if (sid->timeout == 1)
                sid->timeout = opts.timeout_sec;

            s_timeout =
                ((sid->timeout > 20000) ? sid->timeout : opts.timeout_sec) /
                1000;
            get_stream_rhost(sid->sid, ra, sizeof(ra));

            switch (sid->type) {
            case STREAM_RTSP_UDP:
                if (atoi(ra) < 224)
                    snprintf(
                        buf, sizeof(buf),
                        "Transport: "
                        "RTP/"
                        "AVP;unicast;destination=%s;source=%s;client_port=%d-%"
                        "d;server_"
                        "port=%d-%d\r\nSession: "
                        "%010d;timeout=%d\r\ncom.ses.streamID: %d",
                        ra,
                        get_sock_shost(s->sock, localhost, sizeof(localhost)),
                        get_stream_rport(sid->sid),
                        get_stream_rport(sid->sid) + 1,
                        get_sock_sport(sid->rsock), get_sock_sport(sid->rtcp),
                        get_session_id(s->sid), s_timeout, sid->sid + 1);
                else
                    snprintf(
                        buf, sizeof(buf),
                        "Transport: "
                        "RTP/"
                        "AVP;multicast;destination=%s;port=%d-%d\r\nSession: "
                        "%010d;timeout=%d\r\ncom.ses.streamID: %d",
                        ra, get_stream_rport(sid->sid),
                        get_stream_rport(sid->sid) + 1, get_session_id(s->sid),
                        s_timeout, sid->sid + 1);
                break;
            case STREAM_RTSP_TCP:
                snprintf(buf, sizeof(buf),
                         "Transport: RTP/AVP/TCP;interleaved=0-1\r\nSession: "
                         "%010d;timeout=%d\r\ncom.ses.streamID: %d",
                         get_session_id(s->sid), s_timeout, sid->sid + 1);
                break;
            }
        }

        if (strncasecmp(arg[0], "PLAY", 4) == 0) {
            char *qm = strchr(arg[1], '?');
            if (qm)
                *qm = 0;
            if (buf[0])
                strcat(buf, "\r\n");

            snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf) - 1,
                     "RTP-Info: url=%s;seq=%d;rtptime=%jd\r\nRange: npt=0.000-",
                     arg[1], sid->seq, (getTickUs() / 1000000));
        }
        if (buf[0] == 0 && sid->type == STREAM_HTTP)
            snprintf(buf, sizeof(buf),
                     "Content-Type: video/mp2t\r\nConnection: close");
        http_response(s, 200, buf, NULL, cseq, 0);
    } else if (strncmp(arg[0], "TEARDOWN", 8) == 0) {
        buf[0] = 0;
        if (get_sid(s->sid))
            sprintf(buf, "Session: %010d", get_session_id(s->sid));
        close_stream(s->sid);
        http_response(s, 200, buf, NULL, cseq, 0);
    } else if (strncmp(arg[0], "DESCRIBE", 8) == 0) {
        char sbuf[1000];
        char localhost[100];
        char *rv;
        rv = describe_streams(s, arg[1], sbuf, sizeof(sbuf));
        if (!rv) {
            http_response(s, 404, NULL, NULL, cseq, 0);
            return 0;
        }
        snprintf(buf, sizeof(buf),
                 "Content-type: application/sdp\r\nContent-Base: rtsp://%s/",
                 get_sock_shost(s->sock, localhost, sizeof(localhost)));
        http_response(s, 200, buf, sbuf, cseq, 0);
    } else if (strncmp(arg[0], "OPTIONS", 7) == 0) {
        http_response(s, 200, public_str, NULL, cseq, 0);
    } else {
        http_response(s, 501, public_str, NULL, cseq, 0);
    }
    return 0;
}

#define REPLY_AND_RETURN(c)                                                    \
    {                                                                          \
        http_response(s, c, NULL, NULL, 0, 0);                                 \
        return 0;                                                              \
    }

int read_http(sockets *s) {
    char *arg[50];
    char buf[2000]; // the XML should not be larger than 1400 as it will create
                    // problems
    char url[300];
    char ra[50];
    char *space;
    int is_head = 0;
    static char *xml =
        "<?xml version=\"1.0\"?>"
        "<root xmlns=\"urn:schemas-upnp-org:device-1-0\" configId=\"1\">"
        "<specVersion><major>1</major><minor>1</minor></specVersion>"
        "<device><deviceType>urn:ses-com:device:SatIPServer:1</deviceType>"
        "<friendlyName>%s</friendlyName><manufacturer>cata</manufacturer>"
        "<manufacturerURL>http://github.com/catalinii/minisatip</"
        "manufacturerURL>"
        "<modelDescription>%s for "
        "Linux</modelDescription><modelName>%s</modelName>"
        "<modelNumber>1.1</modelNumber><modelURL></modelURL><serialNumber>1</"
        "serialNumber><UDN>uuid:%s</UDN>"
        "<iconList>"
        "<icon><mimetype>image/png</mimetype><width>48</width><height>48</"
        "height><depth>24</depth><url>/sm.png</url></icon>"
        "<icon><mimetype>image/png</mimetype><width>120</width><height>120</"
        "height><depth>24</depth><url>/lr.png</url></icon>"
        "<icon><mimetype>image/jpeg</mimetype><width>48</width><height>48</"
        "height><depth>24</depth><url>/sm.jpg</url></icon>"
        "<icon><mimetype>image/jpeg</mimetype><width>120</width><height>120</"
        "height><depth>24</depth><url>/lr.jpg</url></icon>"
        "</iconList>"
        "<presentationURL>http://%s/</presentationURL>\r\n"
        "<satip:X_SATIPCAP "
        "xmlns:satip=\"urn:ses-com:satip\">%s</satip:X_SATIPCAP>"
        "%s"
        "</device></root>";
    if (s->rlen < 5 || !end_of_header((char *)s->buf + s->rlen - 4)) {
        if (s->rlen > RBUF - 10) {
            LOG("Discarding %d bytes from the socket buffer, request > %d, "
                "consider "
                "increasing  RBUF",
                s->rlen, RBUF);
            s->rlen = 0;
        }
        if (s->flags & 1)
            return 0;
        unsigned char *new_alloc = _malloc(RBUF);
        memcpy(new_alloc, s->buf, s->rlen);
        set_socket_buffer(s->id, new_alloc, RBUF);
        s->flags = s->flags | 1;
        return 0;
    }
    url[0] = 0;
    space = strchr((char *)s->buf, ' ');
    if (space) {
        int i = 0;
        space++;
        while (space[i] && space[i] != ' ') {
            url[i] = space[i];
            if (++i > sizeof(url) - 4)
                break;
        }
        url[i] = 0;
    }

    if (strstr(url, "/?") && !strncasecmp((const char *)s->buf, "GET ", 4)) {
        read_rtsp(s);
        return 0;
    }

    sockets_timeout(s->id, 1); // close the connection

    if (!strncasecmp((const char *)s->buf, "HEAD ", 5))
        is_head = 1;

    if (is_head && strstr(url, "/?")) {
        http_response(s, 200, NULL, NULL, 0, 0);
        return 0;
    }
    s->rlen = 0;

    time_t now;
    struct tm *info;
    time(&now);
    info = localtime(&now);
    sprintf(opts.datetime_current, "%s ", asctime(info));
    opts.datetime_current[24] = ' ';
    opts.datetime_current[25] = '\0';

    double seconds = difftime(now, opts.start_time);
    int days = seconds / 60 / 60 / 24;
    seconds -= days * 24 * 60 * 60;
    int hours = seconds / 60 / 60;
    seconds -= hours * 60 * 60;
    int mins = seconds / 60;
    seconds -= mins * 60;
    int secs = seconds;
    sprintf(opts.time_running, "%.0d%s%02d:%02d:%02d", days,
            days > 0 ? "d " : "", hours, mins, secs);

    LOG("Read HTTP (handle %d) [%s:%d] sid %d, sock %d", s->sid,
        get_sockaddr_host(s->sa, ra, sizeof(ra)), get_sockaddr_port(s->sa),
        s->sid, s->sock);
    LOGM("MSG client >> process :\n%s", s->buf);

    split(arg, (char *)s->buf, ARRAY_SIZE(arg), ' ');
    //      LOG("args: %s -> %s -> %s",arg[0],arg[1],arg[2]);
    if (strncmp(arg[0], "GET", 3) && strncmp(arg[0], "POST", 4) && !is_head)
        REPLY_AND_RETURN(503);

    if (!opts.disable_ssdp)
        ssdp_discovery(s);

    if (strcmp(arg[1], "/" DESC_XML) == 0) {
        extern int tuner_s2, tuner_t, tuner_c, tuner_t2, tuner_c2, tuner_at,
            tuner_ac;
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
        if (tuner_at)
            sprintf(adapters + strlen(adapters), "ATSCT-%d,", tuner_at);
        if (tuner_ac)
            sprintf(adapters + strlen(adapters), "ATSCC-%d,", tuner_ac);
        if (tuner_s2 + tuner_t + tuner_c + tuner_t2 + tuner_c2 + tuner_at +
                tuner_ac ==
            0)
            strcpy(adapters, "DVBS2-0,");
        adapters[strlen(adapters) - 1] = 0;
        snprintf(buf, sizeof(buf), xml, opts.name_app, app_name, opts.name_app,
                 opts.uuid, opts.http_host, adapters,
                 opts.playlist ? opts.playlist : "");
        sprintf(headers,
                "Cache-Control: no-cache\r\nContent-type: "
                "text/xml\r\nX-SATIP-RTSP-Port: %d\r\nConnection: close",
                opts.rtsp_port);
        http_response(s, 200, headers, buf, 0, 0);
        return 0;
    }

    if (strcmp(arg[1], "/state.json") == 0) {
        char *buf = _malloc(JSON_STATE_MAXLEN);
        char *sbuf = _malloc(JSON_STRING_MAXLEN);
        memset(sbuf, 0, JSON_STRING_MAXLEN);
        int len =
            get_json_state(buf, JSON_STATE_MAXLEN, sbuf, JSON_STRING_MAXLEN);
        http_response(s, 200,
                      "Content-Type: application/json; charset=utf-8\r\n"
                      "Connection: close\r\n"
                      "Access-Control-Allow-Origin: *",
                      buf, 0, len);
        _free(buf);
        _free(sbuf);
        return 0;
    }

    if (strcmp(arg[1], "/bandwidth.json") == 0) {
        char *buf = _malloc(JSON_BANDWIDTH_MAXLEN);
        int len = get_json_bandwidth(buf, JSON_BANDWIDTH_MAXLEN);
        http_response(s, 200,
                      "Content-Type: application/json; charset=utf-8\r\n"
                      "Connection: close\r\n"
                      "Access-Control-Allow-Origin: *",
                      buf, 0, len);
        _free(buf);
        return 0;
    }

    // process file from html directory, the images are just sent back

    if (!strcmp(arg[1], "/"))
        arg[1] = "/status.html";

    if (!strchr(arg[1] + 1, '/') && !strstr(arg[1], "..")) {
        char ctype[500];
        char *f;
        int nl;

        f = readfile(arg[1], ctype, &nl);
        if (!f) {
            http_response(s, 404, NULL, NULL, 0, 0);
            return 0;
        }
        if (is_head) {
            http_response(s, 200, ctype, NULL, 0, 0);
            return 0;
        }
        if (strstr(ctype, "html") || strstr(ctype, "xml")) {
            process_file(s, f, nl, ctype);
            closefile(f, nl);
            return 0;
        }

        http_response(s, 200, ctype, f, 0, nl);
        closefile(f, nl);

        return 0;
    }

    http_response(s, 404, NULL, NULL, 0, 0);
    return 0;
}

int close_http(sockets *s) {
    streams *sid = get_sid(s->sid);
    if ((s->flags & 1) && s->buf)
        _free(s->buf);
    s->flags = 0;
    s->buf = NULL;
    LOG("Requested sid close %d timeout %d type %d, sock %d, handle %d, "
        "timeout "
        "%d",
        s->sid, sid ? sid->timeout : -1, sid ? sid->type : -1, s->id, s->sock,
        s->timeout_ms);
    if (sid && ((sid->type == STREAM_RTSP_UDP && sid->timeout != 0) ||
                (sid->type == 0 && sid->timeout != 0)))
        // Do not close rtsp udp as most likely there was no TEARDOWN at this
        // point
        return 0;
    close_stream(s->sid);
    return 0;
}

#undef DEFAULT_LOG
#define DEFAULT_LOG LOG_SSDP

int ssdp_notify(sockets *s, int alive) {
    char buf[500];
    char nt[3][60];
    int ptr = 0;
    char *op = alive ? "Discovery" : "ByeBye";

    int i;
    s->wtime = getTick();

    USockAddr ssdp_sa;
    // use IPv4 only as disc_host is multicast IPv4
    fill_sockaddr(&ssdp_sa, opts.disc_host, 1900, 1);
    strcpy(nt[0], "::upnp:rootdevice");
    sprintf(nt[1], "::uuid:%s", opts.uuid);
    strcpy(nt[2], "::urn:ses-com:device:SatIPServer:1");

    if (s->type != TYPE_UDP)
        return 0;

    LOGM("%s: bootid: %d deviceid: %d http: %s", __FUNCTION__, opts.bootid,
         opts.device_id, opts.http_host);

    for (i = 0; i < 3; i++) {
        strcatf(buf, ptr, "NOTIFY * HTTP/1.1\r\n");
        strcatf(buf, ptr, "HOST: %s:1900\r\n", opts.disc_host);
        if (alive)
            strcatf(buf, ptr, "CACHE-CONTROL: max-age=1800\r\n");
        strcatf(buf, ptr, "LOCATION: http://%s/%s\r\n", opts.http_host,
                opts.xml_path);
        strcatf(buf, ptr, "NT: %s\r\n", nt[i] + 2);
        strcatf(buf, ptr, "NTS: ssdp:%s\r\n", alive ? "alive" : "byebye");
        if (alive)
            strcatf(buf, ptr, "SERVER: Linux/1.0 UPnP/1.1 %s/%s\r\n",
                    opts.name_app, version);
        strcatf(buf, ptr, "USN: uuid:%s%s\r\n", opts.uuid, i == 1 ? "" : nt[i]);
        strcatf(buf, ptr, "BOOTID.UPNP.ORG: %d\r\n", opts.bootid);
        strcatf(buf, ptr, "CONFIGID.UPNP.ORG: 0\r\n");
        if (alive)
            strcatf(buf, ptr, "DEVICEID.SES.COM: %d", opts.device_id);
        strcatf(buf, ptr, "\r\n\r\n");

        LOGM("%s packet %d:\n%s", op, i + 1, buf);
        int wb = sendto(s->sock, buf, ptr, MSG_NOSIGNAL, &ssdp_sa.sa,
                        SOCKADDR_SIZE(ssdp_sa));
        if (wb != ptr)
            LOG("incomplete ssdp_discovery: wrote %d out of %d: error %d: %s",
                wb, ptr, errno, strerror(errno));
        ptr = 0;
    }
    s->rtime = getTick();
    return 0;
}

int ssdp_discovery_sent = 0;
int ssdp_discovery(sockets *s) {
    // Send discovery once only
    if (!ssdp_discovery_sent) {
        ssdp_discovery_sent = 1;
        return ssdp_notify(s, 1);
    }
    return 0;
}

int ssdp_byebye(sockets *s) { return ssdp_notify(s, 0); }

int ssdp;
int ssdp_reply(sockets *s) {
    char *reply = "HTTP/1.1 200 OK\r\n"
                  "CACHE-CONTROL: max-age=1800\r\n"
                  "DATE: %s\r\n"
                  "EXT:\r\n"
                  "LOCATION: http://%s/%s\r\n"
                  "SERVER: Linux/1.0 UPnP/1.1 %s/%s\r\n"
                  "ST: urn:ses-com:device:SatIPServer:1\r\n"
                  "USN: uuid:%s::urn:ses-com:device:SatIPServer:1\r\n"
                  "BOOTID.UPNP.ORG: %d\r\n"
                  "CONFIGID.UPNP.ORG: 0\r\n"
                  "DEVICEID.SES.COM: %d\r\n\r\n\0";
    char *device_id_conflict = "M-SEARCH * HTTP/1.1\r\n"
                               "HOST: %s:1900\r\n"
                               "MAN: \"ssdp:discover\"\r\n"
                               "ST: urn:ses-com:device:SatIPServer:1\r\n"
                               "USER-AGENT: Linux/1.0 UPnP/1.1 %s/%s\r\n"
                               "DEVICEID.SES.COM: %d\r\n\r\n\0";
    char *man, *man_sd, *didsescom, *ruuid, *rdid;
    char buf[500];
    char ra[50];
    int did = 0;
    int ptr = 0;

    if (!opts.disable_ssdp)
        ssdp_discovery(s);

    s->rtime = s->wtime; // consider the timeout of the discovery operation

    ruuid = strcasestr((const char *)s->buf, "uuid:");
    if (ruuid && strncmp(opts.uuid, strip(ruuid + 5), strlen(opts.uuid)) == 0) {
        LOGM("Dropping packet from the same UUID as mine (from %s:%d)",
             get_sockaddr_host(s->sa, ra, sizeof(ra)),
             get_sockaddr_port(s->sa));
        return 0;
    }

    // not my uuid

#ifdef AXE
    axe_set_network_led(1);
#endif

    LOGM("Received SSDP packet (handle %d) from %s:%d", s->sock,
         get_sockaddr_host(s->sa, ra, sizeof(ra)), get_sockaddr_port(s->sa));
    LOGM("MSG querier >> process :\n%s", s->buf);

    if (strncasecmp((const char *)s->buf, "NOTIFY", 6) == 0) {
        rdid = strcasestr((const char *)s->buf, "DEVICEID.SES.COM:");
        if (rdid && opts.device_id == map_int(strip(rdid + 17), NULL)) {
            ptr = 0;
            strcatf(buf, ptr, device_id_conflict,
                    opts.bind ? opts.bind : getlocalip(), opts.name_app,
                    version, opts.device_id);
            LOG("A new device joined the network with the same Device ID:  %s, "
                "asking to change DEVICEID.SES.COM",
                get_sockaddr_host(s->sa, ra, sizeof(ra)));
            int wb = sendto(ssdp, buf, ptr, MSG_NOSIGNAL, &s->sa.sa,
                            SOCKADDR_SIZE(s->sa));
            if (wb != ptr)
                LOG("incomplete ssdp_reply notify: wrote %d out of %d: error "
                    "%d: %s",
                    wb, ptr, errno, strerror(errno));
        }

        return 0;
    }

    man = strcasestr((const char *)s->buf, "MAN");
    man_sd = strcasestr((const char *)s->buf, "ssdp:discover");
    if ((didsescom = strcasestr((const char *)s->buf, "DEVICEID.SES.COM:")))
        did = map_int(didsescom + 17, NULL);

    if (man && man_sd && didsescom && (s->rtime < 15000) &&
        did == opts.device_id) // SSDP Device ID clash, only first 5 seconds
                               // after the announcement
    {
        opts.device_id++;
        s[si].timeout_ms = 1800 * 1000;
        s[si].rtime = -s[si].timeout_ms;
        LOG("Device ID conflict, changing our device id to %d, destination "
            "SAT>IP "
            "server %s",
            opts.device_id, get_sockaddr_host(s->sa, ra, sizeof(ra)));
        readBootID();
    } else
        did = opts.device_id;

    if (strncmp((const char *)s->buf, "HTTP/1", 6) == 0)
        LOG_AND_RETURN(0, "ssdp_reply: the message is a reply, ignoring....");

    ptr = 0;
    strcatf(buf, ptr, reply, get_current_timestamp(), opts.http_host,
            opts.xml_path, opts.name_app, version, opts.uuid, opts.bootid, did);

    LOGM("Send Reply SSDP packet (fd: %d) %s:%d, bootid: %d deviceid: %d http: "
         "%s",
         ssdp, get_sockaddr_host(s->sa, ra, sizeof(ra)),
         get_sockaddr_port(s->sa), opts.bootid, did, opts.http_host);
    // use ssdp (unicast) even if received to multicast address
    LOGM("MSG querier << process :\n%s", buf);
    int wb =
        sendto(ssdp, buf, ptr, MSG_NOSIGNAL, &s->sa.sa, SOCKADDR_SIZE(s->sa));
    if (wb != ptr)
        LOG("incomplete ssdp_reply: wrote %d out of %d: error %d: %s", wb, ptr,
            errno, strerror(errno));
    return 0;
}

#undef DEFAULT_LOG
#define DEFAULT_LOG LOG_HTTP

int new_rtsp(sockets *s) {
    s->type = TYPE_RTSP;
    s->action = (socket_action)read_rtsp;
    s->close = (socket_action)close_http;
    if (!set_linux_socket_nonblock(s->sock))
        s->nonblock = 1;
    return 0;
}

int new_http(sockets *s) {
    s->type = TYPE_HTTP;
    s->action = (socket_action)read_http;
    s->close = (socket_action)close_http;
    if (!set_linux_socket_nonblock(s->sock))
        s->nonblock = 1;
    return 0;
}

void write_pid_file() {
    FILE *f;
    sprintf(pid_file, PID_NAME, app_name);
    if ((f = fopen(pid_file, "wt"))) {
        fprintf(f, "%d", getpid());
        fclose(f);
    }
}

pthread_t main_tid;
extern int sock_signal;

#ifndef TESTING

int main(int argc, char *argv[]) {
    int sock_bw, rv, devices;
    int i;
    unsigned long log_it;
    main_tid = get_tid();
    thread_index = 0;
    thread_info[thread_index].tid = main_tid;
    strcpy(thread_info[thread_index].thread_name, "main");
    init_alloc();
    set_options(argc, argv);
    if ((rv = init_utils(argv[0]))) {
        LOG0("init_utils failed with %d", rv);
        return rv;
    }
    if (opts.daemon)
        becomeDaemon();
    LOG("Running mode: %s", opts.daemon ? "background" : "foreground");
    if (opts.slog)
        openlog(app_name,
                LOG_NDELAY | LOG_NOWAIT | LOG_PID |
                    (opts.slog > 1 ? LOG_PERROR : 0),
                LOG_DAEMON);

    print_version(1, 1);
    LOG("Command line: %s", opts.command_line);

    for (i = 0; loglevels[i]; i++) {
        log_it = (1UL << i);
        LOGL(log_it, " LOG  of the module enabled: %s", loglevels[i]);
        DEBUGL(log_it, "DEBUG of the module enabled: %s", loglevels[i]);
    }

    if (mkdir_recursive(opts.cache_dir) == -1) {
        FAIL("Failed to create cache directory %s", opts.cache_dir);
    }
    readBootID();
    if ((rtsp = tcp_listen(opts.bind, opts.rtsp_port, opts.use_ipv4_only)) < 1)
        FAIL("RTSP: Could not listen on port %d", opts.rtsp_port);
    if ((http = tcp_listen(opts.bind_http, opts.http_port, opts.use_ipv4_only)) < 1)
        FAIL("Could not listen on http port %d", opts.http_port);
    if (!opts.disable_ssdp) {
        if ((ssdp = udp_bind(opts.bind, 1900, opts.use_ipv4_only)) < 1)
            FAIL("SSDP: Could not bind on udp port 1900");
        if ((ssdp1 = udp_bind(opts.disc_host, 1900, 1)) < 1)
            FAIL("SSDP: Could not bind on %s udp port 1900", opts.disc_host);
        if (opts.bind_dev) {
#if defined(SO_BINDTODEVICE)
            struct ifreq ifr;
            memset(&ifr, 0, sizeof(ifr));
            snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", opts.bind_dev);
            if (setsockopt(ssdp, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr,
                           sizeof(ifr)) < 0)
                LOG("SSDP: Failed to set SO_BINDTODEVICE to %s", opts.bind_dev);
            if (setsockopt(ssdp1, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr,
                           sizeof(ifr)) < 0)
                LOG("SSDP: Failed to set SO_BINDTODEVICE to %s", opts.bind_dev);
            LOG("SSDP: Bound to device %s", opts.bind_dev);
#else
            LOG("SSDP: Binding to device with SO_BINDTODEVICE not supported!");
#endif
        }

        si = sockets_add(ssdp, NULL, -1, TYPE_UDP, (socket_action)ssdp_reply,
                         (socket_action)ssdp_byebye,
                         (socket_action)ssdp_discovery);
        si1 = sockets_add(ssdp1, NULL, -1, TYPE_UDP, (socket_action)ssdp_reply,
                          (socket_action)ssdp_byebye,
                          (socket_action)ssdp_discovery);
        if (si < 0 || si1 < 0)
            FAIL("sockets_add failed for ssdp");
    }

    sockets_timeout(si, 60 * 1000);
    set_sockets_rtime(si, -60 * 1000);
    if (0 > sockets_add(rtsp, NULL, -1, TYPE_SERVER, (socket_action)new_rtsp,
                        NULL, (socket_action)close_http))
        FAIL("sockets_add failed for rtsp");
    if (0 > sockets_add(http, NULL, -1, TYPE_SERVER, (socket_action)new_http,
                        NULL, (socket_action)close_http))
        FAIL("sockets_add failed for http");

    if (0 > (sock_signal = sockets_add(SOCK_TIMEOUT, NULL, -1, TYPE_UDP, NULL,
                                       NULL, (socket_action)signal_thread)))
        FAIL("sockets_add failed for signal thread");

    if (!opts.no_threads) {
        set_socket_thread(sock_signal, start_new_thread("signal"));
        sockets_timeout(sock_signal, 300); // 300 ms
    } else
        sockets_timeout(sock_signal, 1000); // 1 sec

    if (0 > (sock_bw = sockets_add(SOCK_TIMEOUT, NULL, -1, TYPE_UDP, NULL, NULL,
                                   (socket_action)calculate_bw)))
        FAIL("sockets_add failed for BW calculation");

    set_socket_thread(sock_bw, get_socket_thread(sock_signal));
    sockets_timeout(sock_bw, 1000);
#ifndef DISABLE_SATIPCLIENT
    if (opts.satip_xml) {
        int sock_satip;
        if (0 >
            (sock_satip = sockets_add(SOCK_TIMEOUT, NULL, -1, TYPE_UDP, NULL,
                                      NULL, (socket_action)satip_getxml)))
            FAIL("sockets_add failed for satip_xml retrieval");

        set_socket_thread(sock_satip, get_socket_thread(sock_signal));
        sockets_timeout(sock_satip, 120000);
        set_sockets_rtime(sock_satip, -120000);
        LOG("Added sockets id %d for polling the satip servers", sock_satip);
    }
#endif
#ifndef DISABLE_PMT
    pmt_init();
#endif
    devices = init_all_hw();
    getAdaptersCount();
    LOG0("Initializing with %d devices", devices);

    write_pid_file();
    select_and_execute(NULL);
    unlink(pid_file);
#ifndef DISABLE_PMT
    pmt_destroy();
#endif
    LOG0("Closing...");
    free_all();
    _free(opts.command_line);
    LOG0("Exit OK.");
    if (opts.slog)
        closelog();
    return 0;
}

#endif

int readBootID() {
    opts.bootid = 0;
    int local_device_id = 0;
    char bootid_path[256];

    // Read existing values
    snprintf(bootid_path, sizeof(bootid_path) - 1, "%s/bootid", opts.cache_dir);
    LOG("Initial values bootid %d, device_id %d, UUID %s (reading new from %s)",
        opts.bootid, opts.device_id, opts.uuid, bootid_path);
    FILE *f = fopen(bootid_path, "rt");
    __attribute__((unused)) int rv;
    if (f) {
        rv = fscanf(f, "%d %d %s", &opts.bootid, &local_device_id, opts.uuid);
        fclose(f);
    }

    // Increment bootid and set defaults if values are missing
    opts.bootid++;
    if (local_device_id < 1) {
        local_device_id = 1;
    }
    if (!strcmp(opts.uuid, "")) {
        uuid4_generate(opts.uuid);
    }

    if (opts.device_id < local_device_id) {
        opts.device_id = local_device_id;
    }

    // Store new values
    f = fopen(bootid_path, "wt");
    if (f) {
        fprintf(f, "%d %d %s", opts.bootid, opts.device_id, opts.uuid);
        fclose(f);
    }
    LOG("Running with bootid %d, device_id %d, UUID %s",
        opts.bootid, opts.device_id, opts.uuid);
    return opts.bootid;
}

void http_response(sockets *s, int rc, char *ah, char *desc, int cseq, int lr) {
    int binary = 0;
    char *desc1;
    char ra[50];
    char *d;
    char *proto;

    if (s->type == TYPE_HTTP)
        proto = "HTTP";
    else
        proto = "RTSP";

    // Check if the request is a "reGET" (successive non-standard HTTP request
    // while streaming) Used when one HTTP client wants to change pids with a
    // new GET request. This doesn't interrupts the streaming.
    if (s->type == TYPE_HTTP && s->iteration > 1 && rc == 200) {
        LOG("Reply hidden because another GET while streaming (handle %d) "
            "[%s:%d] iteration:%jd, sock %d",
            s->sock, get_sockaddr_host(s->sa, ra, sizeof(ra)),
            get_sockaddr_port(s->sa), s->iteration, s->id);
        return;
    }

    if (!ah || !ah[0])
        ah = public_str;
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
    else {
        d = "Service Unavailable";
        rc = 503;
    }
    char resp[10000];
    desc1 = desc;
    resp[sizeof(resp) - 1] = 0;
    if (!lr)
        lr = strlen(desc);
    else {
        binary = 1;
        desc1 = "";
    }

    int lresp = 0;
    strlcatf(resp, sizeof(resp) - 1, lresp, "%s/1.0 %d %s\r\n", proto, rc, d);
    if (rc != 501)
        strlcatf(resp, sizeof(resp) - 1, lresp, "Date: %s\r\n",
                 get_current_timestamp());
    if (s->type != TYPE_HTTP && get_sid(s->sid) && ah &&
        !strstr(ah, "Session") && rc != 454)
        strlcatf(resp, sizeof(resp) - 1, lresp, "Session: %010d\r\n",
                 get_session_id(s->sid));
    if (s->type != TYPE_HTTP && cseq > 0)
        strlcatf(resp, sizeof(resp) - 1, lresp, "CSeq: %d\r\n", cseq);
    if (rc != 501)
        strlcatf(resp, sizeof(resp) - 1, lresp, "Server: %s/%s\r\n", app_name,
                 version);

    if (rc != 454 && rc != 404)
        strlcatf(resp, sizeof(resp) - 1, lresp, "%s\r\n", ah);

    if (lr > 0) {
        strlcatf(resp, sizeof(resp) - 1, lresp, "Content-Length: %d\r\n\r\n",
                 lr);
        strlcatf(resp, sizeof(resp) - 1, lresp, "%s", desc1);
    } else
        strlcatf(resp, sizeof(resp) - 1, lresp, "\r\n");

    LOG("Reply %s(handle %d) [%s:%d] content_len:%d, sock %d",
        (lresp == sizeof(resp) - 1) ? "(message truncated) " : "", s->sock,
        get_sockaddr_host(s->sa, ra, sizeof(ra)), get_sockaddr_port(s->sa), lr,
        s->id);
    LOGM("MSG client << process :\n%s", resp);

    struct iovec iov[2];
    iov[0].iov_base = resp;
    iov[0].iov_len = strlen(resp);
    if (binary) {
        iov[1].iov_base = desc;
        iov[1].iov_len = lr;
    }
    sockets_writev_prio(s->id, iov, binary ? 2 : 1, 1);
}

#ifdef AXE
int has_axe = 1;
#else
int has_axe = 0;
#endif
#ifndef DISABLE_PMT
int has_pmt = 1;
#else
int has_pmt = 0;
#endif

_symbols minisatip_sym[] = {
    {"has_axe", VAR_INT, &has_axe, 1, 0, 0},
    {"has_pmt", VAR_INT, &has_pmt, 1, 0, 0},
    {"name_app", VAR_PSTRING, &opts.name_app, 0, 0, 0},
    {"http_host", VAR_PSTRING, &opts.http_host, 0, 0, 0},
    {"uuid", VAR_STRING, &opts.uuid, 0, 0, 0},
    {"bootid", VAR_INT, &opts.bootid, 1, 0, 0},
    {"deviceid", VAR_INT, &opts.device_id, 1, 0, 0},
    {"http_port", VAR_INT, &opts.http_port, 1, 0, 0},
    {"rtsp_host", VAR_PSTRING, &opts.rtsp_host, 0, 0, 0},
    {"datetime_compile", VAR_PSTRING, &opts.datetime_compile, 0, 0, 0},
    {"datetime_start", VAR_PSTRING, &opts.datetime_start, 0, 0, 0},
    {"datetime_current", VAR_PSTRING, &opts.datetime_current, 0, 0, 0},
    {"time_running", VAR_PSTRING, &opts.time_running, 0, 0, 0},
    {"run_user", VAR_INT, &opts.run_user, 1, 0, 0},
    {"run_pid", VAR_INT, &opts.run_pid, 1, 0, 0},
    {"version", VAR_STRING, &version, 1, 0, 0},
    {"command_line", VAR_PSTRING, &opts.command_line, 1, 0, 0},
    {NULL, 0, NULL, 0, 0, 0}};
