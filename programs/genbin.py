f = open("test1.bin", "wb")
data = bytearray([0xEA,0xEA,0xEA]*5)
f.write(data)
f.close()
