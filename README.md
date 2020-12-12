<a href="https://scan.coverity.com/projects/catalinii-minisatip">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/18049/badge.svg"/>
</a>
<img alt="Build Status" src="https://travis-ci.org/catalinii/minisatip.svg?branch=master" />

# Welcome to Minisatip

Minisatip is a multi-threaded satip server version 1.2 that runs under Linux and it was tested with DVB-S, DVB-S2, DVB-T, DVB-T2, DVB-C, DVB-C2, ATSC and ISDB-T cards.

The protocol specification can be found at: 
http://satip.info/sites/satip/files/resource/satip_specification_version_1_2_2.pdf
It is very lightweight (designed for embedded systems with memory and processing constrains), does not need any additional libraries for basic functionality and can be used by existing satip clients like: Tvheadend, DVBViewer, vdr, VideoLAN or Android/iOS applications. Minisatip can act as a satip client as well in order to connect to satip servers from different networks.

The application is designed to stream the requested data to multiple clients (even with one dvb card) in the same time while opening different pids.

It is tested on x86_64, x86, ARM and MIPS platforms and it requires DVBAPI 5. Supported protocols are RTSP (both tcp and udp), HTTP (port 8080) and SSDP (as specified in the SAT>IP documentation). On top of that, it supports dvbapi protocol implemented by oscam (requires dvbcsa library) to decrypt channels using an official subscription and support dvbca protocol (requires dvben50221 library) for dvb cards with CA hardware. In order to enable/disable features, please edit the Makefile. 

The application shows also a status page by default at the address: http://IP:8080 
The latest binaries for embedded platforms: https://minisatip.org/forum/viewtopic.php?f=5&t=371 

Contact
-------
Please use https://minisatip.org/forum/ for any questions.

In order to speed up the investigation of an issue, please provide the full log and a link to the application that is not working.

If you like minisatip and you want to support the development of the project please make a donation: 
https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=7UWQ7FXSABUH8&item_name=minisatip&currency_code=EUR&bn=PP-DonationsBF:btn_donateCC_LG.gif:NonHostedGuest

Usage:
-------

minisatip version 1.0.4-9c20531, compiled in Dec 12 2020 11:30:51, with s2api version: 050B

	./minisatip [-[fgtzE]] [-a x:y:z] [-b X:Y] [-B X] [-H X:Y] [-d A:C-U ] [-D device_id] [-e X-Y,Z] [-i prio] 
		[-[uj] A1:S1-F1[-PIN]] [-m mac] [-P port] [-l module1[,module2]] [-v module1[,module2]][-o oscam_host:dvbapi_port,offset] [-p public_host] [-r remote_rtp_host] [-R document_root] [-s [*][DELSYS:][FE_ID@][source_ip/]host[:port] [-u A1:S1-F1[-PIN]] [-L A1:low-high-switch] [-w http_server[:port]] 
 	[-x http_port] [-X xml_path] [-y rtsp_port]

Help
-------

* -4 : Force TCP sockets to use IPv6

* -a x:y:z simulate x DVB-S2, y DVB-T2 and z DVB-C adapters on this box (0 means auto-detect)
	* eg: -a 1:2:3  
	- it will report 1 dvb-s2 device, 2 dvb-t2 devices and 3 dvb-c devices 

* -A --disable-ssdp disable SSDP announcement
 
* -b --buffers X:Y : set the app adapter buffer to X Bytes (default: 72192) and set the kernel DVB buffer to Y Bytes (default: 5775360) - both multiple of 188
	* eg: -b 18800:18988

* -B X : set the app socket write buffer to X KB. 
	* eg: -B 10000 - to set the socket buffer to 10MB

* -d --diseqc ADAPTER1:COMMITTED1-UNCOMMITTED1[,ADAPTER2:COMMITTED2-UNCOMMITTED2[,...]
	* The first argument is the adapter number, second is the number of committed packets to send to a Diseqc 1.0 switch, third the number of uncommitted commands to sent to a Diseqc 1.1 switch
	The higher number between the committed and uncommitted will be sent first.
	* eg: -d 0:1-0  (which is the default for each adapter).
	- note: * as adapter means apply to all adapters
	- note: * before committed number enables fast-switch (only voltage/tone)
	- note: @ before committed number sets 'Any Device' diseqc address (0x00)
	- note: . before committed number sets 'LNB' diseqc address (0x11)

* -q --diseqc-timing ADAPTER1:BEFORE_CMD1-AFTER_CMD1-AFTER_REPEATED_CMD1-AFTER_SWITCH1-AFTER_BURST1-AFTER_TONE1[,...]
	* All timing values are in ms, default adapter values are: 15-54-15-15-15-0
	- note: * as adapter means apply to all adapters

* -D --device-id DVC_ID: specify the device id (in case there are multiple SAT>IP servers in the network)
 	* eg: -D 4 

* -0 --diseqc-multi ADAPTER1:DISEQC_POSITION[,...]
	* Send diseqc to selected position before other position is set.
	- note: * as adapter means apply to all adapters

* -E Allows encrypted stream to be sent to the client even if the decrypting is unsuccessful
 
* -Y --delsys ADAPTER1:DELIVERY_SYSTEM1[,ADAPTER2:DELIVERY_SYSTEM2[,..]] - specify the delivery system of the adapters (0 is the first adapter)	
	* eg: --delsys 0:dvbt,1:dvbs
	- specifies adapter 0 as a DVBT device, adapter 1 as DVB-S, which overrides the system detection of the adapter

* --dmx-source ADAPTER1:FRONTENDX - specifies the frontend number specified as argument for DMX_SET_SOURCE 
	* eg: --dmx-source 0:1 - enables DMX_SET_SOURCE ioctl call with parameter 1 for adapter 0

* -e --enable-adapters list_of_enabled adapters: enable only specified adapters
	* eg: -e 0-2,5,7 (no spaces between parameters)
	- keep in mind that the first adapters are the local ones starting with 0 after that are the satip adapters 
	if you have 3 local dvb cards 0-2 will be the local adapters, 3,4, ... will be the satip servers specified with argument -s

* -f foreground, otherwise run in background

* -F --logfile log_file, output the debug/log information to  log_file when running in background (option -f not used), default /tmp/minisatip.log

* -g use syslog instead stdout for logging, multiple -g - print to stderr as well

* -H --threshold X:Y : set the write time threshold to X (UDP) / Y (TCP)  milliseconds. 
	* eg: -H 5:50 - set thresholds to 5ms (UDP) and 50ms (TCP)

* -i --priority prio: set the DVR thread priority to prio 

* -k Emulate pids=all when the hardware does not support it, on enigma boxes is enabled by default 

* -l specifies the modules comma separated that will have increased verbosity, 
	logging to stdout in foreground mode or in /tmp/minisatip.log when a daemon
	Possible modules: general,http,socketworks,stream,adapter,satipc,pmt,tables,dvbapi,lock,netceiver,ca,axe,socket,utils,dmx,ssdp,dvb
	* eg: -l http,pmt

* -v specifies the modules comma separated that will have increased debug level (more verbose than -l), 
	* eg: -v http,pmt

* -L --lnb specifies the adapter and LNB parameters (low, high and switch frequency)
	* eg: -L *:9750-10600-11700 - sets all the adapters to use Universal LNB parameters (default)
	* eg: -L *:10750-10750-10750 - sets the parameters for Sky NZ LNB using 10750 Mhz
	* eg: -L 0:10750-10750-10750,1:9750-10600-11700 - adapter 0 has a SKY NZ LNB, adapter 1 has an Universal LNB

* -m xx: simulate xx as local mac address, generates UUID based on mac
	* eg: -m 001122334455 

* -M multiplies the strength and snr of the DVB adapter with the specified values
	* If the snr or the strength multipliers are set to 0, minisatip will override the value received from the adapter and will report always full signal 100% 
	* eg: -M 4-6:1.2-1.3 - multiplies the strength with 1.2 and the snr with 1.3 for adapter 4, 5 and 6
	* eg: -M *:1.5-1.6 - multiplies the strength with 1.5 and the snr with 1.6 for all adapters

* -N --disable-dvb disable DVB adapter detection
 
* -Z --adapter-timeout ADAPTER1,ADAPTER2-ADAPTER4[,..]:TIMEOUT - specify the timeout for the adapters (0 enabled infinite timeout)	
	eg: --adapter-timeout 1-2:30
	- sets the timeouts for adapter 1 and 2 to 30 seconds 
	--adapter-timeout *:0
	- turns off power management for all adapters (recommended instead of --adapter-timeout 0-32:0) 
	- required for some Unicable LNBs 

* -o --dvbapi host:port - specify the hostname and port for the dvbapi server (oscam). Port 9000 is set by default (if not specified) 
	* eg: -o 192.168.9.9:9000 
	192.168.9.9 is the host where oscam is running and 9000 is the port configured in dvbapi section in oscam.conf.
	* eg: -o /tmp/camd.socket 
	/tmp/camd.socket is the local socket that can be used 

* -p url: specify playlist url using X_SATIPM3U header 
	* eg: -p http://192.168.2.3:8080/playlist
	- this will add X_SATIPM3U tag into the satip description xml

* -P  port: use port number to listen for UDP socket in the RTP communication. port + 1000 will be used to listen by the sat>ip client (option -s)
 	* eg: -P 5500 (default): will use for the sat>ip server 5500 + 2*A and 5500 + 2*A + 1, where A is the adapter number. 
				6500 + 2*A and 6500 + 2*A + 1 - will be used by the sat>ip client
 
* -r --remote-rtp  remote_rtp_host: send the rtp stream to remote_rtp_host instead of the ip the connection comes from
 	* eg: -r 192.168.7.9
 
* -R --document-root directory: document root for the minisatip web page and images

* -s --satip-servers [~][*][DELSYS:][FE_ID@][source_ip/]host[:port] - specify the remote satip host and port with delivery system DELSYS, it is possible to use multiple -s 
	* ~ When using this symbol at start the `pids=all` call is replaced with `pids=0-20`
	* - Use TCP if -O is not specified and UDP if -O is specified
	DELSYS - can be one of: dvbs, dvbs2, dvbt, dvbt2, dvbc, dvbc2, isdbt, atsc, dvbcb ( - DVBC_ANNEX_B ) [default: dvbs2]
	host - the server of the satip server
	port - rtsp port for the satip server [default: 554]
	FE_ID - will be determined automatically
	eg: -s 192.168.1.2 -s dvbt:192.168.1.3:554 -s dvbc:192.168.1.4
	- specifies 1 dvbs2 (and dvbs)satip server with address 192.168.1.2:554
	- specifies 1 dvbt satip server  with address 192.168.1.3:554
	- specifies 1 dvbc satip server  with address 192.168.1.4:554
	eg: -s dvbt:2@192.168.1.100/192.168.1.2:555
	- specifies 1 dvbt adapter to satip server with address 192.168.1.2, port 555. The client will use fe=2 (indicating adapter 2 on the server) and will connect from IP address 192.168.1.100
	address 192.168.1.100 needs to be assigned to an interface on the server running minisatip.
	This feature is useful for AVM FRITZ!WLAN Repeater
	
*  --satip-xml <URL> Use the xml retrieved from a satip server to configure satip adapters 
	eg: --satip-xml http://localhost:8080/desc.xml 

* -O --satip-tcp Use RTSP over TCP instead of UDP for data transport 
 * -S --slave ADAPTER1,ADAPTER2-ADAPTER4[,..]:MASTER - specify slave adapters	
	* Allows specifying bonded adapters (multiple adapters connected with a splitter to the same LNB)
	* This feature is used by FBC receivers and AXE to specify the source input of the adapter
	Only one adapter needs to be master all others needs to have this parameter specified
	eg: -S 1-2:0
	- specifies adapter 1 to 2 as slave, in this case adapter 0 is the master that controls the LNB
	- the slave adapter will not control the LNB polarity or band, but it will just change the internal frequency to tune to a different transponder
	- if there is no adapter using this link, the slave will use master adapters frontend to change the LNB polarity and band
	eg: -S 2-7:0 (default for DVB-S2 FBC), adapter 0 and 1 are masters, 2-7 slave and linked to input 0 (A)
	- all 8 adapters use physical input A to tune
	eg: -S 2-4:0,5-7:1
	- adapters 2,3,4 use physical input A to tune, while 1,5,6,7 uses input B to tune, adapter 0 and 1 are masters

* -t --cleanpsi clean the PSI from all CA information, the client will see the channel as clear if decrypted successfully

* -T --threads: enables/disable multiple threads (reduces memory consumption) (default: ENABLED)

* -u --unicable unicable_string: defines the unicable adapters (A) and their slot (S), frequency (F) and optionally the PIN for the switch:
	* The format is: A1:S1-F1[-PIN][,A2:S2-F2[-PIN][,...]]
	eg: 2:0-1284[-1111]
	* When * character is used before frequency, force 13V only for setup

* -j --jess jess_string - same format as -u 

* -U --sources sources_for_adapters: limit the adapters to specific sources/positions
	* eg: -U 0-2:*:3:2,6,8 (no spaces between parameters)
	- In this example: for SRC=1 only 0,1,2; for SRC=2 all: for SRC=3 only 3; and for SRC=4 the 2,6,8 adapters are used.
	- For each position (separated by : ) you need to declare all the adapters that use this position with no exception.
	- The special char * indicates all adapters for this position.
	- The number of sources range from 1 to 64; but the list can include less than 64 (in this case all are enabled for undefined sources).
	- By default or in case of errors all adapters have enabled all positions.

* -w --http-host http_server[:port]: specify the host and the port (if not 80) where the xml file can be downloaded from [default: default_local_ip_address:8080] 
	* eg: -w 192.168.1.1:8080 

* -x --http-port port: port for listening on http [default: 8080]
	* eg: -x 9090 

* -X --xml PATH: the path to the xml that is provided as part of the satip protocol	
	* by default desc.xml is provided by minisatip without needing an additional file, 
	however satip.xml is included if it needs to be customized

* -y --rtsp-port rtsp_port: port for listening for rtsp requests [default: 554]
	* eg: -y 5544 
	- changing this to a port > 1024 removes the requirement for minisatip to run as root
* -1 --demux-dev [1|2|3]: the protocol used to get the data from demux
	* 0 - use dvrX device 
	* 1 - use demuxX device 
	* 2 - use dvrX device and additionally capture PSI data from demuxX device 
	* 3 - use demuxX device and additionally capture PSI data from demuxX device 

 * -3 --ca-pin mapping_string: set the pin for CIs
	* The format is: ADAPTER1:PIN,ADAPTER2-ADAPTER4:PIN
	* eg: 0:1234,2-3:4567 

* -C --ci mapping_string: disable CI+ mode for specified adapters
	* The format is: ADAPTER1:PIN,ADAPTER2-ADAPTER4
			* eg : 0,2-3

How to compile:
------

- ./configure

Configures minisatip for the current system (use ./configure --help for options)

To cross compile, use something like (static compilation), considering that mips-openwrt-linux-musl-gcc is the gcc executable for that platform:

- ./configure --host=mips-openwrt-linux-musl --enable-static

To compiles the application

- make

To add custom compilation flags you can use:

make EXTRA_CFLAGS=....

Examples:
-------
- In order to listen to a radio after minisatip is started open the following URL in your favorite media player:
	- on Hotbird 13E: "http://MINISATIP_HOST:8080/?msys=dvbs&freq=11623&pol=v&sr=27500&pids=0,10750,254"
	- Astra 19.2E: "http://MINISATIP_HOST:8080/?msys=dvbs&freq=12266&pol=h&sr=27500&pids=0,851"	

- Television FTA programs:
	- Astra 19.2E, Kika HD: "rtsp://MINISATIP_HOST:554/?src=1&freq=11347&pol=v&ro=0.35&msys=dvbs2&mtype=8psk&plts=on&sr=22000&fec=23&pids=0,17,18,6600,6610,6620,6630"

- msys can be one of: dvbs, dvbs2, dvbt, dvbt2, dvbc, dvbc2, atsc, isdbt, dvbcb ( - DVBC_ANNEX_B )

