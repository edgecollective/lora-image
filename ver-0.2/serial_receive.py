import serial
import time
CHUNK_SIZE = 200
#chunk_file=open('chunkfile_receive.png','wb+')

chunk_file=open('image_in.bmp','wb+')
with serial.Serial('/dev/ttyUSB0', 115200, timeout=1) as ser:
    ser.reset_input_buffer()
    while True:
        #print(ser.inWaiting())
        if(ser.inWaiting()>0):
            inBytes=ser.read(CHUNK_SIZE)
            print(repr(inBytes))
            #print("\n")
            chunk_file.write(inBytes)
            #time.sleep(.1)
            #chunk_file.close()
