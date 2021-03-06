import time
import serial
CHUNK_SIZE = 50

ser=serial.Serial('/dev/ttyACM1')

image_file = 'flower.png'

chunk_file=open('chunkfile_test.txt','wb+')

index = 0 
with open(image_file, 'rb') as infile:
    while True:
        # Read 430byte chunks of the image
        chunk = infile.read(CHUNK_SIZE)
        if not chunk: break

        # Do what you want with each chunk (in dev, write line to file)
        #print(index, chunk)
        #print("")

        
        #time.sleep(.4)

        sercom = "SEND " + chunk + " 1 10\n"


        if(index==5):
            ser.write(sercom)
            print(repr(sercom))

        chunk_file.write(chunk)
        time.sleep(.5)

        index=index+1

chunk_file.close()

# STITCH IMAGE BACK TOGETHER
# Normally this will be in another location to stitch it back together
read_file = open('chunkfile_test.txt', 'rb')

# Create the png file
with open('stitched_together.png', 'wb') as image:
    for f in read_file:
        image.write(f)