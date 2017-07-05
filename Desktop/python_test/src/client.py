# Echo client program
import socket
import errno
import random
from time import sleep
# from printer import printbuffer

# GLOBAL variables 
ID_packet_counter = 0
counter_data = 0
# Constants
SIZE_DATA   = 50   
SIZE_PACKET = 4 * (2 + 1 + SIZE_DATA)

# This function will append data to a buffer
# it will append a 2 words header, a one word ID 
# and "size" words of data
# 		1 word = 4 bytes
def create_buffer(buff, size):

	for i in range (0, 8):
		buff.append(0xFF)

	global ID_packet_counter
	global counter_data

	dissemble(buff, ID_packet_counter)

	for i in range (1, size):
		# data = random.randint(0, 0xFFFFFFFF)
		data = int(counter_data)
		dissemble(buff, data)
		counter_data = counter_data + 1

	ID_packet_counter = ID_packet_counter + 1


# This function will break a word into 4 bytes
# and append those values to a buffer 
def dissemble(buff, val):
	buff.append( val >> 24)
	buff.append((val >> 16) & 0x000000FF)
	buff.append((val >> 8)  & 0x000000FF)
	buff.append( val        & 0x000000FF)



# This fucntion create a Client that will connect to a given IP port 
# or return if it does not succeed. I a connexion happens, it will 
# send a "1 word counter" through TCP/IP, continuously
def send_non_packed_data():
	HOST = "127.0.0.1"    # The remote host
	PORT = 50007          # The same port as used by the server
	cnt  = 0
	sent = 1
	
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect((HOST, PORT))
	s.setblocking(0)

	while True:

		if sent == 1:
			buff = bytearray()
			# r_s = s.sendall("\n\r")
			for x in range (0, 4):
				dissemble(buff, cnt)
				cnt = cnt + 1
			# printbuffer(buff)

		try: 
			sent = 0
			r_s  = s.sendall(buff)
			# sleep(0.25)

		except socket.error, e:
			r_s = e.args[0]

			if r_s == errno.EAGAIN or r_s == errno.EWOULDBLOCK:
				continue
			else:
				print(r_s)
				sleep(0.01)
				break

		else:
			sent = 1
			del buff

	print "break"
	s.close()

# This fucntion create a Client that will connect to a given IP port 
# or return if it does not succeed. I a connexion happens, it will
# send packeted data [{2 words header}{1 word ID}{size words data}]
# continuously. The {ID} is a 1 word counter (0 to 2^31 - 1) 
def send_packed_data():
	HOST = "127.0.0.1"    # The remote host
	PORT = 50007          # The same port as used by the server
	cnt  = 0
	sent = 1

	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect((HOST, PORT))
	s.setblocking(0)

	print ("size_packet = {}".format(SIZE_PACKET))
	
	while True:

		if sent == 1:
			buff = bytearray()
			create_buffer(buff, SIZE_DATA)

		try: 
			sent = 0
			r_s  = s.sendall(buff)
			# sleep(0.25)

		except socket.error, e:
			r_s = e.args[0]
			if r_s == errno.EAGAIN or r_s == errno.EWOULDBLOCK:
				continue
			else:
				print("error connexion #{}".format(r_s))
				sleep(0.01)
				break

		else:
			sent = 1
			del buff

	print "break main loop"
	
	s.close()

if __name__ == "__main__":
	send_non_packed_data()