import socket
import time
import struct
import pickle

REMOTE_IP = '192.168.1.254'
REMOTE_PORT = 8888
UDP_IP = '192.168.1.222'
UDP_PORT = 8742
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) 

sock.bind((UDP_IP, UDP_PORT))
print('Test print')
datacount = 0;
l = [];
maxDataCount = 1000;
while datacount < maxDataCount:
	data, addr = sock.recvfrom(1024)
	packet = (struct.unpack('=?Lf?',data))
	l.append(packet)
	print(packet)		
	time.sleep(.01)
	datacount = datacount+1

fileName = open('TestPickleFile', 'wb')
pickle.dump(l, fileName)
fileName.close()

fileName = open('TestPickleFile', 'rb')
x= pickle.load(fileName)
print(x)