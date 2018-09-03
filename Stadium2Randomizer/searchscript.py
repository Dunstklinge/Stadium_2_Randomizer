
byte = b"1"
searchbytes = frozenset([0x4E, 0x4F, 0x50, 0x53, 0x54, 0xAD])
sset = set(searchbytes)

with open("Pokemon Stadium 2 (U) [!].z64", "rb") as f:
    while byte != b"":
        g = f.tell()
        if g % 1000000 == 0:
            print(g)
        byte = f.read(1)
        intVal = int.from_bytes(byte, "little") #i hate script languages
        if intVal in sset:
            sset.remove(intVal)
            if not sset:
                #empty set
                print(f.tell)
        else:
            nElem = len(sset)
            sset = set(searchbytes)
            if(nElem < 6):
                f.seek(f.tell() - (6-nElem))



        