import serial
import time
import subprocess

#chunk_file=open('chunkfile_receive.png','wb+')
#serialPort = '/dev/ttyUSB0'
serialPort = '/dev/ttyACM0'

filenum = 0

#chunk_file=open('image_in.bmp','wb+')

with serial.Serial(serialPort, 115200, timeout=1) as ser:
    while True:
        ser.reset_input_buffer()
        ser.reset_output_buffer()
        #time.sleep(2000)
        print("Checking unread ...")
        ser.write(b"NUM_UNREAD\n")
        time.sleep(2)
        reply=ser.readline().strip()
        replyNum=0
        try:
            replyNum=int(reply)
        except Exception:
            pass
        if(replyNum==1):
            numBytesRead=0
            filename = str(filenum).zfill(3)+"_image.bmp"
            filenum=filenum+1
            chunk_file=open(filename,'wb+')
            time.sleep(.1)
            print("LATEST\n")
            ser.write(b"LATEST\n")
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
            ser.write(b"MARK_READ\n")
            print("sending to server ...")
            directory = "/root/gitwork/bayou/public/images/loracam/"
            subprocess.run(["scp", filename,"root@104.248.50.193:"+directory])

            time.sleep(20) 
        else:
            print("no new images")
            time.sleep(20)
        