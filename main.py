'''info
# titel: LIDAR-Room-Scnanner
# date: 09.11.2022
# autors: Momodou Berlamann, Julian Sommer

# brief explanation:
# This program will read and display the information gathered through a LIDAR-sensor combined, 
# with servo engines to scan rooms and the objects that are in it. 
'''

'''documentation
'''

# main 
import serial
import time
import numpy as np

# Visualization
import open3d as o3d

# csv
import csv
    

# initialize csv file
f = open('path/to/csv_file', 'w')

writer = csv.writer(f)
header = ['xAngle','yAngle','distance']
writer.writerow(header)

# initialize Arduino
arduinoData = serial.Serial('com3', 115200)
time.sleep(1)

# create Visualizer and Window
vis = o3d.visualization.Visualizer()
vis.create_window(height=640, width=860)
visRenderOption = vis.get_render_option()
visRenderOption.background_color = np.asarray([0,0,0])

# initialize PointCloud instance
pcd = o3d.geometry.PointCloud()
pcd.points = o3d.utility.Vector3dVector([[0,0,140],[-20,-20,0]])
vis.add_geometry(pcd)

# run non-blocking visualization
keepRunning = True
while keepRunning:
    while (arduinoData.inWaiting()== 0):
        pass
    dataPacket = arduinoData.readline()
    dataPacket = str(dataPacket, 'utf-8')
    dataPacket = dataPacket.strip('\r\n')
    splitPacket = dataPacket.split(",")

    if (splitPacket[0] != "Initializing..."):
        lidarDistance = int(splitPacket[0])
        xAngle = int(splitPacket[2])
        yAngle = (int(splitPacket[1]) - 100) * -1

        # write real world data to csv file for testing
        writer.writerow([xAngle, yAngle, lidarDistance])

        # convert spherical coordinates to cartesian coordinates
        y = lidarDistance * np.cos(np.deg2rad(yAngle)) * np.sin(np.deg2rad(xAngle))
        x = lidarDistance * np.cos(np.deg2rad(yAngle)) * np.cos(np.deg2rad(xAngle))
        z = lidarDistance * np.sin(np.deg2rad(yAngle))

        # add point to cloud
        numpyArray = np.asarray([[x,y,z]])
        pcd.points.extend(o3d.utility.Vector3dVector(numpyArray))
        pcd.paint_uniform_color([0.9, 0.9, 0.9])
        print(lidarDistance,"cm"," X =",x,"Y =",y)
        
        vis.update_geometry(pcd)
    
    keepRunning = vis.poll_events()
    vis.update_renderer()

f.close()
# vis.destroy_window
    

   
