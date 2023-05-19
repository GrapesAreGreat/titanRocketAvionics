#!/usr/bin/python

import csv
import re

print("Enter the name of the input file: ", newline='')

data_file_name = input().strip()
data_file = open(data_file_name, 'r', newline='\n')

temp_pressure_start_time = 2.0 * 10.24
temp_pressure_sample_rate = 5.0 * 10.24
temp_pressure_current_time = temp_pressure_start_time

tp_file = open('temp_pressure.csv', 'w', newline='')
tp_writer = csv.writer(tp_file)
tp_writer.writerow(['TimeMS', 'TemperatureC', 'PressurePA'])

bno_start_time = 0.0
bno_sample_rate = 10.24 * 2
bno_current_time = bno_start_time

accel_file = open('accel.csv', 'w', newline='')
accel_writer = csv.writer(accel_file)
accel_writer.writerow(['TimeMS', 'dx', 'dy', 'dz', 'x', 'y', 'z'])
ax = ay = az = 0.0

mag_file = open('mag.csv', 'w', newline='')
mag_writer = csv.writer(mag_file)
mag_writer.writerow(['TimeMS', 'mx', 'my', 'mz'])

gyro_file = open('gyro.csv', 'w', newline='')
gyro_writer = csv.writer(gyro_file)
gyro_writer.writerow(['TimeMS', 'dx', 'dy', 'dz', 'x', 'y', 'z'])
rx = ry = rz = 0

orient_file = open('orient.csv', 'w', newline='')
orient_writer = csv.writer(orient_file)
orient_writer.writerow(['TimeMS', 'x', 'y', 'z'])

linear_file = open('lin_accel.csv', 'w', newline='')
linear_writer = csv.writer(linear_file)
linear_writer.writerow(['TimeMS', 'x', 'y', 'z'])

event_file = open('events.csv', 'w', newline='')
event_writer = csv.writer(event_file)
event_writer.writerow(['TimeMS', 'Event'])

for line in data_file.readlines():
    match = re.search(r'C: (-?[\d]+.?[\d]*) Pa: (-?[\d]+.?[\d]*)', line)

    if (match):
        tp_writer.writerow([ 
            str(temp_pressure_current_time), 
            match.group(1), 
            match.group(2) 
        ])
        temp_pressure_current_time += temp_pressure_sample_rate
        continue

    match = re.search(r'Accl: x= (-?[\d]+.?[\d]*) y= (-?[\d]+.?[\d]*) z= (-?[\d]+.?[\d]*)', line)

    if (match):
        dx = float(match.group(1))
        dy = float(match.group(2))
        dz = float(match.group(3))
        ax += dx
        ay += dy
        az += dz
        accel_writer.writerow([
            str(bno_current_time),
            str(dx),
            str(dy),
            str(dz),
            str(ax),
            str(ay),
            str(az)
        ])
        bno_current_time += bno_sample_rate
    
    match = re.search(r'Mag: x= (-?[\d]+.?[\d]*) y= (-?[\d]+.?[\d]*) z= (-?[\d]+.?[\d]*)', line)

    if (match):
        mag_writer.writerow([
            str(bno_current_time),
            match.group(1),
            match.group(2),
            match.group(3)
        ])
        bno_current_time += bno_sample_rate
    
    match = re.search(r'Gyro: x= (-?[\d]+.?[\d]*) y= (-?[\d]+.?[\d]*) z= (-?[\d]+.?[\d]*)', line)

    if (match):
        dx = float(match.group(1))
        dy = float(match.group(2))
        dz = float(match.group(3))
        rx += dx
        ry += dy
        rz += dz
        gyro_writer.writerow([
            str(bno_current_time),
            str(dx),
            str(dy),
            str(dz),
            str(rx),
            str(ry),
            str(rz)
        ])
        bno_current_time += bno_sample_rate
    
    match = re.search(r'Orient: x= (-?[\d]+.?[\d]*) y= (-?[\d]+.?[\d]*) z= (-?[\d]+.?[\d]*)', line)

    if (match):
        orient_writer.writerow([
            str(bno_current_time),
            match.group(1),
            match.group(2),
            match.group(3)
        ])
        bno_current_time += bno_sample_rate
    
    match = re.search(r'Linear: x= (-?[\d]+.?[\d]*) y= (-?[\d]+.?[\d]*) z= (-?[\d]+.?[\d]*)', line)

    if (match):
        orient_writer.writerow([
            str(bno_current_time),
            match.group(1),
            match.group(2),
            match.group(3)
        ])
        bno_current_time += bno_sample_rate
    
    if "Firing drogue pyro signal" in line:
        event_writer.writerow([ min(temp_pressure_current_time, bno_current_time), "DROGUE" ])
    
    if "Firing chute pyro signal" in line:
        event_writer.writerow([ min(temp_pressure_current_time, bno_current_time), "MAIN" ])

data_file.close()
tp_file.close()
accel_file.close()
mag_file.close()
gyro_file.close()
orient_file.close()
linear_file.close()
event_file.close()
