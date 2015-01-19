Welcome to Minisatip

Minisatip is a single threaded satip server version 1.1 that runs under Linux and it was tested with DVB-S, DVB-S2, DVB-T, DVB-T2 and DVB-C DVB cards.
The protocol specification can be found at: http://www.satip.info/sites/satip/files/resource/satip_specification_version_1_2_1.pdf
It is very lightweight (designed for embedded systems with memory and processing constrains), does not need any additional libraries and can be used by existing satip clients like: Tvheadend, DVBViewer.
The application is designed to stream the requested data even to multiple clients (even with one dvb card) in the same time and has few additional features, like capping the bandwidth or forcing the application to send the rtp streams to a specified address. 

Usage:

minisatip [-f] [-r remote_rtp_host] [-d discovery_host] [-w http_server[:port]] [-p public_host] [-s rtp_port] [-a no] [-m mac] [-l]
		-f foreground, otherwise run in background
		-r remote_rtp_host: send remote rtp to remote_rtp_host
		-d send multicast annoucement to discovery_host instead
		-w http_server[:port]: specify the host and the port where the xml file can be downloaded from
		-x port: port for listening on http
		-s force to get signal from the DVB hardware every 200ms (use with care, onle when needed)
		-a x:y:z simulate x DVB-S2, y DVB-T2 and z DVB-C adapters on this box (0 means autodetect)
		-m xx: simulate xx as local mac address, generates UUID based on mac
		-c X: bandwidth capping for the output to the network (default: unlimited)
		-b X: set the DVR buffer to X KB (default: XX KB)
		-l increases the verbosity (you can use multiple -l), logging to stdout in foreground mode or in /tmp/log when a daemon

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
	
	- As long as you want to use the channels on the same frequency, you can use -a or -t parameter to report multiple tunners to the client so you can watch/record multiple channels from the same frequency (using tvheadend for example)
   	
