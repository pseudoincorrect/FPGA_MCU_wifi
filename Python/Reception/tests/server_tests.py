import sys
sys.path.insert(0, "../src")
import server
import interface
import integrity_check


def test_reception():
	"""
	Main function that organises (automatically) 
	the data reception and data integrity test
	"""
	print ""
	print " *****************************"
	print "            TESTS 		     "
	print " *****************************"
	
	(port, packet_count, received_data_path) = interface.retrieve_info_auto()

	print ""
	print "Connexion parameters :"
	print "Port = {}".format(port) 
	print "Packet_count = {}".format(packet_count) 
	print "Received_data_path = {}".format(received_data_path)
	print ""


	print "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
	print "Let's connect the CC3200"
	print "Or"
	print "Run the launch_client.sh bash script, in project_file"
	print "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
	print ""

	server.receive_data(port, packet_count, received_data_path)

	integrity_check.integrity_check_counter(received_data_path)

	print ""
	print " *****************************"
	print "             END 		     "
	print " *****************************"

if __name__ == "__main__":
	test_reception()