import time

# STITCH IMAGE BACK TOGETHER
# Normally this will be in another location to stitch it back together
read_file = open('chunkfile_receive.txt', 'rb')

# Create the jpg file
with open('stitched_together.png', 'wb') as image:
    for f in read_file:
        image.write(f)