#Alex Cen Feng

import serial
import re
import json
import time
import socket

# COM port and baud rate
SERIAL_PORT = "COM3"
BAUD_RATE   = 115200

# Calibration offsets (the stable/idle values)
CALIB_X = 2030
CALIB_Y = -2240
CALIB_Z = 1330

# ± range around the calibration offsets in which we consider the reading 0
CALIB_RANGE = 70

# Regex to capture three float values
gyro_pattern = re.compile(
    r"GYRO \[X,Y,Z\] = \[\s*([-\d\.]+)\s*,\s*([-\d\.]+)\s*,\s*([-\d\.]+)\s*\]"
)

BUTTON_TEXT = "User button is pressed!"

# Configure a UDP socket
UDP_IP   = "127.0.0.1"  
UDP_PORT = 41234
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def calibrate(raw_value, offset, threshold=70):
    """
    If raw_value is within ±threshold of offset, return 0.
    Otherwise, return (raw_value - offset).
    """
    diff = raw_value - offset
    if abs(diff) <= threshold:
        return 0.0
    return diff

def main():
    # Open the serial port
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    print(f"Reading from {SERIAL_PORT} at {BAUD_RATE} baud...")

    try:
        while True:
            # Read one line from the serial output
            line = ser.readline().decode(errors='ignore').strip()
            if not line:
                continue

            # 1) Check for continuous button press messages
            if BUTTON_TEXT in line:
                print("Continuous button press detected!")
                payload = {
                    "type": "button_press",
                    "message": "User button is pressed"
                }
                sock.sendto(json.dumps(payload).encode('utf-8'), (UDP_IP, UDP_PORT))

            # 2) If not a button press, try to parse gyroscope data
            else:
                match = gyro_pattern.search(line)
                if match:
                    x_str, y_str, z_str = match.groups()

                    # Convert to floats
                    raw_x_val = float(x_str)
                    raw_y_val = float(y_str)
                    raw_z_val = float(z_str)

                    # Apply calibration with ±CALIB_RANGE tolerance
                    x_val = calibrate(raw_x_val, CALIB_X, CALIB_RANGE)
                    y_val = calibrate(raw_y_val, CALIB_Y, CALIB_RANGE)
                    z_val = calibrate(raw_z_val, CALIB_Z, CALIB_RANGE)
                    
                    # Clamp values between -500 and 500 
                    x_val = max(-5000, min(x_val, 5000))
                    y_val = max(-5000, min(y_val, 5000))
                    z_val = max(-5000, min(z_val, 5000))

                    reading = {"x": x_val, "y": y_val, "z": z_val}
                    print(f"Received gyro data (calibrated): {reading}")

                    payload = {
                        "type": "gyro",
                        "x": x_val,
                        "y": y_val,
                        "z": z_val
                    }
                    sock.sendto(json.dumps(payload).encode('utf-8'), (UDP_IP, UDP_PORT))
                else:
                    print(f"No match: {line}")

            time.sleep(0.01)

    except KeyboardInterrupt:
        print("\nStopped by user.")
    finally:
        ser.close()
        sock.close()
        print("Serial port and socket closed.")

if __name__ == "__main__":
    main()
