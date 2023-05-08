import numpy

class Point:
    def __init__(self, _id, x, y):
        self.id = _id
        self.x = x
        self.y = y

class Element:
    def __init__(self, _id, p1: Point, p2: Point, p3: Point):
        self.id = _id - 1
        self.p1 = p1
        self.p2 = p2
        self.p3 = p3

    def get_perimeter(self):
        sum = 0
        sum += numpy.sqrt((self.p1.x - self.p2.x) ** 2 + (self.p1.y - self.p2.y) ** 2)
        sum += numpy.sqrt((self.p1.x - self.p3.x) ** 2 + (self.p1.y - self.p3.y) ** 2)
        sum += numpy.sqrt((self.p2.x - self.p3.x) ** 2 + (self.p2.y - self.p3.y) ** 2)
        return sum
        