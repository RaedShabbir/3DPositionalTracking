# 3D Positional Tracking with Markers

Program to calibrate camera and track cartesian distances to markers, with camera as the origin. Greatest accuracy achieved was within 1mm. The intended purpose was for use as a tracking module in an arthroscopic surgical training station.

To-Do: 
- [ ] Implement final code in Python to test speed differences
- [ ] Implement particle tracking to combat marker occlusion 
- [ ] Implement dual camera setup to further combat occlusion.   

![Alt text](https://github.com/RaedShabbir/3DPositionalTracking/blob/master/Result%20Pictures/calibration.PNG)
Camera Calibration 
![Alt text](https://github.com/RaedShabbir/3DPositionalTracking/blob/master/Result%20Pictures/90degrees%2030%20cm%20away%202.PNG)

Great thanks to George Lecakes for amazing series on marker tracking with OpenCV. Can be found here for those interested: https://www.youtube.com/watch?v=CfymgQwB_vE
