#! /usr/bin/python -u
import os,sys
import array
import re,traceback
import collections
import socket
import time
import threading

stime = 30
map = {}
repl = {}

mymap={}

def print_map(m):
	for i in m:
		if(len(m[i]) < 10):
			print "PID %d -> %s" % (i,m[i])

def detect_rtp_errors(key):
	port = map[key]["port"]
	sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	rtcp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	total_len = 0
	rtcp_len = 0
	print "Listening on %d for key %s" % (port, key)
	try:
		sock.bind(('', port))
		rtcp.bind(('', port+1))
	except:
		print "Binding on port %d failed for sock %s" % (port, key)
		sock.close()
		rtcp.close()
		return 
	sock.setblocking(0)
	sock.settimeout(2)
	rtcp.setblocking(0)
	last_cnt = 0
	cnt = 0
	diff = 0
	data = array.array('c', ' '*1500) 
	while (map.has_key(key)):
		try:
			
			(nb, ad) = sock.recvfrom_into(data, 1500)
			
			total_len = total_len + 1;
			if ord(data[0])==0x80 and ord(data[1])==0x21:
				cnt = ord(data[2])*256 + ord(data[3])
				last_cnt=last_cnt+1
				if last_cnt==65536:
					last_cnt = 0
					print "Resetting sequence number for port %d" % port
				if last_cnt != cnt:
					diff = diff + abs (last_cnt - cnt)
					print "Missing rtp packet connection %s expected cnt %x, actual cnt %x, diff %d" % (key, last_cnt, cnt, diff)
				if cnt % 0x1000 == 0:
					print "got data %x -> %x" % (last_cnt, cnt)
					
				last_cnt = cnt
				
				idx = 12
				while idx < nb:
					pid = (ord(data[idx+1])&0x1F )*256 + ord(data[idx+2])
					for k in range(4,184):
						fb = hex(ord(data[idx+k]))
						if(mymap.has_key(pid)):
							if mymap[pid].has_key(fb):
								mymap[pid][fb] = mymap[pid][fb] +1 
							else:
								mymap[pid][fb] = 1
						else :
							mymap[pid]={fb:1}
					idx = idx+188
				
			else:
				print "nothing %d %d" % (ord(data[0]), ord(data[1]))
		except:
			print traceback.format_exc()
			print "timeout on port %d" % port
			pass
		try:
			rtcp.recvfrom_into(data, 1500)
#			print "Read rtcp %x %x %x %x %x %x" % (ord(data[0]), ord(data[1]), ord(data[24]), ord(data[25]), ord(data[26]), ord(data[27]))
			if ord(data[0])==0x80 and ord(data[1])==0xC8:
				rtcp_len = ord(data[20])*65536*256 + ord(data[21])*65536 + ord(data[22])*256 + ord(data[23])
				print "rtcp len %d total_len %d connection %s" % (rtcp_len, total_len, key)
		except:
#			print traceback.format_exc()
			pass

	print "Closing detect_rtp_errors thread for port %d -> rtcp len %d total_len %d" % (port, rtcp_len, total_len)
	print_map(mymap)
	sock.close()
	rtcp.close()
	
			


if len(sys.argv) != 3:
	print "Syntax is replay.py MINISAT_IP LOG_FILE"
	sys.exit(1)

log = ""	
fn = sys.argv[2]
dest = sys.argv[1]
try:
	log = open(fn).read()
except: 
	print "Could not open file "+fn+", exiting.."
	print traceback.format_exc()
	sys.exit(1)
	
	
	
	
	
for s in  re.findall(r"(read RTSP \(from handle ([0-9]+).*?:\n(.*?)\n\[[0-9]{2}\/[0-9]{2})|(select_and_execute.*?on socket ([0-9]+))|(reply -> ([0-9]+).*?\n\[)",log,flags=re.I | re.S):
	sockid = ""
	if len(s[1])>0:
		if not map.has_key(s[1]):
				map[s[1]]={}
				map[s[1]]["sock"] = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
				map[s[1]]["sock"].connect((dest, 554))
		str = s[2].replace("\n","\r\n") + "\r\n"
		for k in repl:
			str = str.replace(k, repl[k])
		print "Sending to the server (connection %s):\n%s" % (s[1],str)
		if(str.startswith("OPTIONS") and map[s[1]].has_key("port")):
			time.sleep(30)
		if str.startswith("TEARDOWN"):
			time.sleep(stime)
			
		if(str.find("Transport:") > 0):
			icp = str.find("client_port=")
			icp = icp + len("client_port=")
			map[s[1]]["port"] = int (str[icp:str.find("-", icp)])
			print "Detected transport port %d" % map[s[1]]["port"]
			t = threading.Thread(target=detect_rtp_errors, args=(s[1],))
			t.start()
			map[s[1]]["th"] = t
		map[s[1]]["sock"].sendall(str)
		data = map[s[1]]["sock"].recv(4096)
		print "received from the server:\n%s" % data
		for l in re.findall("Session:[ ]*([0-9]+)", data):
			print "Identified Session ID: "+ l
			map[s[1]]["sess"] = l
		
		if str.startswith("TEARDOWN"):
			sockid = s[1]
	
	if sockid=="":
		sockid = s[4]
	if len(sockid)>0:
		if map.has_key(sockid):
			print "Closing the socket %s after %d s  s[4] %s" % (sockid,stime,s[4])
			sock = map[sockid]["sock"]
			th = map[sockid]["th"]
			if len(s[4])>=2:
				time.sleep(stime)
			del map[sockid]
			sock.close()
			time.sleep(3)
			th.join()
			
			
	if len(s[6])>0:
		for l in re.findall("Session:[ ]*([0-9]+)", s[5]):
			if not repl.has_key(l):
				print "Adding replace string from Session ID "+ map[s[6]]["sess"]+" to Session ID "+l
				repl[l]= map[s[6]]["sess"]
		
		

print "Finished execution: waiting %d s.... " % stime	
			
time.sleep(stime)
print_map(mymap)
list = [x for x in map]
for i in list:
			map[i]["sock"].close()
			t = map[i]["th"]
			del map[i]
			t.join()
	
