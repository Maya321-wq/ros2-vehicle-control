# ROS 2 Autonomous Vehicle Control System

Features
- **Custom Physics Simulator:** Simulates vehicle kinematics (speed, position, yaw).
- **Modular Controllers:** Basic P-Controller, Time-Based State Machine, and Event-Driven Controller.
- **Event-Driven Logic:** Reacts to traffic lights and emergency obstacle detection.
- **Real-Time Visualization:** 2D Matplotlib path tracking and 3D RViz2 markers.
- **Custom ROS 2 Messages:** Strongly-typed `VehicleState` and `VehicleControl` interfaces.
- 
##  Project Structure
```text
vehicle_control/
├── CMakeLists.txt          # Build configuration
├── package.xml             # Dependencies
├── msg/                    # Custom ROS 2 messages
│   ├── VehicleState.msg
│   └── VehicleControl.msg
├── src/                    # C++ Nodes
│   ├── vehicle_simulator.cpp           # Physics engine
│   ├── speed_controller.cpp            # Basic P-Controller
│   ├── scenario_controller.cpp         # Time-based state machine
│   ├── advanced_scenario_controller.cpp # Event-driven controller
│   └── visualizer.cpp                  # RViz2 3D markers
├── scripts/                # Python Nodes
│   └── plot_path.py        # 2D Matplotlib path visualizer
└── launch/                 # Launch files
      |__ vehicle_control.launch.py
      |__ advanced_scenario.launch.py 

Steps to Run:

Prerequisites
ROS 2 installed on Ubuntu/WSL2.
Python 3 with matplotlib installed (sudo apt install python3-matplotlib).

Step 1: Clone and Build
Open your terminal, navigate to your ROS 2 workspace, and build the package: 
cd ~/ros2_ws
colcon build --packages-select vehicle_control
source install/setup.bash

Step 2: Run the Simulation & Controller
Launch the vehicle simulator and the advanced event-driven controller:
ros2 launch vehicle_control advanced_scenario.launch.py

Step 3: Run the 2D Visualizer
Open a second terminal, source your workspace, and run the Python visualizer:
cd ~/ros2_ws
source install/setup.bash
python3 src/vehicle_control/scripts/plot_path.py

Step 4: Test the Scenarios
Open a third terminal to inject external events and watch how the controller and visualizer react:
1. Simulate a Red Traffic Light (Vehicle will brake to a stop):
ros2 topic pub --once /traffic_light std_msgs/msg/String "{data: 'RED'}"
2. Simulate a Green Traffic Light (Vehicle will accelerate back to 30 km/h)
ros2 topic pub --once /traffic_light std_msgs/msg/String "{data: 'GREEN'}"
3. Simulate an Emergency Obstacle (Vehicle will slam the brakes immediately):
ros2 topic pub --once /obstacle_detected std_msgs/msg/Bool "{data: true}" 
