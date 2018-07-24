import numpy
import cv2
import argparse
import imutils

video = cv2.VideoCapture(0)

if (video.isOpened() == False):
    print("Error opening videstream or file")

while (video.isOpened()):
    ok, frame = video.read()
    if ok:
        cv2.imshow("Frame", frame)
        if cv2.waitKey(25) & 0xFF == ord('q'):
            break
    else:
        print ("something went wrong")
        break

video.release()
cv2.destroyAllWindows()
