Welcome to Minisatip

Minisatip is a single threaded satip server version 1.1 that runs under Linux and it was tested with DVB-S, DVB-S2, DVB-T, DVB-T2, DVB-C, DVB-C2, ATSC DVB cards.
The protocol specification can be found at: satip.info/sites/satip/files/resource/satip_specification_version_1_2_2.pdf
It is very lightweight (designed for embedded systems with memory and processing constrains), does not need any additional libraries and can be used by existing satip clients like: Tvheadend, DVBViewer, vdr or Android/iOS applications.
The application is designed to stream the requested data even to multiple clients (even with one dvb card) in the same time and has few additional features, like capping the bandwidth or forcing the application to send the rtp streams to a specified address.
It is tested on x86_64, x86, ARM and MIPS platforms and it requires DVBAPI 5. Supported protocols are rtsp (both tcp and udp), HTTP (port 8080) and SSDP (as specified in the SAT>IP documentation).

Please use https://toda.ro/projects/minisatip/issues/new for features requests.
In order to speed up the investigation of an issue, please provide the full log and a link to the application that is not working.
minisatip requires libdvbcsa development files. If you want to compile minisatip without libdvbcsa, remove # from the line #NODVBCSA=-DDISABLE_DVBCSA in Makefile

Usage:

minisatip [-f] [-r remote_rtp_host] [-d device_id] [-w http_server[:port]] [-p public_host] [-s rtp_port] [-a no] [-m mac] [-l] [-a X:Y:Z] [-e X-Y,Z]
		-f foreground, otherwise run in background
		-r remote_rtp_host: send remote rtp to remote_rtp_host
		-d specify the device id (in case there are multiple SAT>IP servers in the network)
		-w http_server[:port]: specify the host and the port where the xml file can be downloaded from
		-x port: port for listening on http
		-y rtsp_port: port for listening for rtsp requests (default: 554)
		-s force to get signal from the DVB hardware every 200ms (use with care, only when needed)
		-a x:y:z simulate x DVB-S2, y DVB-T2 and z DVB-C adapters on this box (0 means autodetect)
		-m xx: simulate xx as local mac address, generates UUID based on mac
		-e list_of_enabled adapters: enable only specified adapters, example 0-2,5,7 (no spaces between parameters)
		-c X: bandwidth capping for the output to the network (default: unlimited)
		-b X: set the DVR buffer to X KB (default: XX KB)
		-l increases the verbosity (you can use multiple -l), logging to stdout in foreground mode or in /tmp/log when a daemon
		-p url: specify playlist url using X_SATIPM3U header
		-u unicable_string: defines the unicable adapters (A) and their slot (S), frequency (F) and optionally the PIN for the switch:\n\
			The format is: A1:S1-F1[-PIN][,A2:S2-F2[-PIN][,...]] example: 2:0-1284[-1111]
		-j jess_string: same format as unicable_string 
		-g use syslog instead stdout for logging, multiple -g - print to stderr as well\n\
		-o host:port: specify the hostname and port for the dvbapi server (oscam) 
		-s DELSYS:host:port - specify the remote satip host and port with delivery system DELSYS: example: dvbs2:192.168.9.9[:554]

Example of Usage:

	minisatip 

	- Will act as a daemon and listen for connections on port 1900 (udp), 554 (rtsp) and 8080 (http) and will be able to serve satip clients connected to the LAN
	
	minisatip -r xx.xx.xx.xx 
	
	- Forces to send the rtp packets to xx.xx.xx.xx instead of the IP that the connection comes from
	
	To run the client and server in different networks:
	
	- On the box/network with the client (simulates 1 adapter and generates the same uuid on both client and server):
	
	minisatip -m 001122334455 -a 1 -w <server_ip>:8080
	
	- On the box with the DVB card:
	
	minisatip -m 001122334455 
	
	- As long as you want to use the channels on the same frequency, you can use -a or -t parameter to report multiple tuners to the client so you can watch/record multiple channels from the same frequency (using tvheadend for example)
	
	- Connect to oscam using the dvbapi protocol
	
	minisatip -o 192.168.9.9:9000
	
Examples:
	- In order to listen to a radio on Hotbird 13E, after minisatip is started open the following URL in your favourite media player:

	"http://MINISATIP_HOST:8080/?msys=dvbs&freq=11623&pol=v&sr=27500&pids=0,10750,254"
	
	msys can be one of: dvbs, dvbs2, dvbt, dvbt2, dvbc, dvbc2, atsc, dvbcb ( - DVBC_ANNEX_B )
	

   	
