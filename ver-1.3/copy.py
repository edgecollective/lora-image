import subprocess
filename="005_image.bmp"
subprocess.run(["scp", filename,"root@104.248.50.193:/root/imagetest/"])
#subprocess.run(["scp",filename,"root@104.248.50.193:/root/imagetest/"])