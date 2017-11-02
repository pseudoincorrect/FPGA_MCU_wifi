import struct
import sys


def reconstitute(a, b, c, d):
	"""
	# This fonction concatenate 4 bytes into a word
	@params:
		a - Required : first  byte 0x000000XX (Int)
		b - Required : second byte 0x000000XX (Int)
		c - Required : third  byte 0x000000XX (Int)
		d - Required : fourth byte 0x000000XX (Int)
	"""
	whole = (   ((d << 24) & 0xFF000000)
		  	  | ((c << 16) & 0x00FF0000)
		  	  | ((b <<  8) & 0x0000FF00)
		  	  | ( a        & 0x000000FF) )
	return whole


def integrity_check_counter(received_data_path):
	"""
	This function will test the integrity of the data of a given file
	It will open the actual file and verify that each data af incrementing
	by one, one after an other
	@params:
		received_data_path - Required : path to the received data file (Str)
	"""
	data_file    = open(received_data_path, "r")
	buff_str     = data_file.read()
	whole        = 0
	whole_prec   = 0
	packet_cnt   = 0
	error_cnt    = 0
	first		 = 0
	first_error  = 1
	buff         = bytearray(buff_str)
	buff_iter    = iter(buff)
	word_current = [0,0,0,0]

	for j in range(0, 8):
		next(buff_iter)

	for i in buff_iter:
		packet_cnt      = packet_cnt + 1
		word_current[0] = i

		for j in range(1, 4):
			i = next(buff_iter)
			word_current[j] = i

		# reconstitute a word with 4 bytes
		whole = reconstitute(*word_current)

		if (whole_prec != (whole - 1) ):
			# we ignore the first error, since it is usually due 
			# to a RingBuffer synchronisation at the beginning of 
			# the transmission
			if (first < 2):
				first = first + 1
				pass
			else:
				error_cnt = error_cnt + (whole - whole_prec)
				# error_cnt = error_cnt + 1
				print "******************************"
				print "ERROR ar value #{}".format(packet_cnt)
				print "value N-2 = {}".format(whole_prec_prec)
				print "value N-1 = {}".format(whole_prec)
				print "value N   = {}".format(whole)
				print "Error count = {}".format(error_cnt)
				print "******************************"

		whole_prec_prec = whole_prec
		whole_prec = whole

	print ""
	if error_cnt == 0:
		print "Integrity test: no error"
	else:
		print "Integrity test: {} 32 bits words lost". format(error_cnt)
	
	print "Total values tested: {}". format(packet_cnt)

	return error_cnt

	

def integrity_check_packet(received_data_path):
	"""
	This function will test the integrity of the data of a given file
	It will open the actual file and verify the the Headers, ID, and 
	data integrity of all transmission recorded in the file
	@params:
		received_data_path - Required : path to the received data file (Str)
	"""
	data_file    = open(received_data_path, "r")
	buff_str     = data_file.read()
	whole        = 0
	whole_prec   = 0
	packet_cnt   = 0
	error_cnt    = 0
	ignore       = 1
	buff         = bytearray(buff_str)
	buff_iter    = iter(buff)
	word_current = [0,0,0,0]

	for i in buff_iter:
		packet_cnt      = packet_cnt + 1
		word_current[0] = i

		for j in range(1, 4):
			i = next(buff_iter)
			word_current[j] = i

		# whole and whole_prec contain the current and precedent 
		# word (4 bytes) 
		whole_prec_prec = whole_prec
		whole_prec = whole

		# reconstitute a word with 4 bytes
		whole = reconstitute(*word_current)

		if ignore == 0:
			if (whole_prec != (whole - 1)):
				error_cnt = error_cnt + 1
				ignore    = 1
				print "ERROR ar value #{}".format(packet_cnt)
				print "value N-2 = {}".format(whole_prec_prec)
				print "value N-1 = {}".format(whole_prec)
				print "value N   = {}".format(whole)
		else:
			ignore = 0

	if error_cnt == 0:
		print "integrity test: no error"
	else:
		print "integrity test: {} error(s)". format(error_cnt)



def integrity_string_conversion(received_data_path):
	"""
	This function will convert the data received to a string format for an easier display. 
	When received,  the data bytes appears in the wrong order. since we don't reorder the data 
	during the receiption, we will use this function to reorder them in a readable 
	form. 
	@params:
		received_data_path - Required : path to the received data file (Str)
	"""
	data_file    = open(received_data_path, "r")
	buff_str     = data_file.read()

	str_split = received_data_path.split(".")
	
	temp_path = ""

	for i in str_split:
		last_path = temp_path
		if (i == ''):
			temp_path += "."
		else:
			temp_path += i
		extension = i

	data_reconst_path = last_path + "_reconstructed." + extension

	reconst_file = open(data_reconst_path, "w")

	buff         = bytearray(buff_str)
	buff_iter    = iter(buff)
	word_current = [0,0,0,0]

	whole = 0
	packet_cnt   = 0

	print "\n\rreconstruction... \n\r"

	for j in range(0, 8):
		next(buff_iter)

	for i in buff_iter:
		packet_cnt      = packet_cnt + 1
		word_current[0] = i

		for j in range(1, 4):
			i = next(buff_iter)
			word_current[j] = i

		# reconstitute a word with 4 bytes
		whole  = reconstitute(*word_current)

		# if we want to write a string representation of a integer (take twice more space)
		reconst_file.write(str(whole) + "\n\r")
		# if we want to write a integer
		# reconst_file.write(struct.pack('i', whole)) # write an int

if __name__ == "__main__":
	integrity_check_counter(sys.argv[1])
