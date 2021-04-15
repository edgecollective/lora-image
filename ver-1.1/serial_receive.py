import serial
import time

#chunk_file=open('chunkfile_receive.png','wb+')
#serialPort = '/dev/ttyUSB0'
serialPort = '/dev/ttyACM0'

filenum = 0

#chunk_file=open('image_in.bmp','wb+')

sleepCount = 0

with serial.Serial(serialPort, 115200, timeout=1) as ser:
    while True:
        ser.reset_input_buffer()
        ser.reset_output_buffer()
        #time.sleep(2000)
        print("Checking unread ...")
        ser.write("NUM_UNREAD\n")
        time.sleep(2)
        reply=ser.readline().strip()
        if(int(reply)>0):
            numBytesRead=0
            filename = str(filenum).zfill(3)+"_image.bmp"
            filenum=filenum+1
            chunk_file=open(filename,'wb+')
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
            print("Marking read ...")
            ser.write("MARK_READ\n")
            time.sleep(.1)
        else:
            print("no new images")
        
        while (sleepCount < 10):
            print("will check again in (sec): ",100-sleepCount*10)
            time.sleep(10) # wait 10 seconds
            sleepCount=sleepCount+1
