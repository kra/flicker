import serial
import time
import random
import sys

#dev = "/dev/tty.usbmodem12341"
dev = "/dev/ttyACM0"
start_event = 'GameStart'
end_event = 'GameOver'
death_event = '3on'
beat_event = 'BEAT'
addr = '1'

ser = serial.Serial(dev)

ser.write('%s%s:%s\n' % (start_event, addr, 2))
try:
    while True:
        print 'event'
        ser.write('%s%s:%s\n' % (death_event, addr, 2))
        ser.write('%s%s:%s\n' % (beat_event, addr, 2))
        time.sleep(5)
except KeyboardInterrupt:
    ser.write('%s%s:%s\n' % (end_event, addr, 2))
    sys.exit()
