import string


def getAllocs(script):
    return [(size(line), pointer(line)) for line in script.readlines() if isMemOp(line)]

def size(line):
    if line[0:9] == "Allocated":
        d = 0
        while(line[10+d].isdigit()):
            d+=1
        return int(line[10:d+11])
    else:
        return 0
            
def pointer(line):
    #print line[-10:]
    return line[-10:]
def isMemOp(line):
    #print line[:4], line[:9]
    return line[:4] == "Free" or line[:9] == "Allocated"

def findLeaks():
    script = open("memLeakRaw.txt", 'r')
    allocs = getAllocs(script)
    script.close()

    allocated, pointers = zip(*allocs)
    
    output = open("memLeakOutput.txt", 'w')
    
    for i in range(len(allocs)):
        if(allocated[i] and not any([not size and p==pointers[i] for (size,p) in allocs[i:]])):
               output.write(str(allocated[i])+" : "+pointers[i])

    output.close()

if __name__ == "__main__":
    findLeaks()
