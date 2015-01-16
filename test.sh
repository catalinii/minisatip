(echo "PLAY rtsp://192.168.128.5/?freq=623.25&msys=dvbc&mtype=128qam&sr=6900&specinv=0&pids=0,16,50,201,301 RTSP/1.0";echo "Transport: RTP/AVP;unicast;client_port=38076-38077";echo "CSeq: 1";printf '\r\n';printf '\r\n';sleep 10) | nc localhost 554


