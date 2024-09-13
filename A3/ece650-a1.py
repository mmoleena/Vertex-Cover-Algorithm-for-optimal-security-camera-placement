#!/usr/bin/env python

import sys
import re
import math

stnames = []
list = []

#Below specifies the Point and Line classes, which are representing points and line segments in 2D
class Point(object):
    def __init__(self, x, y):
        self.x = float(x)
        self.y = float(y)

    def __str__(self):
        return '(' + str(self.x) + ',' + str(self.y) + ')'

    def __eq__(self, point):
        return self.x == point.x and self.y == point.y

class Line(object):
    def __init__(self, src, dst):
        self.src = src
        self.dst = dst

    def __str__(self):
        return str(self.src) + '-->' + str(self.dst)

    def __iter__(self):
        yield self.src
        yield self.dst

#Determines if the intersection point between two line segments l1 and l2, exists
#Reference taken from the sample code given in GitLab
def intersect(l1, l2):
    x1, y1 = l1.src.x, l1.src.y
    x2, y2 = l1.dst.x, l1.dst.y
    x3, y3 = l2.src.x, l2.src.y
    x4, y4 = l2.dst.x, l2.dst.y

    x_numb = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4))
    x_denomb = ((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4))

    if (x_denomb == 0):
        return None

    cordx = x_numb / x_denomb

    y_numb = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4))
    y_denomb = x_denomb

    cordy = y_numb / y_denomb

#This part of code determines if the found intersection points, cordx and cordy, are included inside the limits of the two line segments (l1 and l2)
    if (
        min(x1, x2) <= cordx <= max(x1, x2)
        and min(x3, x4) <= cordx <= max(x3, x4)
        and min(y1, y2) <= cordy <= max(y1, y2)
        and min(y3, y4) <= cordy <= max(y3, y4)
    ):
        s1 = Point(x1, y1)
        d1 = Point(x2, y2)
        s2 = Point(x3, y3)
        d2 = Point(x4, y4)
        intersection = Point(cordx, cordy)

        if (
            intersection == s1
            or intersection == s2
            or intersection == d1
            or intersection == d2
        ):
            if (s1 != s2 and s1 != d2) and (d1 != s2 and d1 != d2):
                return Point(cordx, cordy)
            return None
        else:
            return Point(cordx, cordy)

    return None

#This function parses and translates a coordinate string in the format (x, y) into a Point object
def strToPoint(coordinate):
    x, y = coordinate.split(',')
    x = x.strip('(')
    y = y.strip(')')

    return Point(x, y)

#This function creates line segments from a list of coordinates
def create_line(coordinates):
    temp = []
    lines = []
    for coord in coordinates:
        if len(temp) == 0:
            temp.append(coord)
        else:
            Lin = Line(coord, temp[0])
            temp.pop(0)
            temp.append(coord)
            lines.append(Lin)

    return lines

# This function creates a graph from a list of street coordinates, calculating intersections, vertices, and edges.
def generateGraph(list):
    totalLines = []
    vertices = dict()
    edges = set()
    intersectVertex = None

    for record in list:
        for street, coords in record.items():
            coordsAsPoints = []
            for coord in coords:
                p = strToPoint(coord)
                coordsAsPoints.append(p)
            allLines = create_line(coordsAsPoints)

            for line in allLines:
                totalLines.append(line)

    vert_counter = 1

    for i in range(len(totalLines)):
        for j in range(i + 1, len(totalLines)):
            l1 = totalLines[i]
            l2 = totalLines[j]

            intersectionPoint = intersect(l1, l2)
            if intersectionPoint:
                for line in [l1, l2]:
                    for point in [line.src, line.dst]:
                        if point not in vertices.values():
                            vertices[vert_counter] = point
                            vert_counter += 1

                if intersectionPoint not in vertices.values():
                    vertices[vert_counter] = intersectionPoint
                    intersectVertex = vert_counter
                    vert_counter += 1

                idx1, _ = next((n, v) for n, v in vertices.items() if v == l1.src)
                idx2, _ = next((n, v) for n, v in vertices.items() if v == l1.dst)
                idx3, _ = next((n, v) for n, v in vertices.items() if v == l2.src)
                idx4, _ = next((n, v) for n, v in vertices.items() if v == l2.dst)

                edges.add(tuple(sorted((idx1, intersectVertex))))
                edges.add(tuple(sorted((idx2, intersectVertex))))
                edges.add(tuple(sorted((idx3, intersectVertex))))
                edges.add(tuple(sorted((idx4, intersectVertex))))

    # Below line recalculates the edges after collecting all the new intersections
    vertices, edges = recompute_Edges(vertices, edges, totalLines)

    return vertices, edges

#This function recalculates graph edges by taking into account line segment intersections and creating ad hoc edges based on the closest vertices
#Have taken multiple online references for this function
def recompute_Edges(vertices, edges, lines_sum):
    intersecting_line = []
    adhoc_edges = set()

    for a in range(len(lines_sum)):
        for b in range(a + 1, len(lines_sum)):
            pt_intersect = intersect(lines_sum[a], lines_sum[b])
            if pt_intersect:
                intersecting_line.append(pt_intersect)

    for edge in edges:
        fromVertex = vertices.get(edge[0])
        toVertex = vertices.get(edge[1])

        if fromVertex and toVertex:
            temporaryLineSegment = Line(fromVertex, toVertex)
            uncertainPoints = [fromVertex, toVertex]

            for intersection in intersecting_line:
                if ValidPointOnLineSeg(temporaryLineSegment, intersection):
                    uncertainPoints.append(intersection)

            vrtx_dic = {(v.x, v.y): n for n, v in vertices.items() if (v.x, v.y) in [(p.x, p.y) for p in uncertainPoints]}

            for key_vertex in uncertainPoints:
                vertex_index = vrtx_dic.get((key_vertex.x, key_vertex.y))
                if vertex_index:
                    finalConVertx = findClosestVertex(key_vertex, uncertainPoints)
                    VxConIndex = vrtx_dic.get((finalConVertx.x, finalConVertx.y))
                    if VxConIndex:
                        adhoc_edges.add(tuple(sorted([vertex_index, VxConIndex])))

    return vertices, adhoc_edges



#Based on Euclidean distance, this function determines the nearest vertex to a specified point within a set of vertices
def findClosestVertex(vertex, totalVertices):
    ClosestVertex = None

    for i in totalVertices:
        if i == vertex:
            continue

        distance = math.sqrt((vertex.x - i.x) ** 2 + (vertex.y - i.y) ** 2)
        
        try:
            if distance < shortestDistance:
                shortestDistance = distance
                ClosestVertex = i
        except NameError:
            shortestDistance = distance
            ClosestVertex = i

    return ClosestVertex

#This function checks the validity of a point on a line segment
#Online reference taken for this function
def ValidPointOnLineSeg(Li, ptr):
    if ptr == Li.src or ptr == Li.dst:
        return False

    fx_small_value = 0.000001

    x1, y1 = Li.src.x, Li.src.y
    x2, y2 = Li.dst.x, Li.dst.y
    x, y = ptr.x, ptr.y

    vectorCrossProduct = (y - y1) * (x2 - x1) - (x - x1) * (y2 - y1)

    if abs(vectorCrossProduct) < fx_small_value:
        if (x1 <= x <= x2 or x2 <= x <= x1) and (y1 <= y <= y2 or y2 <= y <= y1):
            return True

    return False

#This function prints the final count of Vertices and the Edges
def printFunction(vertices, edges):
    print('V {}'.format(len(vertices)))
    if edges:
        e_str = 'E {'
        edge_strings = ['<{},{}>'.format(edge[0], edge[1]) for edge in edges]
        e_str += ','.join(edge_strings)
        e_str += '}'
        print(e_str)
    sys.stdout.flush()

#Defining add command 
def Add_Cmd(street, coordinates):
    stnames.append(street)
    x = dict([(street, coordinates)])
    list.append(x)

#Defining mod command
def Mod_Cmd(street, coordinates):
    for item in list[:]:
        if street in item:
            list.remove(item)

    x = dict([(street, coordinates)])
    list.append(x)

#Defining rm command
def Rem_Cmd(street):
    for item in list[:]:
        if street in item:
            list.remove(item)

#This function divides the input line into three sections: the command, the street name and the coordinates, and returns them in a list
def splitInput(line):
    line = line.strip()
    coordinates = (re.findall(r"\(.*\)", line))
    street = (re.findall(r"\".*\"", line))
    command = line.split(" ")[0]

    return [command, street, coordinates]


#This function does the error validations of the input
def choice(line):
    [command, street, coordinates] = splitInput(line)

    if (command.lower() == "gg"):
        vertices, edges = generateGraph(list)
        printFunction(vertices, edges)

    street = ' '.join(street)
    street = street[1:-1]

    coordinates = ''.join(coordinates)
    coordinates = coordinates.split(" ")

    if (command.lower() == "add"):
        if street in stnames:
            print("Error: Street already exists!")
        elif (len(street) == 0 or len(coordinates) == 0):
            print("Error: Blank Street Name / No co-ordinates")
        else:
            stnames.append(street)
            Add_Cmd(street, coordinates)

    if (command.lower() == "mod"):
        if street not in stnames:
            print("Error: Street does not exist!")
        else:
            Mod_Cmd(street, coordinates)

    if (command.lower() == "rm"):
        if street not in stnames:
            print("Error: Cannot remove street. Does not exist!")
        else:
            Rem_Cmd(street)

    if (
        (command.lower() != "add")
        and (command.lower() != "mod")
        and (command.lower() != "rm")
        and (command.lower() != "gg")
    ):
        print("Error: Invalid Command")

#Main function for this program
def main():
    while True:
        line = sys.stdin.readline()
        if line == "":
            break
        choice(line)
    sys.exit(0)

if __name__ == "__main__":
    main()
