import numpy as np
import cv2
import cv2.aruco as aruco
import sys

video = cv2.VideoCapture(0)

#Error checking
if not video.isOpened():
    print ("ERROR: COULD NOT OPEN VIDEO ")
    sys.exit()

aruco_dict = aruco.getPredefinedDictionary(aruco.DICT_4X4_50)

while(True):
    #read frame
    ok, frame = video.read()
    if not ok:
        print ("ERROR: COULD NOT READ FIRST FRAME FROM FILE")
        sys.exit()

    #frame operations
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    parameters = aruco.DetectorParameters_create()

    #list ids and respective corners
    corners, ids, rejectedImgPoints = aruco.detectMarkers(gray, aruco_dict, parameters=parameters)
    frame = aruco.drawDetectedMarkers(frame, corners, borderColor=(255,0,0))
    frame = aruco.drawDetectedMarkers(frame, rejectedImgPoints, borderColor=(0,0,255))

    cv2.imshow('frame', frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

video.release()
cv2.destroyAllWindows()
