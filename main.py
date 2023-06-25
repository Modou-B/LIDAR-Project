'''info
# titel: LIDAR-Room-Scnanner
# date: 09.11.2022
# autors: Momodou Berlamann, Julian Sommer
# brief explanation:
# This program will read and display the information gathered through a LIDAR-sensor combined, 
# with servo engines to scan rooms and the objects that are in it. 
'''

# main 
import random
import time
import numpy as np
import json

# Visualization
import open3d as o3d

# Mqtt
from paho.mqtt import client as mqtt_client

# Mqtt Client Values
BROKER = '127.0.0.1'
PORT = 1883

# Topics
CORD_TOPIC = "lidar/sendCords"
RESTART_TOPIC = "lidar/restart"

# Current Cord Values
cordsWereUpdated = False
xCord = 0
yCord = 0
zCord = 0

# Current Restart Value
restartValue = 0

def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)

    # Generate a Client ID with the publish prefix.
    client_id = f'Visualization-{random.randint(0, 1000)}'

    client = mqtt_client.Client(client_id)
    client.on_connect = on_connect
    client.connect(BROKER, PORT)
    
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

def subscribeTopics(client: mqtt_client.Client)-> None:
    client.subscribe(CORD_TOPIC)
    client.subscribe(RESTART_TOPIC)
    
def addCallbacksToTopics(client: mqtt_client.Client)-> None:
    def setCords(client, userdata, msg: mqtt_client.MQTTMessage)-> None:
        cordData = json.loads(msg.payload)
        
        global zCord, xCord, yCord, cordsWereUpdated
        zCord = float(cordData['z'])
        xCord = float(cordData['x'])
        yCord = (float(cordData['y']) - 100) * -1
        cordsWereUpdated = True
  
    def setRestartValue(client, userdata, msg: mqtt_client.MQTTMessage)-> None:
        global restartValue

        restartValue = int(msg.payload)

    client.message_callback_add(sub=CORD_TOPIC, callback=setCords)
    client.message_callback_add(sub=RESTART_TOPIC, callback=setRestartValue)

def initStartingPoints(vis)-> None:
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
        [180,70,400]
    ]

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
    
    return pcd
    
def startGUI(): 
    time.sleep(1)
    # create Visualizer and Window
    vis = o3d.visualization.Visualizer()
    vis.create_window(height=640, width=860)

    pcd = initStartingPoints(vis)

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
        if keepRunning == False:
            break
        
        global cordsWereUpdated
        if cordsWereUpdated == True:
            # set the depth of the white color dependent on the lidar distance
            colorDepth = 1 - zCord / 1000

            # convert spherical coordinates to cartesian coordinates
            x = zCord * np.cos(np.deg2rad(yCord)) * np.cos(np.deg2rad(xCord))
            y = zCord * np.cos(np.deg2rad(yCord)) * np.sin(np.deg2rad(xCord))
            z = zCord * np.sin(np.deg2rad(yCord))

            # add point with white color depth to cloud
            pointCloudColors.extend([[colorDepth,colorDepth,colorDepth]])
            pcd.colors = o3d.utility.Vector3dVector(pointCloudColors)      
            pcd.points.extend(o3d.utility.Vector3dVector([[x,y,z]]))
            
            vis.update_geometry(pcd)
            
            cordsWereUpdated = False
         
        global restartValue
        if restartValue == 1:
            vis.clear_geometries()
            restartValue = 0
            pcd = initStartingPoints(vis)

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
            
        keepRunning = vis.poll_events()
        vis.update_renderer()

def setUpClient() -> mqtt_client.Client:
    client = connect_mqtt()
    subscribeTopics(client)
    addCallbacksToTopics(client)
    client.loop_start()
    
    return client

def disconnectClient(client: mqtt_client.Client) -> None:
    client.loop_stop()
    client.disconnect()
    
if __name__ == '__main__':
    client = setUpClient()
    
    startGUI()
    
    disconnectClient(client)