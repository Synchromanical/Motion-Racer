# Motion-Racer
Pseudo3D Racer with STM32 Motion Controller

A full-stack “motion controller” racing game:  
- **Frontend**: A pseudo-3D JavaScript racer in the browser.  
- **Backend**: A Python bridge that reads STM32 gyroscope/button data over UART and relays it via UDP/WebSocket.  
- **Firmware**: STM32L475 IoT board firmware that samples the on-board LSM6DSL gyro and button and prints over UART.

## Features

- **Pseudo-3D Racing**  
  - Canvas-based rendering of a 3D road, cars & sprites  
  - Adjustable lanes, draw distance, FOV, fog, camera height, etc.  

- **Motion-Based Steering**  
  - Gyroscope-driven left/right input  
  - Button for throttle (“accelerate”) 

- **Seamless Integration**  
  - Python backend parses UART, applies calibration & dead-zone, sends JSON‐over‐UDP  
  - Frontend polls `/latestGyro` and listens on Socket.IO for button presses  

## System Architecture


1. **Firmware**
   - Initializes BSP gyroscope (LSM6DSL) & button  
   - Every 500 ms: reads gyro (X,Y,Z), prints  
   - On button press: prints `User button is pressed!`

2. **Backend** 
   - Opens COM3 @115200 baud, parses lines via regex  
   - Calibrates & dead-zones raw gyro around offsets  
   - Sends UDP JSON payloads:  
     - `{ type: "gyro", x, y, z }`  
     - `{ type: "button_press" }`  
   - Includes simple range-clamping and 10 ms loop 

3. **Frontend**  
   - Renders a “3D” road via canvas; supports dynamic settings via UI panels  
   - Polls `/latestGyro` endpoint every 100 ms to steer left/right based on `y` value  
   - Listens on Socket.IO for “button_press” to accelerate  
   - Lap timing, HUD, leaderboards & save/quit logic 
