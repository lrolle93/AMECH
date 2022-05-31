# read data from the imu and plot

# sudo apt-get install python3-pip
# python -m pip install pyserial
# sudo apt-get install python-matplotlib

import serial
import numpy as np
ser = serial.Serial('COM3',230400)
print('Opening port: ')
print(ser.name)

read_samples = 10 # anything bigger than 1 to start out
ax = []
ay = []
az = []
gx = []
gy = []
gz = []
temp = []
print('Requesting data collection...')
ser.write(b'\n')
while read_samples > 1:
    data_read = ser.read_until(b'\n',200) # get the data as bytes
    data_text = str(data_read,'utf-8') # turn the bytes to a string
    data = [float(i) for i in data_text.split()] # turn the string into a list of floats

    if(len(data)==8):
        read_samples = int(data[0]) # keep reading until this becomes 1
        ax.append(data[1])
        ay.append(data[2])
        az.append(data[3])
        gx.append(data[4])
        gy.append(data[5])
        gz.append(data[6])
        temp.append(data[7])
print('Data collection complete')
# plot it
import matplotlib.pyplot as plt 
t = range(len(ax)) # time array
plt.plot(t,ax,'r*-',t,ay,'b*-')
plt.ylabel('G value')
plt.xlabel('sample')
plt.show()

t = range(len(gx)) # time array
plt.plot(t,gx,'r*-',t,gy,'b*-',t)
plt.ylabel('Omega value')
plt.xlabel('sample')
plt.show()

t = range(len(temp)) # time array
plt.plot(t,temp,'r*-')
plt.ylabel('Temperature value')
plt.xlabel('sample')
plt.show()


xang= np.degrees(np.arctan((np.divide(ax,ay))))
yang= np.degrees(np.arctan((np.divide(ay,az))))
plt.plot(t,xang,'r*-',t,yang,'b*')
plt.ylabel('Time')
plt.ylabel('Angle')
plt.show()

# be sure to close the port
ser.close()