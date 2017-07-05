import struct


# This fonction concatenate 4 bytes into a word
def reconstitute(a, b, c, d):
	whole = (   ((a << 24) & 0xFF000000)
		  	  | ((b << 16) & 0x00FF0000)
		  	  | ((c <<  8) & 0x0000FF00)
		  	  | ( d        & 0x000000FF) )
	return whole

# This function will test the integrity of the data of a given file
# It will open the actual file and verify that each data af incrementing
# by one, one after an other
def integrity_check_counter():
	data_file    = open("Received_Data.txt", "r")
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

		whole_prec = whole
		whole      = reconstitute(*word_current)

		if ignore == 0:
			if (whole_prec != (whole - 1)):
				error_cnt = error_cnt + 1
				ignore    = 1
				print ("ERROR ar value #{}".format(packet_cnt))
		else:
			ignore = 0

	if error_cnt == 0:
		print ("integrity test: no error")
	else:
		print ("integrity test: {} error(s)". format(error_cnt))


# This function will test the integrity of the data of a given file
# It will open the actual file and verify the the Headers, ID, and 
# data integrity of all transmission recorded in the file
def integrity_check_packet(data_size):
	data_file    = open("Received_Data.txt", "r")
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
		whole_prec = whole
		whole      = reconstitute(*word_current)

		# MAYBE USE STRUCT HERE (Todo)

		if ignore == 0:
			if (whole_prec != (whole - 1)):
				error_cnt = error_cnt + 1
				ignore    = 1
				print ("ERROR ar value #{}".format(packet_cnt))
		else:
			ignore = 0

	if error_cnt == 0:
		print ("integrity test: no error")
	else:
		print ("integrity test: {} error(s)". format(error_cnt))



if __name__ == "__main__":
	# integrity_check_packet()
	integrity_check_counter()
