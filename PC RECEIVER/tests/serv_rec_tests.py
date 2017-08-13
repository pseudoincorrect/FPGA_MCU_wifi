from nose.tools import *
import sys
sys.path.insert(0, "/home/max/prog/python/MIT/serv_rec")

from src.server import receive_data
from src.client import send_non_packed_data
from src.integrity_check import integrity_check_counter

# To run nosetests, please run the following command in the 
# main folder (serv_rec) :
# nosetests -v --nocapture

def test_basic():
	print "tests running"
	print "please open a new terminal and run client.py"
	receive_data()
	integrity_check_counter()
	print "tests done"

if __name__ == "__main__":
	test_basic()
