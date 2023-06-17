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
# import open3d as o3d

# python 3.6

import random
import time

from paho.mqtt import client as mqtt_client


broker = '127.0.0.1'
port = 1883
topic = "lidar/sendCords"
# Generate a Client ID with the publish prefix.
client_id = f'publish-{random.randint(0, 1000)}'
# username = 'emqx'
# password = 'public'

def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(client_id)
    # client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client


# def publish(client):
#     # msg_count = 1
#     # while True:
#         time.sleep(1)
#         msg = f"Funktioniert"
#         result = client.publish(topic, msg)
#         # result: [0, 1]
#         status = result[0]
#         if status == 0:
#             print(f"Send `{msg}` to topic `{topic}`")
#         else:
#             print(f"Failed to send message to topic {topic}")
#         msg_count += 1
        # if msg_count > 5:
        #     break


def subscribe(client: mqtt_client):
    def on_message(client, userdata, msg):
        print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")
        

    client.subscribe(topic)
    client.on_message = on_message
    # PARSE YXZ
    
    msg.payload.decode()
    
    
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
    
    
    

def runMQTT():
    client = connect_mqtt()
    subscribe(client)
    client.loop_forever()
    # client.loop_start()
    # publish(client)
    
    # client.loop_stop()








def startGUI(y, x, z): 
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

    


if __name__ == '__main__':
    startGUI()