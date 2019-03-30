import re

lines = []

def elem2(l):
    return int(l[1][2])

with open("genTrainerList.txt") as ins:
    for line in ins:
        x = re.findall(r'\d+', line)
        if(len(x) > 3) :
            lines.append([line,x])

for elem in lines:
    print (elem[0])
    print (elem[1])
print("------------------\r\n")
lines.sort(key=elem2)
print("------------------\r\n")
for elem in lines:
    print (elem[0])
    print (elem[1])
with open("outlist.txt", "w+") as out:
    for elem in lines:
        out.write(str(elem[0]))