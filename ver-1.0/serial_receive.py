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
    ser.reset_output_buffer()
    #time.sleep(2000)
    print("Checking unread ...")
    ser.write("NUM_UNREAD\n")
    time.sleep(2)
    reply=ser.readline().strip()
    if(int(reply)>0):
        time.sleep(.1)
        print("LATEST\n")
        ser.write("LATEST\n")
        #time.sleep(.2)
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
    else:
        print("no new images")
