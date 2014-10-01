import serial
import time
import random
import sys

#dev = "/dev/tty.usbmodem12341"
dev = "/dev/ttyACM0"
start_event = 'GameStart'
end_event = 'GameOver'
beat_event = 'BEAT'
addr = '1'

secs = int(time.time())

ser = serial.Serial(dev)

cmd = '%s%s:%s\n' % (start_event, addr, 2)
print cmd
ser.write(cmd)
try:
    while True:
        print int(time.time()) - secs
        cmd = '%s%s:%s\n' % (beat_event, addr, 2)
        print cmd
        ser.write(cmd)
        time.sleep(5)
except KeyboardInterrupt:
    cmd = '%s%s:%s\n' % (end_event, addr, 2)
    ser.write(cmd)
    ser.write(cmd)
    sys.exit()
