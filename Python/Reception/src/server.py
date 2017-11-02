#	 .d8888b.  8888888888 8888888b.  888     888 8888888888 8888888b.  	
#	d88P  Y88b 888        888   Y88b 888     888 888        888   Y88b 	
#	Y88b.      888        888    888 888     888 888        888    888 	
#	 "Y888b.   8888888    888   d88P Y88b   d88P 8888888    888   d88P 	
#	    "Y88b. 888        8888888P"   Y88b d88P  888        8888888P"  	
#	      "888 888        888 T88b     Y88o88P   888        888 T88b   	
#	Y88b  d88P 888        888  T88b     Y888P    888        888  T88b  	
#	 "Y8888P"  8888888888 888   T88b     Y8P     8888888888 888   T88b	


# Echo server program
import os
import socket
import sys
from datetime import datetime
from time import sleep

def receive_data(port, packet_count, received_data_path):
	"""
	This function create a server for TCP connexion and write any data 
	sent to the server to a file
	@params:
		port				- Required : TCP port (Int)
		packet_count		- Required : number of packet to be received (Int)
		received_data_path  - Required : path to file to save data (Str)
	"""

	HOST = ""                 # Symbolic name meaning all available interfaces
	
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	s.bind((HOST, port))

	print "Waiting for Connexions"

	s.listen(1)
	conn, addr = s.accept()

	print "Connected by", addr
	print ""
	
	text_file = open(received_data_path, "w")
	cnt       = 0
	first_packet_toggle = 1;

	while cnt < packet_count:

		data = conn.recv(1024)

		# start the timer after the second packet to calculate 
		# transmission speed (throughput)
		if (first_packet_toggle == 2) :
			print "reception started"
			start_time = datetime.now()
			first_packet_toggle = first_packet_toggle + 1
		elif (first_packet_toggle < 2):
			first_packet_toggle = first_packet_toggle + 1
		else :
			pass

		cnt  = cnt + 1
		if not (cnt % 1000):
			print("\r{} / {}". format(cnt, packet_count)),
			sys.stdout.flush()

		if not data: 
			break

		text_file.write(data)

	text_file.close()
	conn.close()

	now_time = datetime.now()
	interval_time = now_time - start_time
	interval_time_f = interval_time.total_seconds()

	size_file = os.path.getsize(received_data_path)

	print"\n\r"

	if (size_file >= 1048576):
		size_file_Mb = float( float(size_file) / 1048576 )
		print "File size received: {0:.3f} Mbytes".format(size_file_Mb)
	else:
		print "File size received: {} bytes".format(size_file)

	data_rate = float (((size_file * 8) / interval_time_f  ) / (10**6))

	
	print "Transmission time: {} seconds".format(interval_time_f)
	print "Data rate transmission: {0:.3f} Mbits/s".format(data_rate)
	print ""

	return 0

if __name__ == "__main__":
	receive_data()