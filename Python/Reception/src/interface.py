#	8888888 888b    888 88888888888 8888888888 8888888b.  8888888888 
#	  888   8888b   888     888     888        888   Y88b 888        
#	  888   88888b  888     888     888        888    888 888        
#	  888   888Y88b 888     888     8888888    888   d88P 8888888    
#	  888   888 Y88b888     888     888        8888888P"  888        
#	  888   888  Y88888     888     888        888 T88b   888    
#	  888   888   Y8888     888     888        888  T88b  888    
#	8888888 888    Y888     888     8888888888 888   T88b 888    


import os


DEFAULT_PORT = 5001
DEFAULT_PACKET_COUNT = 40000
DEFAULT_PATH_RECEIVED_DATA = "../received_data/received_data_0.dat"
PATH_INFO_FILE = "../info_connexion.dat"

def get_port():
	"""
	Get the port number for TCP connexion
    """
	print ""
	print "Specifie a connexion port number,"
	print "if no value entered default value = {} ".format(DEFAULT_PORT)
	
	try:
		port_raw = int(input("port = "))
	except (NameError, ValueError):
		print "wrong input, port set to {}".format(DEFAULT_PORT)
		port_raw = DEFAULT_PORT
	except SyntaxError:
		port_raw = DEFAULT_PORT
		print DEFAULT_PORT

	return port_raw


def get_packet_count():
	"""
	Get the packet count which determine the amount of data to be received
	"""
	print ""
	print "Specifie a number of packets to receive"
	print "(20,000 packets take 1 minute at 6 Mbits/s (CC3200))"
	print "if no value entered default value = {} " \
		   .format(DEFAULT_PACKET_COUNT)

	try:
		packet_count_raw = int(input("packet count = "))
	except (NameError, ValueError):
		print "wrong input, packet counting set to {}"\
			   .format(DEFAULT_PACKET_COUNT)
		packet_count_raw = DEFAULT_PACKET_COUNT
	except SyntaxError:
		packet_count_raw = DEFAULT_PACKET_COUNT
		print DEFAULT_PACKET_COUNT

	return packet_count_raw


def get_received_data_path():
	"""
	Get path for received data
	"""
	print ""
	print "Specifie a file name/path for the received data"
	print "if no value entered, default file path : {}"\
		   .format(DEFAULT_PATH_RECEIVED_DATA)
	
	file_path_raw = raw_input("file path = ")

	if file_path_raw == "":
		file_path_raw = DEFAULT_PATH_RECEIVED_DATA
		print DEFAULT_PATH_RECEIVED_DATA

	if os.path.isdir(os.path.dirname(file_path_raw)):
			print ""
	else:
		print "path does no exist creating repertory"
		os.makedirs(os.path.dirname(file_path_raw))		
	
	open(file_path_raw, "w").close()

	return file_path_raw


def create_info_file(path_info_file):
	"""
	Create a file contining the data needed for test 
	"""
	print "Creation of a connexion configuration file \n\r"

	port         = get_port()
	packet_count = get_packet_count()
	file_path    = get_received_data_path()

	print "writing connexion parameters into a connexion configuration file \n\r"
	print "port = {}".format(port)
	print "packets count = {}".format(packet_count)
	print "file path = \"{}\"".format(file_path)

	conn_config_file = open(path_info_file, "w")
	conn_config_file.write("port = " + str(port) 			+ "\n")
	conn_config_file.write("packet_count = " + str(packet_count)  + "\n")
	conn_config_file.write("received_data_path = " + str(file_path) 	+ "\n")
	conn_config_file.close()


def user_agree(question, quick_answer):
	"""
	Ask a clocsed question return the boolean answer
	@params: 
		question 	 - Required : question to ask to the use (Str)
		quick_answer - Required : "yes" or "no" (Str)
	"""
	if quick_answer == "yes":
		while True:
			print question
			decision = raw_input("Yes (y) or No (n), no input = Yes: ")
			if decision == "Y" or decision == "y" or decision == "":
				return True
			elif decision == "N" or decision == "n":
				return False
	elif quick_answer == "no":
		while True:
			print question
			decision = raw_input("Yes (y) or No (n), no input = No: ")
			if decision == "Y" or decision == "y":
				return True
			elif decision == "N" or decision == "n" or decision == "":
				return False


def remove_file(path_info_file):
	"""
	Function that remove an existing file
	@params:
		path_info_file - Required : path to the file to delete (Str)
	"""
	os.remove(path_info_file)


def display_info_file(path_info_file):
	"""
	Display the content of a file line by line
	@params:
		path_info_file - Required : path to the file to display (Str)
	"""
	print ""
	print "Configuration parameters:"
	info_file = open(path_info_file, "r")
	info_data = info_file.read().splitlines()
	for i in info_data:
		print i
	info_file.close()
	print ""


def retrieve_info():
	"""
	Retrieve the test configuration from an info file
	"""
	is_file = 1
	modif_file = 0
	
	is_file = os.path.isfile(PATH_INFO_FILE)

	if is_file:
		display_info_file(PATH_INFO_FILE)
		question = "Modification to the configuration (above) needed ?"
		if user_agree(question, "no"):
			modif_file = 1
	else:
		print "no connexion info file found, creating one"

	if (not is_file) or (modif_file):
		if (modif_file):
			remove_file(PATH_INFO_FILE)
		create_info_file(PATH_INFO_FILE)

	with open(PATH_INFO_FILE) as info_file:
	    for line in info_file:
			if "port" in line:
				port = int(line.split("=",1)[1].strip())
			elif "packet_count" in line:
				packet_count = int(line.split("=",1)[1].strip())
			elif "received_data_path" in line:
				received_data_path = line.split("=",1)[1].strip()
				if not os.path.isfile(received_data_path):
					if not os.path.isdir(os.path.dirname(received_data_path)):
						os.makedirs(os.path.dirname(received_data_path))
						open(received_data_path, "w").close()

	info_file.close() 

	return (port, packet_count, received_data_path)


def retrieve_info_auto():
	"""
	Retrieve the test configuration automatically, 
	creating one with default values 
	"""
	is_file = 1
	
	is_file = os.path.isfile(PATH_INFO_FILE)


	if ( is_file):
		remove_file(PATH_INFO_FILE)
	
	port               = DEFAULT_PORT
	packet_count       = DEFAULT_PACKET_COUNT
	received_data_path = DEFAULT_PATH_RECEIVED_DATA

	if not os.path.isfile(received_data_path):
					os.makedirs(os.path.dirname(received_data_path))
					open(received_data_path, "w").close()

	conn_config_file = open(PATH_INFO_FILE, "w")
	conn_config_file.write("port = " + str(port) + "\n")
	conn_config_file.write("packet_count = " + str(packet_count) + "\n")
	conn_config_file.write("received_data_path = " + str(received_data_path) + "\n")
	conn_config_file.close()

	conn_config_file.close() 

	return (port, packet_count, received_data_path)


if __name__ == "__main__":
	retrieve_info()