 #! /usr/bin/python -u
import os,sys,select
import array
import re,traceback
import collections
import socket
import time
import threading

stime = 30
sockets = {}

rtsp_port = 5544
rtsp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
rtsp.bind(('',rtsp_port))
rtsp.listen(5)
inputs = [rtsp]
outputs = []
enc_odd = "D27F3EBF673549157F4772356B8347C04F705667D5B4D8BF7DDE6931DAE3E417242EAE057BABEE902EB878D617C1491EE591DF7907F3D3148230440A7909D91B0D3323B22BE4FF0F45F584D148EF2C47C04740D6FF892FFA552D8BB7394ED1C48853526A8A8B4C2D5FEBE07074747B236DF96B55DCE021C93B69EA16285E603057B7AE4807594EE85090E35D77EF47E143EEAB61089C5E9E47C5DB400BFF3906683430DB55BC064FC87435DCFBAF0FB9744BDCDD0E02D414"
enc_even = "0CBBCFD795D811274C4F270C49B49ED42C4F515AF20E84D07FBB32E3C51A53BC2D5C220A3729E5948060E137711C94C3C943CB023D2BC02A9FAF8288ADAFAA9C5D6BF682E58DFFB435E676958B4A02A6ED475E411E14D8695D8EF27905CEE02295B7A7B0B36824513B28F947E4832499A32326CCFF6ECE8D1567A2FEE3190656FDAF040FA64A0869B613AA9786B9076A4D3BD18CE77D7C36DBE181F11D288988F1B540756037A51BCA8E87511E801507F0859A1CA0D4B3C7"
enc_odd = enc_odd.decode("hex")
enc_even = enc_even.decode("hex")
x = array.array('c', ' '*1328) 
odd = 0
def ts_read(s):
	f = s["file"]
	d = f.read(7*188)
	if len(d)==0:
		print "Seeking to the begining of the file"
		f.seek(0,0)
	data = "123456789012"+d
	return data

# csa odd key: 22 22 22 66 22 22 22 66
# csa even key: 99 99 99 cb 99 99 99 cb	
# csa odd dec: 11 11 11 11 11 11 11 11 
# csa even dec: 88 88 88 88 88 88 88 88
def generate_packet(p, i, pid, odd, cc):
	
	p[i+0] = '\x47'
	p[i+1] = chr(0x40 | (pid >> 8))	
	p[i+2] = chr(pid & 0xFF)
	p[i+3] = chr(0x80 + odd*0x40 + cc)
	cc = (cc + 1 ) & 0xF
	enc = enc_even
	if odd:
		enc = enc_odd
	for j in range(0,184):
		p[i + 4 + j] = enc[j]
	return cc

def generate_7(s):
	i = 0
	pids = list(s["pids"])
	if not s.has_key("pos"):
		s["pos"] = 0
	pos = s["pos"]
	x[0] = '\x81'
	x[1] = '\x21'
	for j in range(0,7):
#		print "generate packet for pid %d, pos %d out of %s " % (pids[pos], pos, pids)
		if pos < len(pids):
			s["cc"] = generate_packet(x, 12 + j*188, pids[pos], odd, s.get("cc",0))
		pos = pos + 1
		if pos >= len(pids):
			pos = 0
		s["pos"] = pos
	return x


ratio = 100
current = 0;
total = 0
while 1:
	readable, writable, exceptional = select.select(inputs, outputs, inputs, 0.0005)
	
	if len(readable) == 0:
		for i in sockets:
			if sockets[i].has_key("file"):
				if current > ratio:
					current = 0
				if current == 0:
					data = ts_read(sockets[i])
				else:
					data = generate_7(sockets[i])
				current = current + 1
				total = total + 1
				if total % 1600 == 0:
					print "parity change"
					odd = 1 - odd
				try:
					sockets[i]["sock"].send(data, 0)
				except:
					print "Failure sending rtp data to %d : %s - sockets keys: %s" % (i,str(sockets[i]["sock"]),sockets)
					print traceback.format_exc()
					sys.exit(1)
				
	else:
		for s in readable:
			if s is rtsp:
				connection, client_address = s.accept()
				print 'new connection from', client_address
				inputs.append(connection)
				sockets[connection.fileno()]={"rtsp":s}
			else:
				try:
					data = s.recv(2048)
				except:
					print "Failed reading from %s" % str(s)
					exceptional.append(s)
				sock = None
				dest = None
				port = 0
				if data.find("client_port=")!=-1:
					pos=data.find("client_port=")
					end = data.find("-", pos)
					port = int(data[pos + 12:end])
					dest = s.getpeername()[0]
					print "Transport destination %s:%d" % (dest,port)
					sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
					sock.connect((dest,port))
					sockets[connection.fileno()]["sock"] = sock
					print "Started RTP socket %d: %s" % (sock.fileno(),str(sock))
				
				transport=""
				if(sock):
					lp = sock.getsockname()[1]
					transport = "Transport: RTP/AVP;unicast;client_port=%d-%d;server_port=%d-%d\r\n" % (port, port+1, lp, lp+1)
					print transport
				final = "RTSP/1.0 200 OK\r\n"+transport+"Session: %d12345678;timeout=30\r\ncom.ses.streamID: %d\r\nRTP-Info: url=rtsp://192.168.101.2:554/;seq=0\r\n\r\n" % (s.fileno(),s.fileno())
				
				if data.find("freq=") != -1:
					start = data.find("freq=")
					end = data.find("&", start)
					fn = "/usr/src/minisatip/"+data[start:end]+".ts"
					print "Opening file %s" % fn
					try:
						f = open(fn)
						pp = f.read()
						i = 1
						myset = set()
						while i < len(pp):
							pid = (ord(pp[i]) & 0x1F)*256 + ord(pp[i+1])							
							myset.add(pid)
							i = i + 188
						f.close()
						print "Removed pid lists: %s, total len: %d", (myset, len(pp))
						sockets[connection.fileno()]["diff"] = myset
						f = open(fn)					
						if sockets[connection.fileno()].has_key("file"):
							sockets[connection.fileno()]["file"].close()
						sockets[connection.fileno()]["file"] = f
						sockets[connection.fileno()]["type"] = data.find("dvbs2") + data.find("8psk") + 2
						sockets[connection.fileno()]["pids"] = set()
					except:
						print "Error opening ", fn
						print traceback.format_exc()
				
				for cur in ["&addpids=","&delpids=","&pids=","?addpids=","?delpids=","?pids="]:
					if data.find(cur) != -1:
						
						myset = sockets[connection.fileno()]["pids"]
						newset = set()
						pos = data.find(cur)+len(cur)
						pids = data[pos:data.find(" ",pos)]
						for pid in pids.split(','):
							newset.add(int(pid))
						if cur.find( "pids=") == 1:
							print "resetting the set ", myset
							myset = newset
						if cur.find( "addpids=") != -1:
							myset = myset.union(newset)
						if cur.find( "delpids=") != -1:
							myset = myset.difference(newset)
						myset = myset.difference(sockets[connection.fileno()]["diff"])
						print "%s ->|%s| => %s"  % (cur, newset, myset)
						
						sockets[connection.fileno()]["pids"] = myset
					
					
#				print "Read %d data:" % (len(data))
#				print data
				if len(data) == 0:
					print "adding to exceptional %d" % s.fileno()
					exceptional.append(s)
				else:
#					print final
					try:
						s.send(final)
					except:
						print "Failure writing to %s" % str(s)
						exceptional.append(s)

	if len(exceptional):
		for s in exceptional:
			try:
				print 'handling exceptional condition for', s.getpeername()
			except:
				print "Closing %s" % str(s)
			try:
				inputs.remove(s)
				if s in outputs:
					outputs.remove(s)
				if sockets[s.fileno()].has_key("sock"):
					sockets[s.fileno()]["sock"].close()

				if sockets[s.fileno()].has_key("file"):
					sockets[s.fileno()]["file"].close()
				
				del sockets[s.fileno()]
				s.close()
			except:
				print "Error closing %s" % str(s)
				pass
			continue



