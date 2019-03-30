areas = []
arStart = 0
index = 0


with open("Pokemon Stadium 2 (U) [!].z64", "rb") as file:
    b = file.read(1)
    while b:
        if(int.from_bytes(b,"little") == int(0xFF)):
           if(arStart == 0):
               arStart = index
        else:
            if(arStart != 0 and index - arStart > 0x1000):
                areas.append([arStart, index])
            arStart = 0
        b = file.read(1)
        index = index + 1
        if(index % 0x100000 == 0):
            print(hex(index))
            
for ar in areas:
    print(hex(ar[0]) + " - " + hex(ar[1]) + " (" + (hex(ar[1]-ar[0])) + ")" )