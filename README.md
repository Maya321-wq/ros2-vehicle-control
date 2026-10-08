# Vehicle Control (ROS 2)

A ROS 2 demo of a simulated vehicle, controllers, scripted scenarios, CSV
logging, and Matplotlib plots. This is a simple teaching simulation, not
software for controlling a real vehicle.

## Project structure

```text
vehicle_control/
├── CMakeLists.txt                 # Generates messages and builds C++ nodes
├── package.xml                    # Package dependencies
├── config/
│   └── controller.yaml            # Advanced controller defaults
├── include/vehicle_control/
│   └── topics.hpp                 # Shared topic-name constants
├── launch/
│   ├── advanced_scenario.launch.py
│   ├── full_scenario.launch.py
│   ├── s_curve_scenario.launch.py
│   ├── traffic_light_scenario.launch.py
│   ├── vehicle_control.launch.py
│   └── visualization.launch.py    # Launch compositions
├── msg/
│   ├── ScenarioState.msg
│   ├── VehicleControl.msg
│   └── VehicleState.msg           # ROS message interfaces
├── scripts/
│   ├── live_plotter.py
│   ├── plot_path.py
│   ├── plot_results.py
│   └── plot_s_curve.py            # Matplotlib live/offline plotting
└── src/nodes/
    ├── control/
    │   ├── advanced_scenario_controller.cpp
    │   ├── scenario_controller.cpp
    │   └── speed_controller.cpp
    ├── observability/
    │   └── experiment_logger.cpp
    ├── scenario/
    │   └── auto_scenario_manager.cpp
    ├── simulation/
    │   └── vehicle_simulator.cpp
    └── visualization/
        └── visualizer.cpp
```

## What the files do

### ROS nodes

| File | Function |
| --- | --- |
| `src/nodes/simulation/vehicle_simulator.cpp` | Simulates speed, position, yaw, and steering. Reads control commands and publishes vehicle state at 10 Hz. |
| `src/nodes/control/speed_controller.cpp` | Basic target-speed proportional controller. |
| `src/nodes/control/scenario_controller.cpp` | Time-based accelerate/cruise/brake controller with scripted turns. |
| `src/nodes/control/advanced_scenario_controller.cpp` | Controls speed from scenario events, brakes for red lights, reduces speed for yellow, and performs emergency braking for obstacles. |
| `src/nodes/scenario/auto_scenario_manager.cpp` | Publishes a traffic-light/obstacle scenario or an S-curve steering scenario once per second. |
| `src/nodes/observability/experiment_logger.cpp` | Records the latest vehicle, control, and scenario values to a CSV file on each state update. |
| `src/nodes/visualization/visualizer.cpp` | Intended to publish RViz markers. Its implementation is currently commented out. |

Run only **one controller** for a simulator. All controllers publish on the same
control topic.

### Other package files

| File/folder | Function |
| --- | --- |
| `msg/VehicleState.msg` | Vehicle speed (km/h), position (m), yaw (radians), and steering. |
| `msg/VehicleControl.msg` | Throttle, brake, and steering commands. |
| `msg/ScenarioState.msg` | Traffic-light state, obstacle flag, and target steering. Light values are GREEN=0, YELLOW=1, RED=2. |
| `include/vehicle_control/topics.hpp` | Common names for state, control, scenario, and marker topics. |
| `config/controller.yaml` | Defaults for the advanced controller: target speed, yellow-light speed, gains, and emergency brake. |
| `launch/*.launch.py` | Selects which ROS nodes and scenario to start. |
| `scripts/plot_path.py` | Displays a live 2D vehicle path. |
| `scripts/live_plotter.py` | Displays live speed and steering; saves a timestamped plot after 35 seconds or when interrupted with Ctrl+C. |
| `scripts/plot_results.py` | Creates a speed plot with traffic-light and obstacle intervals from CSV. |
| `scripts/plot_s_curve.py` | Creates speed and steering plots from CSV. |
| `CMakeLists.txt` | Generates ROS message code, builds and installs C++ nodes, and installs launch/config files. |
| `package.xml` | Declares build and runtime dependencies. |

## How the nodes communicate

```text
auto_scenario_manager ── ScenarioState ──> advanced_scenario_controller
vehicle_simulator <───── VehicleControl ── advanced_scenario_controller
       │
       └── VehicleState ──> experiment_logger ──> CSV
                        ├─> Matplotlib scripts
                        └─> visualizer ──> RViz markers (disabled)
```

| Topic | Message | Used for |
| --- | --- | --- |
| `vehicle_state` | `vehicle_control/msg/VehicleState` | Simulator output for controllers, logger, and plots. |
| `vehicle_control` | `vehicle_control/msg/VehicleControl` | Controller output to the simulator and logger. |
| `scenario_state` | `vehicle_control/msg/ScenarioState` | Scenario-manager input to the advanced controller and logger. |
| `vehicle_markers` | `visualization_msgs/msg/Marker` | Optional RViz output from the visualizer. |

The message fields and topic names are defined in `msg/` and
`include/vehicle_control/topics.hpp`.

## Build

From the workspace root:

```bash
cd ~/ros2_ws
source /opt/ros/<ros-distro>/setup.bash
MAKEFLAGS=-j1 colcon build --packages-select vehicle_control \
  --executor sequential --parallel-workers 1
source install/setup.bash
```

Replace `<ros-distro>` with the ROS 2 distribution installed on your system.
The single-worker build is recommended on memory-limited machines.

> **Current RViz limitation:** `visualizer.cpp` is fully commented out, but
> `CMakeLists.txt` still declares it as an executable and `full_scenario` and
> `visualization` launch files still try to start it. The RViz node is not
> currently usable. Use the traffic-light or S-curve launch below for
> Matplotlib-based testing. If the build fails while linking `visualizer`,
> restore its implementation or remove that target and its launch actions.

## Run the simulation with Matplotlib

### Traffic-light and obstacle scenario

Terminal 1 — start the simulator, advanced controller, scenario manager, and
CSV logger:

```bash
cd ~/ros2_ws
source /opt/ros/<ros-distro>/setup.bash
source install/setup.bash
ros2 launch vehicle_control traffic_light_scenario.launch.py
```

Terminal 2 — start the live speed/steering plot:

```bash
cd ~/ros2_ws
source /opt/ros/<ros-distro>/setup.bash
source install/setup.bash
mkdir -p results
python3 src/vehicle_control/scripts/live_plotter.py
```

The plotter saves a timestamped image in `~/ros2_ws/results/` after 35 seconds.
Press Ctrl+C to stop it sooner and save the current plot.

### S-curve scenario

Use the same two-terminal setup, replacing the launch command with:

```bash
ros2 launch vehicle_control s_curve_scenario.launch.py
```

The manager commands straight steering, then left, right, and straight again.

### Other launch files

| Command | Starts |
| --- | --- |
| `ros2 launch vehicle_control advanced_scenario.launch.py` | Simulator, advanced controller, and default traffic-light scenario manager. |
| `ros2 launch vehicle_control vehicle_control.launch.py` | Simulator and timed scenario controller. |
| `ros2 launch vehicle_control full_scenario.launch.py` | Full composition including logger and the currently disabled RViz visualizer. |
| `ros2 launch vehicle_control visualization.launch.py` | Simulator, advanced controller, scenario manager, and the currently disabled RViz visualizer. |

## Check that the program is running

While a scenario launch is active, use another sourced terminal:

```bash
ros2 topic list
ros2 topic echo /vehicle_state --once
ros2 topic echo /scenario_state --once
```

The state topic should report changing speed/position, and the scenario topic
should report traffic-light, obstacle, and steering values. You can also inspect
available nodes with:

```bash
ros2 node list
```

## Save and plot experiment data

The logger appends to `experiment_data.csv` in its current working directory.
When launched from `~/ros2_ws`, this is normally `~/ros2_ws/experiment_data.csv`.
The offline plotting scripts currently expect the CSV at
`~/ros2_ws/results/experiment_data.csv`. After stopping the logger, copy the
CSV there:

```bash
mkdir -p ~/ros2_ws/results
cp ~/ros2_ws/experiment_data.csv ~/ros2_ws/results/experiment_data.csv
```

Then run one of the plots from the workspace root:

```bash
python3 src/vehicle_control/scripts/plot_results.py
python3 src/vehicle_control/scripts/plot_s_curve.py
```

The scripts save images under `~/ros2_ws/results/`. The logger appends rather
than replacing data, so move or rename an old CSV before a fresh experiment if
you want a clean dataset.

## Tests

There are currently no dedicated automated test files in this package. Run the
package test command to execute any tests registered with colcon:

```bash
cd ~/ros2_ws
source /opt/ros/<ros-distro>/setup.bash
source install/setup.bash
colcon test --packages-select vehicle_control
colcon test-result --verbose
```

For an end-to-end manual test, build the package, run the traffic-light or
S-curve launch, start `live_plotter.py` in a second terminal, and check that
`/vehicle_state` and `/scenario_state` publish as shown above.
