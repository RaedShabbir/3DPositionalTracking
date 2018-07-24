import numpy as np
import cv2
import cv2.aruco as aruco
import sys
import argparse
from ar_markers import detect_markers
from scipy.spatial import distance as dist

def generateMarkers():
    aruco_dict = aruco.Dictionary_get(aruco.DICT_6X6_250)
    print(aruco_dict)
    # second parameter is id number
    # last parameter is total image size
    img = aruco.drawMarker(aruco_dict, 2, 700)
    cv2.imwrite("../Samples/Pictures/Markers/marker.jpg", img)

    cv2.imshow('frame',img)
    cv2.waitKey(0)
    cv2.destroyAllWindows()


def calcValues(refCoords, objCoords):

    """
    Take two objects and their coordinates in the form of a list or tuple
    Each object is defined by a bounding rectangle
    returns:
        - euclidean distance between two in pixels
        - center point of each object
        - ppm ratio current
    """
    a,b = refCoords
    c,d = objCoords

    centerRef = midpoint(a,b)
    centerObj = midpoint(c,d)

    euclid_dist = dist.euclidean()

    ppm = dist.euclidean()
    return euclid_dist, ppm
