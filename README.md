<a href="https://scan.coverity.com/projects/minisatip2">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/23090/badge.svg"/>
</a>

# Welcome to Minisatip

Minisatip is a multi-threaded satip server version 1.2 that runs under Linux and it was tested with DVB-S, DVB-S2, DVB-T, DVB-T2, DVB-C, DVB-C2, ATSC and ISDB-T cards. More details about supported hardware: https://github.com/catalinii/minisatip/blob/master/Supported_Hardware.md

The protocol specification can be found at: 
http://satip.info/sites/satip/files/resource/satip_specification_version_1_2_2.pdf
It is very lightweight (designed for embedded systems with memory and processing constrains), does not need any additional libraries for basic functionality and can be used by existing satip clients like: Tvheadend, DVBViewer, vdr, VideoLAN or Android/iOS applications. Minisatip can act as a satip client as well in order to connect to satip servers from different networks.

The application is designed to stream the requested data to multiple clients (even with one dvb card) in the same time while opening different pids.

It is tested on x86_64, x86, ARM and MIPS platforms and it requires DVBAPI 5. Supported protocols are RTSP (both tcp and udp), HTTP (port 8080) and SSDP (as specified in the SAT>IP documentation). On top of that, it supports dvbapi protocol implemented by oscam (requires dvbcsa library) to decrypt channels using an official subscription and support dvbca protocol (requires libssl-dev library) for dvb cards with CA hardware. In order to enable/disable features, please edit the Makefile. 

The application shows also a status page by default at the address: http://IP:8080 

## Contact

Please use github for any question: https://github.com/catalinii/minisatip/discussions

In order to speed up the investigation of an issue, please provide the full log and a link to the application that is not working.

If you like minisatip and you want to support the development of the project please make a donation: 
https://paypal.me/minisatip

## Usage:

(Message automatically generated from `minisatip --help`)
```
minisatip version v2.0.19~7def250, compiled on Aug  9 2025 11:02:04, with s2api version: 050C

	./minisatip [-[fgtzE]] [-a x:y:z] [-b X:Y] [-B X] [-H X:Y] [-d A:C-U ] [-D device_id] [-e X-Y,Z] [-i prio] 
	[-[uj] A1:S1-F1[-PIN]] [-P port] [-l module1[,module2]] [-v module1[,module2]] 
	[-o [~]oscam_host:dvbapi_port[,offset] [-p public_host] [-r remote_rtp_host] [-R document_root] [-s [*][DELSYS:][FE_ID@][source_ip/]host[:port] 
	[-u A1:S1-F1[-PIN]] [-L A1:low-high-switch] [-w http_server[:port]] 
 	[-x http_port] [-X xml_path] [-y rtsp_port] [-I name_service]

Help
-------

* -4 : Force TCP sockets to use IPv6

* -a --adapters x:y:z simulate x DVB-S2, y DVB-T2 and z DVB-C adapters on this box (0 means auto-detect)
	* eg: -a 1:2:3  
	- it will report 1 dvb-s2 device, 2 dvb-t2 devices and 3 dvb-c devices 

* -G --disable-ssdp disable SSDP announcement
 
* -b --buffer X:Y : set the app adapter buffer to X Bytes (default: 376000) and set the kernel DVB buffer to Y Bytes (default: 5775360) - both multiple of 188
	* eg: -b 18800:18988

* -B X : set the app socket write buffer to X MB. 
	* eg: -B 10 - to set the socket buffer to 10MB (default value)

* --client-send-buffer X : set the socket write buffer for client connections to X KB. 
	- The default value is 0, corresponding to use the kernel default value
	* eg: --satip-receive-buffer  100 - to set the socket buffer to 100KB
	* eg: --satip-receive-buffer 1024 - to set the socket buffer to 1MB

* -z --cache-dir dir : set the app cache directory to dir. The directory will be created if it doesn't exist. 
	* defaults to /var/cache/minisatip (/var/lib/minisatip on Enigma) 

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
        Duplicating it (-E -E) all undecrypted packets are send as stuffing TS packets. Usefull for regular player clients.
	- note: when pids=all is emulated this pass NULLs too

* -Y --delsys ADAPTER1:DELIVERY_SYSTEM1[,ADAPTER2:DELIVERY_SYSTEM2[,..]] - specify the delivery system of the adapters (0 is the first adapter)	
	* eg: --delsys 0:dvbt,1:dvbs
	- specifies adapter 0 as a DVBT device, adapter 1 as DVB-S, which overrides the system detection of the adapter

* -e --enable-adapters list_of_enabled adapters: enable only specified adapters
	* eg: -e 0-2,5,7 (no spaces between parameters)
	- keep in mind that the first adapters are the local ones starting with 0 after that are the satip adapters 
	if you have 3 local dvb cards 0-2 will be the local adapters, 3,4, ... will be the satip servers specified with argument -s

* -f foreground: otherwise run in background

* -F --logfile log_file: output the debug/log information to  log_file when running in background (option -f not used), default /tmp/minisatip.log

* -g --syslog: use syslog instead stdout for logging, multiple -g - print to stderr as well

* -H --threshold X:Y : set the write time threshold to X (UDP) / Y (TCP)  milliseconds. 
	* eg: -H 5:50 - set thresholds to 5ms (UDP) and 50ms (TCP)

* -I --name-app specificies an alternative Service Name

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
	* eg: -L *:10750-0-0 - sets all adapters to use 10750 MHz Ku-band LNB parameters (e.g. Sky NZ)
	* eg: -L *:5150-0-0 - sets all adapters to use 5150 MHz C-band LNB parameters
	* eg: -L 0:10750-0-0,1:9750-10600-11700 - adapter 0 has a SKY NZ LNB, adapter 1 has an Universal LNB
	
	For backward-compatibility reasons, linear LNB parameters may also be specified as *:5150-5150-5150 instead of *:5150-0-0

* -M --multiplier: multiplies the strength and snr of the DVB adapter with the specified values
	* If the snr or the strength multipliers are set to 0, minisatip will override the value received from the adapter and will report always full signal 100% 
	* eg: -M 4-6:1.2-1.3 - multiplies the strength with 1.2 and the snr with 1.3 for adapter 4, 5 and 6
	* eg: -M *:1.5-1.6 - multiplies the strength with 1.5 and the snr with 1.6 for all adapters
	* [%] This symbol forces to read values as percentage
	* [#] This symbol forces to read values as decibels
	* eg: -M *:%1.0-#1.0 - not multiply the strength but force it as percentage and for the snr interpret it as decibels
	* Zero values will disable frontend polling, and this may solve CC errors with some hardware/drivers (eg: -M *:0-0)

* -N --disable-dvb disable DVB adapter detection
 
* -Z --adapter-timeout ADAPTER1,ADAPTER2-ADAPTER4[,..]:TIMEOUT - specify the timeout for the adapters (0 enabled infinite timeout)	
	eg: --adapter-timeout 1-2:30
	- sets the timeouts for adapter 1 and 2 to 30 seconds 
	--adapter-timeout *:0
	- turns off power management for all adapters (recommended instead of --adapter-timeout 0-32:0) 
	- required for some Unicable LNBs 

* -5 --disable-cat ADAPTER1,ADAPTER2-ADAPTER4
	* eg: -5 1-3,4 
	- disable passing the CAT to the DDCI device 1,2,3 and 4 

* -o --dvbapi [~]host:port,offset - specify the hostname and port for the dvbapi server (oscam). Port 9000 is set by default (if not specified) 
	* [~] This symbol at the beginning indicates that in all `pids=all` requests the filtering of unencrypted packets must be disabled (useful when not using -E).
	* eg: -o 192.168.9.9:9000 or -o 192.168.9.9:9999,2  with offset if multiple dvbapi clients use the same server
	192.168.9.9 is the host where oscam is running and 9000 is the port configured in dvbapi section in oscam.conf.
	* eg: -o /tmp/camd.socket 
	/tmp/camd.socket is the local socket that can be used 
* --send-all-ecm Pass all received ECM to the DVBAPI server. Warning: This option may overload your server. Use with caution. Not necessary for regular use.

* -p --playlist url: specify playlist url using X_SATIPM3U header 
	* eg: -p http://192.168.2.3:8080/playlist
	- this will add X_SATIPM3U tag into the satip description xml

* -P  port: use port number to listen for UDP socket in the RTP communication. port + 1000 will be used to listen by the sat>ip client (option -s)
 	* eg: -P 5500 (default): will use for the sat>ip server 5500 + 2*A and 5500 + 2*A + 1, where A is the adapter number. 
				6500 + 2*A and 6500 + 2*A + 1 - will be used by the sat>ip client
 
* -r --remote-rtp  remote_rtp_host: send the rtp stream to remote_rtp_host instead of the ip the connection comes from
 	* eg: -r 192.168.7.9
 
* -R --document-root directory: document root for the minisatip web page and images

* -s --satip-servers [~][*][DELSYS:][FE_ID@][source_ip/]host[:port] - specify the remote satip host and port with delivery system DELSYS, it is possible to use multiple -s 
	* [~] When using this symbol at start the `pids=all` call is replaced with `pids=0-20`
	* [*] Use TCP if -O is not specified and UDP if -O is specified
	* DELSYS - can be one of: dvbs, dvbs2, dvbt, dvbt2, dvbc, dvbc2, isdbt, atsc, dvbcb ( - DVBC_ANNEX_B ) [default: dvbs2]
	* host - the server of the satip server
	* port - rtsp port for the satip server [default: 554]
	* FE_ID - will be determined automatically
	* eg: -s 192.168.1.2 -s dvbt:192.168.1.3:554 -s dvbc:192.168.1.4
	- specifies 1 dvbs2 (and dvbs)satip server with address 192.168.1.2:554
	- specifies 1 dvbt satip server  with address 192.168.1.3:554
	- specifies 1 dvbc satip server  with address 192.168.1.4:554
	* eg: -s dvbt:2@192.168.1.100/192.168.1.2:555
	- specifies 1 dvbt adapter to satip server with address 192.168.1.2, port 555. The client will use fe=2 (indicating adapter 2 on the server) and will connect from IP address 192.168.1.100
	address 192.168.1.100 needs to be assigned to an interface on the server running minisatip.
	This feature is useful for AVM FRITZ!WLAN Repeater
	
* -6 --satip-xml <URL> Use the xml retrieved from a satip server to configure satip adapters 
	eg: --satip-xml http://localhost:8080/desc.xml 

* -O --satip-tcp Use RTSP over TCP instead of UDP for data transport 

* --satip-receive-buffer X : set the socket read buffer for connecting to SAT>IP servers to X KB. 
	- The default value is 0, corresponding to use the kernel default value
	* eg: --satip-receive-buffer  350 - to set the socket buffer to 350KB
	* eg: --satip-receive-buffer 1024 - to set the socket buffer to 1MB

* -S --slave ADAPTER1,ADAPTER2-ADAPTER4[,..]:MASTER - specify slave adapters	
	* Allows specifying bonded adapters (multiple adapters connected with a splitter to the same LNB)
	* This feature is used by FBC receivers to specify the source input of the adapter
	Only one adapter needs to be master all others needs to have this parameter specified
	eg: -S 1-2:0
	- specifies adapter 1 to 2 as slave, in this case adapter 0 is the master that controls the LNB
	- the slave adapter will not control the LNB polarity or band, but it will just change the internal frequency to tune to a different transponder
	- if there is no adapter using this link, the slave will use master adapters frontend to change the LNB polarity and band
	eg: -S 2-7:0 (default for DVB-S2 FBC), adapter 0 and 1 are masters, 2-7 slave and linked to input 0 (A)
	- all 8 adapters use physical input A to tune
	eg: -S 2-4:0,5-7:1
	- adapters 2,3,4 use physical input A to tune, while 1,5,6,7 uses input B to tune, adapter 0 and 1 are masters

* -u --unicable unicable_string: defines the unicable adapters (A) and their slot (S), frequency (F) and optionally the PIN for the switch:
	* The format is: A1:S1-F1[-PIN][,A2:S2-F2[-PIN][,...]]
	eg: 2:0-1284[-1111]
	* When * character is used before frequency, force 13V only for setup

* -j --jess jess_string - same format as -u 

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
* -V --bind address: address for listening (RTSP + SSDP) 
* -U --bind-http address: address for listening (HTTP)
* -J --bind-dev device: device name for binding (all services)
        * beware that only works with 1 device. loopback may not work!

* -A --virtual-diseqc mapping_string: absolute source mapping for virtual diseqc mode
	* The format is: SRC1[-END1]:AD1:DISEQC1[,SRC2:INP2:DISEQC2]
       * SRC: satellite position requested by clients (src argument for SAT>IP request minus 1, range: 0-63) (MAX_ADAPTERS 64)
       * END: end of the range for the position requested by clients (end argument for SAT>IP request minus 1, range: 0-63) (MAX_ADAPTERS 64)
       * ADAP: internal adapter (see -a and -e) (range: 0-63) (MAX_SOURCES 64)
       * DISEQC: target diseqc position of the adapter (range: 0-15) (or more if your adapter supports it)
	   * eg: 13E,19.2E on inputs 0&1 and 23.5E,28.2E on inputs 2&3:
		-A 0:0:0,0:1:0,1:0:0,1:1:1,2:2:0,2:3:0,3:2:1,3:2:2
	   * eg: 13E,19.2E on adapter 0 and 23.5E,28.2E on adapter 2&3 (source 0,1):
		-A 0-1:0:0,2-3:1:0
	* Note: Option -A is not compatible with clients requesting fe= (tvheadend)

* -9 --disable-pmt-scan: Disables scanning PMTs and only reads the PMTs that are requested by the client
	* Provides more reliable decrypting for channels included in multiple providers

* -3 --ca-pin mapping_string: set the pin for CIs
	* The format is: ADAPTER1:PIN,ADAPTER2-ADAPTER4:PIN
	* eg: 0:1234,2-3:4567 

* -C --ci mapping_string: disable CI+ mode for specified adapters
	* The format is: ADAPTER1:PIN,ADAPTER2-ADAPTER4
			* eg : 0,2-3

* -c --ca-channels adapter_list:maximum_number_of_channels_supported: Specify the number of channels supported by the CAM and override the CAIDs
	* The format is: ADAPTER1:[*]MAX_CHANNELS[-CAID1[-CAID2]...,ADAPTER2:MAX_CHANNELS[-CAID3[-CAID4]...] 
		 * before the MAX_CHANNELS enable 2 PMTs inside of the same CAPMT to double the number of decrypted channels
			* eg : 0:*1-100
The DDCI adapters 0 will support maximum of 1 CAPMT (2 channels because of *) and will use CAID1. If CAID is not specified it will use CAMs CAIDs
Official CAMs support 1 or 2 channels, with this option this is extended to 2 or 4
By default every CAM supports 1 channels

```

## How to compile:

```bash
./configure
```

Configures minisatip for the current system (use `./configure --help` for options)

To cross compile, use something like (static compilation), considering that `mips-openwrt-linux-musl-gcc` is the gcc executable for that platform:

```bash
./configure --host=mips-openwrt-linux-musl --enable-static
```

To compile the application:

```bash
make
```

To add custom compilation flags you can use for example:

```bash
make EXTRA_CFLAGS="-DNEEDS_SENDMMSG_SHIM"
```

The above command is useful if you're getting errors like `sendmmsg(): errno 38: Function not implemented` (usually only concerns really old systems).

## Contributing


Ensure clang-format and pre-commit are installed
```bash
pip3 install pre-commit
pre-commit install
apt-get install clang-format
```


## Building a Debian package:

```bash
dpkg-buildpackage -b -us -uc
```

## Examples:

- In order to listen to a radio after minisatip is started open the following URL in your favorite media player:
	- on Hotbird 13E: `http://MINISATIP_HOST:8080/?msys=dvbs&freq=11623&pol=v&sr=27500&pids=0,10750,254`
	- Astra 19.2E: `http://MINISATIP_HOST:8080/?msys=dvbs&freq=12266&pol=h&sr=27500&pids=0,851`

- Television FTA programs:
	- Astra 19.2E, Kika HD: `http://MINISATIP_HOST:554/?src=1&freq=11347&pol=v&ro=0.35&msys=dvbs2&mtype=8psk&plts=on&sr=22000&fec=23&pids=0,17,18,6600,6610,6620,6630`

- msys can be one of: dvbs, dvbs2, dvbt, dvbt2, dvbc, dvbc2, atsc, isdbt, dvbcb ( - DVBC_ANNEX_B )
