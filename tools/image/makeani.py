import sys
import struct

if __name__ == '__main__':
	if len(sys.argv) < 2 or sys.argv[1] == '-h' or sys.argv[1] == '--help':
		print "./makeani {duration1} {image1.lcd} ... > animat.ani"
		sys.exit(0)
	for (duration, filename) in zip(sys.argv[1::2], sys.argv[2::2]):
		sys.stdout.write(struct.pack('I', int(duration)) + open(filename).read())
	
