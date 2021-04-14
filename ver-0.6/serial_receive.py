import serial
import time
CHUNK_SIZE = 200
#chunk_file=open('chunkfile_receive.png','wb+')
serialPort = '/dev/ttyUSB0'
#serialPort = '/dev/ttyACM0'

chunk_file=open('image_in.bmp','wb+')
numBytesRead=0
with serial.Serial(serialPort, 115200, timeout=1) as ser:
    ser.reset_input_buffer()
    while True:
        #print(ser.inWaiting())
        if(ser.inWaiting()>0):
            #numBytesRead=numBytesRead+ser.inWaiting()
            inBytes=ser.read(ser.inWaiting())
            print(repr(inBytes))
            #print(numBytesRead)
            #print("\n")
            chunk_file.write(inBytes)
            #time.sleep(.1)
            #chunk_file.close()
