#!/usr/bin/python

import csv
import re

current_rollover = 0
last_time = 0
tick_time = 10.24 # ms

def convert_time(time_ticks):
    global current_rollover, last_time, tick_time
    if last_time < time_ticks:
        current_rollover += 1
    converted_time = tick_time * time_ticks + (65535 * current_rollover)
    last_time = time_ticks
    return converted_time

print("Enter the name of the input file: ", sep='')

data_file_name = input().strip()
data_file = open(data_file_name, 'r', newline='\n')

tp_file = open('temp_pressure.csv', 'w', newline='')
tp_writer = csv.writer(tp_file)
tp_writer.writerow(['TimeTicks', 'TemperatureC', 'PressurePA'])

accel_file = open('accel.csv', 'w', newline='')
accel_writer = csv.writer(accel_file)
accel_writer.writerow(['TimeTicks', 'dx', 'dy', 'dz'])

mag_file = open('mag.csv', 'w', newline='')
mag_writer = csv.writer(mag_file)
mag_writer.writerow(['TimeTicks', 'mx', 'my', 'mz'])

gyro_file = open('gyro.csv', 'w', newline='')
gyro_writer = csv.writer(gyro_file)
gyro_writer.writerow(['TimeTicks', 'dx', 'dy', 'dz'])

orient_file = open('orient.csv', 'w', newline='')
orient_writer = csv.writer(orient_file)
orient_writer.writerow(['TimeTicks', 'x', 'y', 'z'])

linear_file = open('lin_accel.csv', 'w', newline='')
linear_writer = csv.writer(linear_file)
linear_writer.writerow(['TimeTicks', 'x', 'y', 'z'])

event_file = open('events.csv', 'w', newline='')
event_writer = csv.writer(event_file)
event_writer.writerow(['TimeTicks', 'Event'])

for line in data_file.readlines():
    match = re.search(r'T: ([\d]+) C: (-?[\d]+.?[\d]*) Pa: (-?[\d]+.?[\d]*)', line)

    if (match):
        tp_writer.writerow([ 
            convert_time(int(match.group(1))), 
            match.group(2), 
            match.group(3) 
        ])
        continue

    match = re.search(r'T: ([\d]+) Accl: x= (-?[\d]+.?[\d]*) y= (-?[\d]+.?[\d]*) z= (-?[\d]+.?[\d]*)', line)

    if (match):
        accel_writer.writerow([
            convert_time(int(match.group(1))),
            match.group(2),
            match.group(3),
            match.group(4)
        ])
        continue
    
    match = re.search(r'T: ([\d]+) Mag: x= (-?[\d]+.?[\d]*) y= (-?[\d]+.?[\d]*) z= (-?[\d]+.?[\d]*)', line)

    if (match):
        mag_writer.writerow([
            convert_time(int(match.group(1))),
            match.group(2),
            match.group(3),
            match.group(4)
        ])
        continue
    
    match = re.search(r'T: ([\d]+) Gyro: x= (-?[\d]+.?[\d]*) y= (-?[\d]+.?[\d]*) z= (-?[\d]+.?[\d]*)', line)

    if (match):
        gyro_writer.writerow([
            convert_time(int(match.group(1))),
            match.group(2),
            match.group(3),
            match.group(4)
        ])
        continue
    
    match = re.search(r'T: ([\d]+) Orient: x= (-?[\d]+.?[\d]*) y= (-?[\d]+.?[\d]*) z= (-?[\d]+.?[\d]*)', line)

    if (match):
        orient_writer.writerow([
            convert_time(int(match.group(1))),
            match.group(2),
            match.group(3),
            match.group(4)
        ])
        continue
    
    match = re.search(r'T: ([\d]+) Linear: x= (-?[\d]+.?[\d]*) y= (-?[\d]+.?[\d]*) z= (-?[\d]+.?[\d]*)', line)

    if (match):
        linear_writer.writerow([
            convert_time(int(match.group(1))),
            match.group(2),
            match.group(3),
            match.group(4)
        ])
        continue
    
    match = re.search(r'T: ([\d]+) Firing drogue pyro signal', line)

    if (match):
        event_writer.writerow([ convert_time(int(match.group(1))), "DROGUE" ])
        continue

    match = re.search(r'T: ([\d]+) Firing chute pyro signal', line)

    if (match):
        event_writer.writerow([ convert_time(int(match.group(1))), "MAIN" ])
        continue


data_file.close()
tp_file.close()
accel_file.close()
mag_file.close()
gyro_file.close()
orient_file.close()
linear_file.close()
event_file.close()
