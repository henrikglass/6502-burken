colorbyte = 0x07
fin = open("6502burken_splash_screen.bin", "rb")
fout = open("6502burken_splash_screen_converted.bin", "wb")
data = bytearray()

try:
    read_byte = fin.read(1)
    while read_byte != b"":
        data += read_byte
        data.append(colorbyte)
        read_byte = fin.read(1)
finally:
    fout.write(data)
    fout.close()

fin.close()
