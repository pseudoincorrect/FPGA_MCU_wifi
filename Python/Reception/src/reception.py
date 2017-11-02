
import interface
import server
import integrity_check

def reception():
	"""
	Mains function that organises (with inputs from the usr)
	the data reception and data integrity test
	"""
	print ""
	print " *****************************"
	print "          RECEPTION 		     "
	print " *****************************"
	
	(port, packet_count, received_data_path) = interface.retrieve_info()

	print ""
	print "Connexion parameters :"
	print "Port = {}".format(port) 
	print "Packet_count = {}".format(packet_count) 
	print "Received_data_path = {}".format(received_data_path)
	print ""

	print "Let's connect the CC3200"
	print "Or"
	print "Run the client.py python script"
	print ""

	server.receive_data(port, packet_count, received_data_path)

	print "to display the content of the file in an unix environement"
	print "you can use the following command :"
	print "hexdump -e \'/4 \"%08X\\n\"\' path_to_your_received_data_file.dat"
	print ""

	question = "Proceed to integrity test ?"
	if (interface.user_agree(question, "no")):
		integrity_check.integrity_check_counter(received_data_path)

	print ""
	print " *****************************"
	print "             END 		     "
	print " *****************************"

if __name__ == "__main__":
	reception()