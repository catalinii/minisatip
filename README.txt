Welcome to Minisatip

Minisatip is a single threaded satip server version 1.2 that runs under Linux and it was tested with DVB-S, DVB-S2, DVB-T, DVB-T2, DVB-C, DVB-C2, ATSC and ISDB-T cards.
The protocol specification can be found at: satip.info/sites/satip/files/resource/satip_specification_version_1_2_2.pdf
It is very lightweight (designed for embedded systems with memory and processing constrains), does not need any additional libraries for basic functionality and can be used by existing satip clients like: Tvheadend, DVBViewer, vdr or Android/iOS applications. minisatip can act as a satip client as well in order to connect to satip servers from different networks.
The application is designed to stream the requested data to multiple clients (even with one dvb card) in the same time while opening different pids.
It is tested on x86_64, x86, ARM and MIPS platforms and it requires DVBAPI 5. Supported protocols are rtsp (both tcp and udp), HTTP (port 8080) and SSDP (as specified in the SAT>IP documentation). On top of that, it supports dvbapi protocol implemented by oscam (requires dvbcsa library) to decrypt channels using an official subscription and support dvbca protocol (requires dvben50221 library) for dvb cards with CA hardware. In order to enable/disable features, please edit the Makefile. 

Please use https://toda.ro/forum/ for any questions.
In order to speed up the investigation of an issue, please provide the full log and a link to the application that is not working.

If you like minisatip and you want to support the development of the project please make a donation: 

https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=7UWQ7FXSABUH8&item_name=minisatip&currency_code=EUR&bn=PP-DonationsBF:btn_donateCC_LG.gif:NonHostedGuest

Usage:

minisatip [-[fgltz]] [-a x:y:z] [-b X:Y] [-c X] [-d A:C-U ] [-D device_id] [-e X-Y,Z] [-i prio] [-j A1:S1-F1[-PIN]] [-m mac] [-o oscam_host:dvbapi_port] [-p public_host] [-r remote_rtp_host] [-R document_root] [-s [DELSYS:]host[:port] [-u A1:S1-F1[-PIN]] [-w http_server[:port]] [-x http_port] [-X xml_path] [-y rtsp_port] 

 
-a x:y:z simulate x DVB-S2, y DVB-T2 and z DVB-C adapters on this box (0 means auto-detect)
	eg: -a 1:2:3  
	- it will report 1 dvb-s2 device, 2 dvb-t2 devices and 3 dvb-c devices 

-b --buffers X:Y : set the app adapter buffer to X Bytes (default: 25004) and set the kernel DVB buffer to Y Bytes (default: 5775360) - both multiple of 188
	eg: -b 18800:18988

-c X: bandwidth capping for the output to the network [default: unlimited]
	eg: -c 2048  (does not allow minisatip to send more than 2048KB/s to all remote servers)

-d --diseqc ADAPTER1:COMMITED1-UNCOMMITED1[,ADAPTER2:COMMITED2-UNCOMMITED2[,...]
	The first argument is the adapter number, second is the number of commited packets to send to a Diseqc 1.0 switch, third the number of uncommited commands to sent to a Diseqc 1.1 switch
	The higher number between the commited and uncommited will be sent first.
	eg: -d 0:1-0  (which is the default for each adapter).

-D --device-id DVC_ID: specify the device id (in case there are multiple SAT>IP servers in the network)
 	eg: -D 4 

-e --enable-adapters list_of_enabled adapters: enable only specified adapters
	eg: -e 0-2,5,7 (no spaces between parameters)
	- keep in mind that the first adapters are the local ones starting with 0 after that are the satip adapters 
	if you have 3 local dvb cards 0-2 will be the local adapters, 3,4, ... will be the satip servers specified with argument -s

-f  foreground, otherwise run in background

-g use syslog instead stdout for logging, multiple -g - print to stderr as well

-i --priority prio: set the process priority to prio (-10 increases the priority by 10)

-l increases the verbosity (you can use multiple -l), logging to stdout in foreground mode or in /tmp/log when a daemon
	eg: -l -l -l

-m xx: simulate xx as local mac address, generates UUID based on mac
	-m 001122334455 

-o --dvbapi host:port - specify the hostname and port for the dvbapi server (oscam) 
	eg: -o 192.168.9.9:9000 
	192.168.9.9 is the host where oscam is running and 9000 is the port configured in dvbapi section in oscam.conf

-p url: specify playlist url using X_SATIPM3U header 
	eg: -p http://192.168.2.3:8080/playlist
	- this will add X_SATIPM3U tag into the satip description xml

-r --remote-rtp  remote_rtp_host: send the rtp stream to remote_rtp_host instead of the ip the connection comes from
 	eg: -r 192.168.7.9
 
-R --document-root directory: document root for the minisatip web page and images

-s --satip-servers DELSYS:host:port - specify the remote satip host and port with delivery system DELSYS, it is possible to use multiple -s 
	DELSYS - can be one of: dvbs, dvbs2, dvbt, dvbt2, dvbc, dvbc2, isdbt, atsc, dvbcb ( - DVBC_ANNEX_B ) [default: dvbs2]
	host - the server of the satip server
	port - rtsp port for the satip server [default: 554]
	eg: -s 192.168.1.2 -s dvbt:192.168.1.3:554 -s dvbc:192.168.1.4
	- specifies 1 dvbs2 (and dvbs)satip server with address 192.168.1.2:554
	- specifies 1 dvbt satip server  with address 192.168.1.3:554
	- specifies 1 dvbc satip server  with address 192.168.1.4:554

-t --cleanpsi clean the PSI from all CA information, the client will see the channel as clear if decrypted successfully

-u --unicable unicable_string: defines the unicable adapters (A) and their slot (S), frequency (F) and optionally the PIN for the switch:
	The format is: A1:S1-F1[-PIN][,A2:S2-F2[-PIN][,...]]
	eg: 2:0-1284[-1111]

-j --jess jess_string - same format as -u 

-w --http-host http_server[:port]: specify the host and the port (if not 80) where the xml file can be downloaded from [default: default_local_ip_address:8080] 
	eg: -w 192.168.1.1:8080 

-x --http-port port: port for listening on http [default: 8080]
	eg: -x 9090 

-X --xml PATH: the path to the xml that is provided as part of the satip protocol	
    by default desc.xml is provided by minisatip without needing an additional file, 
    however satip.xml is included if it needs to be customized

-y --rtsp-port rtsp_port: port for listening for rtsp requests [default: 554]
	eg: -y 5544 
	- changing this to a port > 1024 removes the requirement for minisatip to run as root

-z --check-signal force to get signal from the DVB hardware every 200ms (use with care, only when needed)
	- retrieving signal could take sometimes more than 200ms which could impact the rtp stream, using it only when you need to adjust your dish

Streaming examples:

- In order to listen to a radio on Hotbird 13E, after minisatip is started open the following URL in your favourite media player:

"http://MINISATIP_HOST:8080/?msys=dvbs&freq=11623&pol=v&sr=27500&pids=0,10750,254"

- In order to listen to a radio on Astra 19.2E, after minisatip is started open the following URL in your favourite media player:

"http://MINISATIP_HOST:8080/?msys=dvbs&freq=12266&pol=h&sr=27500&pids=0,851"	

- msys can be one of: dvbs, dvbs2, dvbt, dvbt2, dvbc, dvbc2, atsc, isdbt, dvbcb ( - DVBC_ANNEX_B )
	

   	
