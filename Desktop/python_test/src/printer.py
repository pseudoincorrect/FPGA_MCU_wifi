from sys import argv

# This function print all datas contained in a given file
def print_file():

	script, first = argv
	data_file = open(first, "r")
	buff_str  = data_file.read()
	buff      = bytearray(buff_str)
	buff_iter = iter(buff)

	for i in buff_iter:
		a = i
		i = next(buff_iter)
		b = i
		i = next(buff_iter)
		c = i
		i = next(buff_iter)
		d = i
		print("{:02x}{:02x}{:02x}{:02x}".format(a,b,c,d))


# This function print a bytearray buffer word (4 Bytes)
# by word
def printbuffer(buff):

	buff_iter = iter(buff)

	for i in buff_iter:
		a = i
		i = next(buff_iter)
		b = i
		i = next(buff_iter)
		c = i
		i = next(buff_iter)
		d = i
		print("{}{}{}{}".format(a,b,c,d))


if __name__ == "__main__":
	print_file()