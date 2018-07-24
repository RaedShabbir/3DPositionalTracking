import cv2
import sys
import argparse
from ar_markers import detect_markers

"""
Provides tracking through 2 mechanisms
-Tracker based with specified selectROI --> move towards using a CONVNET?
-Marker baed mech --> highlights current marker position
"""
#save which opencv version is being used
(major_ver, minor_ver, subminor_ver) = (cv2.__version__).split(".")

#this control flow ensures this file is being called not imported
if __name__ == '__main__':
    #argument parsing for main program
    ap = argparse.ArgumentParser()
    ap.add_argument("-v", "--video", help="path to video from current directory")
    args = vars(ap.parse_args())

    #tracker setup by default uses KCF
    tracker_types = ['BOOSTING', 'MIL','KCF', 'TLD', 'MEDIANFLOW', 'GOTURN']
    tracker_type = tracker_types[1]

    #ensure it works for different versions
    #and initialize tracker type
    if int(minor_ver) < 3:
        tracker = cv2.Tracker_create(tracker_type)
    else:
        if tracker_type == 'BOOSTING':
            tracker = cv2.TrackerBoosting_create()
        if tracker_type == 'MIL':
            tracker = cv2.TrackerMIL_create()
        if tracker_type == 'KCF':
            tracker = cv2.TrackerKCF_create()
        if tracker_type == 'TLD':
            tracker = cv2.TrackerTLD_create()
        if tracker_type == 'MEDIANFLOW':
            tracker = cv2.TrackerMedianFlow_create()
        if tracker_type == 'GOTURN':
            tracker = cv2.TrackerGOTURN_create()

    #activate webcam or livestream
    if not args.get("video", False):
        video = cv2.VideoCapture(0)
    else:
        video = cv2.VideoCapture(args["video"])

    #exit if video not read
    if not video.isOpened():
        print ("ERROR: COULD NOT OPEN VIDEO FILE")
        sys.exit()

    #read frame one
    ok, frame = video.read()
    if not ok:
        print ("ERROR: COULD NOT READ FIRST FRAME FROM FILE")
        sys.exit()

    #init bounding box
    bbox = (287,23,86,320)

    #different initla bounding box done by specifying rectangle of interest
    #bbox = cv2.selectROI(frame,False)
    #print bbox

    #init tracker
    ok = tracker.init(frame,bbox)

    #begin looping over frames
    while True:
        #next frame, make sure its there or break
        ok, frame = video.read()
        if not ok:
            print ("ERROR: COULD NOT READ NEXT FRAME")
            break

        #start timer
        timer = cv2.getTickCount()

        ##marker detection
        markers = detect_markers(frame)
        for marker in markers:
            marker.highlite_marker(frame)

        #update tracker and bbox to current frame
        ok, bbox = tracker.update(frame)

        #calc fps [frames/change in time] == fps
        fps = cv2.getTickFrequency() / (cv2.getTickCount() - timer)

        #draw bbox if tracker updated else indicate failure
        if ok:
            p1 = (int(bbox[0]), int(bbox[1]))
            p2 = (int(bbox[0] + bbox[2]), int(bbox[1] + bbox[3]))
            cv2.rectangle(frame, p1, p2, (255,0,0), 2, 1)
        else:
            cv2.putText(frame, "Tracking failure detected", (100,80), cv2.FONT_HERSHEY_SIMPLEX, 0.75,(0,0,255),2)

        #display tracker type, fps, and resultant frames
        cv2.putText(frame, tracker_type + " Tracker", (100,20), cv2.FONT_HERSHEY_SIMPLEX, 0.75, (50,170,50),2);
        cv2.putText(frame, "FPS : " + str(int(fps)), (100,50), cv2.FONT_HERSHEY_SIMPLEX, 0.75, (50,170,50), 2);
        cv2.imshow("Tracking", frame)

        # Exit if ESC pressed
        k = cv2.waitKey(1) & 0xff
        if k == 27 : break
