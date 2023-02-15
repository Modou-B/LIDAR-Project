'''info
# titel: LIDAR-Room-Scnanner
# date: 09.11.2022
# autors: Momodou Berlamann, Julian Sommer
# brief explanation:
# This program will read and display the information gathered through a LIDAR-sensor combined, 
# with servo engines to scan rooms and the objects that are in it. 
'''

# main 
import serial
import time
import numpy as np

# Visualization
import open3d as o3d

# initialize Arduino
arduinoData = serial.Serial('com3', 115200)
time.sleep(1)

# create Visualizer and Window
vis = o3d.visualization.Visualizer()
vis.create_window(height=640, width=860)

# set render options (background color and size of the points)
visRenderOption = vis.get_render_option()
visRenderOption.background_color = np.asarray([0,0,0])
visRenderOption.point_size = 1.5

startingPoints = [
    [0,0,0],
    [180,0,0],
    [0,70,0],
    [180,70,0],
    [0,0,400],
    [180,0,400],
    [0,70,400],
    [180,70,400]]

convertedStartingPoints = []
for startingPoint in startingPoints:
    y = startingPoint[2] * np.cos(np.deg2rad(startingPoint[1])) * np.sin(np.deg2rad(startingPoint[0]))
    x = startingPoint[2] * np.cos(np.deg2rad(startingPoint[1])) * np.cos(np.deg2rad(startingPoint[0]))
    z = startingPoint[2] * np.sin(np.deg2rad(startingPoint[1]))
    convertedStartingPoints.append([x,y,z])

# initialize PointCloud instance
pcd = o3d.geometry.PointCloud()
pcd.points = o3d.utility.Vector3dVector(convertedStartingPoints)
vis.add_geometry(pcd)

# initialize PointCloudcolor array with the 8 starting points
pointCloudColors = [
    [0.1,0.1,0.1],
    [0.1,0.1,0.1],
    [0.1,0.1,0.1],
    [0.1,0.1,0.1],
    [0.1,0.1,0.1],
    [0.1,0.1,0.1],
    [0.1,0.1,0.1],
    [0.1,0.1,0.1]
]

# run non-blocking visualization
keepRunning = True
while keepRunning:
    while (arduinoData.inWaiting()== 0):
        arduinoData.cancel_read()
        y = 800 * np.cos(np.deg2rad(180)) * np.sin(np.deg2rad(180))
        x = 800 * np.cos(np.deg2rad(180)) * np.cos(np.deg2rad(180))
        z = 800 * np.sin(np.deg2rad(180))
        pointCloudColors.extend([[0.1,0.1,0.1]])
        pcd.colors = o3d.utility.Vector3dVector(pointCloudColors)      
        pcd.points.extend(o3d.utility.Vector3dVector([[x,y,z]]))
        vis.update_geometry(pcd)
        keepRunning = vis.poll_events()

        if keepRunning == False:
            break

        vis.update_renderer()
        continue
        
    if keepRunning == False:
        break
        
    time.sleep(.001)                  
    dataPacket = arduinoData.readline()            
    # check if full data is received. 
    while not '\\n'in str(dataPacket):        
    
        time.sleep(.001)              
        temp = arduinoData.readline()        
        if not not temp.decode():     
            dataPacket = (dataPacket.decode()+temp.decode()).encode()
      
    dataPacket = str(dataPacket, 'utf-8')
    dataPacket = dataPacket.strip('\r\n')
    splitPacket = dataPacket.split(",")

    if ('' not in splitPacket and len(splitPacket) == 3):
        for i in range(len(splitPacket)):
            splitPacket[i] = '.'.join(splitPacket[i].split('.',-1)[:2])
            
            if splitPacket[i].startswith('.'):
                continue
        
        lidarDistance = float(splitPacket[0])
        xAngle = float(splitPacket[2])
        yAngle = (float(splitPacket[1]) - 100) * -1

        # set the depth of the white color dependent on the lidar distance
        colorDepth = 1 - lidarDistance / 1000

        # convert spherical coordinates to cartesian coordinates
        y = lidarDistance * np.cos(np.deg2rad(yAngle)) * np.sin(np.deg2rad(xAngle))
        x = lidarDistance * np.cos(np.deg2rad(yAngle)) * np.cos(np.deg2rad(xAngle))
        z = lidarDistance * np.sin(np.deg2rad(yAngle))

        # add point with white color depth to cloud
        pointCloudColors.extend([[colorDepth,colorDepth,colorDepth]])
        pcd.colors = o3d.utility.Vector3dVector(pointCloudColors)      
        pcd.points.extend(o3d.utility.Vector3dVector([[x,y,z]]))
        
        vis.update_geometry(pcd)

        print("Y = ",y," X = ",x," Z = ",z)
        
    keepRunning = vis.poll_events()
    vis.update_renderer()
