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

arduinoData = serial.Serial('com3', 115200)
time.sleep(1)
while True:
    while (arduinoData.inWaiting()== 0):
        pass
    dataPacket = arduinoData.readline()
    print(dataPacket)
