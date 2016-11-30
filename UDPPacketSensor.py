import socket
import time
import struct

REMOTE_IP = '192.168.1.254'
REMOTE_PORT = 8888
UDP_IP = '192.168.1.222'
UDP_PORT = 8742
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) 

sock.bind((UDP_IP, UDP_PORT))
print('Test print')
while True:
	data, addr = sock.recvfrom(1024)
	print(len(data))
	print(struct.unpack('=?Lf?',data))		
	time.sleep(.01)