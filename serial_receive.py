import serial
CHUNK_SIZE = 50
#chunk_file=open('chunkfile_receive.png','wb+')


while True:
    with serial.Serial('/dev/ttyACM0', 115200, timeout=1) as ser:
        inBytes=ser.read(50)
        if (len(inBytes)>5):
            print(repr(inBytes))
            print("\n")
            chunk_file=open('image_in.png','ab+')
            chunk_file.write(inBytes)
            chunk_file.close()
