import serial
import time
CHUNK_SIZE = 200
#chunk_file=open('chunkfile_receive.png','wb+')
#serialPort = '/dev/ttyUSB0'
serialPort = '/dev/ttyACM0'

chunk_file=open('image_in.bmp','wb+')
numBytesRead=0
with serial.Serial(serialPort, 115200, timeout=1) as ser:
    ser.reset_input_buffer()
    #time.sleep(2000)
    print("--> NEW\n")
    ser.write("NEW\n")
    time.sleep(2) # the gateway listens to radio for 1 sec ea loop; make sure we give it time
    reply=ser.readline().strip()
    print("<--",reply)
    time.sleep(1)
    if(int(reply)>0):
        ser.write("LATEST\n")
        time.sleep(.2)
        while (numBytesRead < 9600):
            #print(ser.inWaiting())
            #if(ser.inWaiting()>0):
            numBytesRead=numBytesRead+ser.inWaiting()
            inBytes=ser.read(ser.inWaiting())
            #print(repr(inBytes))
            print(numBytesRead)
            chunk_file.write(inBytes)
            chunk_file.flush()
            time.sleep(.1)
        chunk_file.close()
