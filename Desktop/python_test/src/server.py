# Echo server program
import socket

# This function create a server for TCP connexion and write any data 
# sent to the server to a file
def receive_data():
	HOST = ""                 # Symbolic name meaning all available interfaces
	PORT = 50007              # Arbitrary non-privileged port

	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	s.bind((HOST, PORT))
	s.listen(1)
	conn, addr = s.accept()

	print "Connected by", addr

	text_file = open("Received_Data.txt", "w")
	cnt       = 0

	while cnt < 20:

		data = conn.recv(1024)
		cnt  = cnt + 1
		if not data: 
			break
		text_file.write(data)

	text_file.close()
	conn.close()


if __name__ == "__main__":
	receive_data()