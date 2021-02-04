
Version 1.0.X
- Added ddci.conf to configure the channel assignments for independent CAMs
- Added support for TBS 6900
- For axe, use -S instead of -7 or --dmx-source 
- Remove support for oscam version < 11553
- Add multiple_pmt (-c option) to enable packing multiple PMTs in the same CAPMT that is sent to the cam. If the CAM supports just 1 or 2 channels, enabling this option will allow decrypting more channels. Still in testing
- Emulate pids_all on enigma boxes (-k option): Enigma boxes (openatv, ... ) does not support pid 8192 in hardware. This option enables the client to request pids=all in the stream and then it adds indiviually all the pids present in the PMT.
- Fix the CA module to delete correctly the channel
- Send dummy EPG to the CAM to prevent it from requesting a PIN
- Allow configuring dvbapi_offset for oscam
- Add support for IPv6 (-4 option)
- Fix compatibility with VU DVB-S2X tuner

