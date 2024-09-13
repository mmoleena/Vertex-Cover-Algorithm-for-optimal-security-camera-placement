import sys
import re
stnames = []
tplist = []
list=[]
# vertices = dict()
# edges = set()

class Point(object):
    def __init__ (self, x, y):
        self.x = float(x)
        self.y = float(y)
    
    def __str__ (self):
        return '(' + str(self.x) + ',' + str(self.y) + ')'
    
    def __eq__ (self, point):
        return self.x == point.x and self.y == point.y
    
class Line(object):
    def __init__ (self, src, dst):
        self.src = src
        self.dst = dst

    def __str__(self):
        return str(self.src) + '-->' + str(self.dst)
    
    def __iter__(self):
        yield self.src
        yield self.dst
    
def intersect (l1, l2):

    x1, y1 = l1.src.x, l1.src.y
    x2, y2 = l1.dst.x, l1.dst.y
    x3, y3 = l2.src.x, l2.src.y
    x4, y4 = l2.dst.x, l2.dst.y

    x_numb = ((x1*y2-y1*x2)*(x3-x4) - (x1-x2)*(x3*y4-y3*x4))
    x_denomb = ((x1-x2)*(y3-y4) - (y1-y2)*(x3-x4))
   

    if (x_denomb == 0):
        return None
    
    cordx =  x_numb / x_denomb
        
    y_numb = ((x1*y2-y1*x2)*(y3-y4) - (y1-y2)*(x3*y4-y3*x4))
    y_denomb = x_denomb
    
    cordy = y_numb / y_denomb
        
    if(min(x1, x2) <= cordx <= max(x1, x2) and min(x3, x4) <= cordx <= max(x3, x4) and min(y1, y2) <= cordy <= max(y1, y2) and min(y3, y4) <= cordy <= max(y3, y4)):
            s1 = Point(x1,y1)
            d1 = Point(x2,y2)
            s2 = Point(x3,y3)
            d2 = Point(x4,y4)
            intersect = Point(cordx, cordy)

            if(intersect == s1 or intersect == s2 or intersect == d1 or intersect == d2):
                if((s1 != s2 and s1 != d2) and (d1 != s2 and d1 != d2)):
                    return Point(cordx,cordy)    
                return None
            else:
                return Point(cordx, cordy)
    
    return None

def strToPoint(coordinate):
     x, y = coordinate.split(',')
     x = x.strip('(')
     y = y.strip(')')
     
     return Point(x,y)

def create_line(coordinates):
    temp = []
    lines = []
    for coord in coordinates:
        if(len(temp)==0):
            temp.append(coord)
            
        else:
            Lin = Line(coord,temp[0])
            temp.pop(0)
            temp.append(coord)
            lines.append(Lin)    

    return lines

def generateGraph(list):
    
    totalLines = []
    vertices = dict()
    edges = set()
    

    for record in list:
        for street, coords in record.items():
            coordsAsPoints = []
            for coord in coords:
            
                p = strToPoint(coord)
                coordsAsPoints.append(p)
            
            allLines = create_line(coordsAsPoints)

            for l in allLines:
                totalLines.append(l)

        #print(f"Total Lines. {len(totalLines)}")

    vert_counter = 1

    for a,l1 in enumerate(totalLines):
        for b,l2 in enumerate(totalLines):
            if(a<b):
                
                intersectionPoint = intersect(l1,l2)
                
                if intersectionPoint:
                    
                    
                    for point in [l1.src,l1.dst,l2.src,l2.dst]:
                        if point not in vertices.values():
                            vertices[vert_counter]=point
                            vert_counter+=1
                    
                    if intersectionPoint not in vertices.values():
                        vertices[vert_counter] = intersectionPoint
                        intersectVertex = vert_counter
                        vert_counter+=1
                    
                    sr1 = [k for k, v in vertices.items() if v == l1.src][0]
                    ds1 = [k for k, v in vertices.items() if v == l1.dst][0]
                    sr2 = [k for k, v in vertices.items() if v == l2.src][0]
                    ds2 = [k for k, v in vertices.items() if v == l2.dst][0]
                    

                    edges.add(tuple(sorted((sr1, intersectVertex))))
                    edges.add(tuple(sorted((sr2, intersectVertex))))
                    edges.add(tuple(sorted((ds1, intersectVertex))))
                    edges.add(tuple(sorted((ds2, intersectVertex))))

    printFunction(vertices,edges)

def printFunction(v,e):

    print('V : {')
    for i,v in v.items():
        print(f'{i}:{v}')
    print('}')

    print('E : {')
    for itr in e:
        print(f'<{itr[0]},{itr[1]}>') 
    print('}')   

def Add_Cmd(street,coordinates):
    
    
    stnames.append(street)

    x = dict([(street,coordinates)])
    list.append(x)
    #print(list)
    

def Mod_Cmd(street,coordinates):

    # count=0
    # for i in range(len(list)):
    #     if list[i][0]== street:
    #         count=i
    #         break
    # list.pop(count)

    # coordinates = [(p.x,p.y) for p in strToPoints(coordinates)]

    for item in list[:]:
        if street in item:
            list.remove(item)

    x = dict([(street,coordinates)])
    
    list.append(x)
    #print(list)
    
def Rem_Cmd(street):

    for item in list[:]:
        if street in item:
            list.remove(item)
    
    #print(list)

def splitInput(line:str):
    line= line.strip()
    coordinates = (re.findall(r"\(.*\)",line))
    street = (re.findall(r"\".*\"",line))
    command = line.split(" ")[0]
    

    return [command,street,coordinates]

def choice(line):   
    [command,street,coordinates] = splitInput(line)
    
    #print(command)
    #print(street)
    #print(coordinates)

    if(command.lower() == "gg"):
        generateGraph(list)
    
    street = ' '.join(street)
    street = street[1:-1]

    coordinates = ''.join(coordinates) #making it a string earlier it was list separted by space
    coordinates = coordinates.split(" ") #identify the space, every set is one element of array
    #print (coordinates)
    #print (len(coordinates))


    if(command.lower() == "add"):
        if(street in stnames):
            #raise(Exception("Street already exists!"))
            print("Street already exists!")

        elif(len(street)==0 or len(coordinates)==0):
            #raise(Exception("Blank Street Name / No co-ordinates"))
            print("Blank Street Name / No co-ordinates")
        
        else:
            stnames.append(street)
            #print("Calling Add_Cmd Function")
            Add_Cmd(street,coordinates)
            
    if(command.lower() == "mod"):
        #if type(coordinates) == int:
        if(street not in stnames):
            #raise(Exception("Street does not exist!"))
            print("Street does not exist!")
        else:
            Mod_Cmd(street,coordinates)
    
    if(command.lower() == "rm"):
        if(street not in stnames):
            #raise(Exception("Wrong Input for rm"))
            print("Cannot remove street. Does not exist!")
        else:
            Rem_Cmd(street)
       

    if((command.lower()!= "add") and (command.lower()!= "mod") and (command.lower()!= "rm") and (command.lower()!= "gg")):
        #raise(Exception("Invalid Command"))
        print("Invalid Command")
            
    #print([command,street,coordinates])

def main():
    #file1 = open('input.txt', 'r')
    #Lines = file1.readlines()
    #for line in Lines:
        #line = sys.stdin.readline()
        #if line == '':
            #break
        #choice(line)
    while True:
        line = sys.stdin.readline()
        if line == '':
            break
        choice(line)
    #print("Finished reading input")
    # return exit code 0 on successful termination
    sys.exit(0)


if __name__ == "__main__":
    main()
   