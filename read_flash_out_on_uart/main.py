import serial

def execute():
    read_buffer_length = 64

    serial_port_name = '/dev/ttyUSB0';
    baudrate = 115200
    timeout = 5 # seconds

    truart = serial.Serial(port=serial_port_name, baudrate=baudrate, timeout=timeout) 

    with fopen('data_file.txt', 'w') as file:

        while True:
            bytes_read = truart.read(read_buffer_length)

            file.write(bytes_read.decode(encoding='ascii', errors='ignore'))

            if len(bytes_read) != read_buffer_length:
                break
    
    return


if __name__ == '__main__':
    execute()