cmake_minimum_required(VERSION 3.8)
project(vehicle_control)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)

# 1. Generate custom messages
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/VehicleState.msg"
  "msg/VehicleControl.msg"
  DEPENDENCIES std_msgs
)

# 2. Executable 1: Vehicle Simulator
add_executable(vehicle_simulator src/vehicle_simulator.cpp)
target_link_libraries(vehicle_simulator rclcpp::rclcpp std_msgs::std_msgs)
add_dependencies(vehicle_simulator ${PROJECT_NAME})
target_link_libraries(vehicle_simulator ${PROJECT_NAME}__rosidl_typesupport_cpp)

# 3. Executable 2: Speed Controller
add_executable(speed_controller src/speed_controller.cpp)
target_link_libraries(speed_controller rclcpp::rclcpp std_msgs::std_msgs)
add_dependencies(speed_controller ${PROJECT_NAME})
target_link_libraries(speed_controller ${PROJECT_NAME}__rosidl_typesupport_cpp)

# 4. Executable 3: Scenario Controller
add_executable(scenario_controller src/scenario_controller.cpp)
target_link_libraries(scenario_controller rclcpp::rclcpp std_msgs::std_msgs)
add_dependencies(scenario_controller ${PROJECT_NAME})
target_link_libraries(scenario_controller ${PROJECT_NAME}__rosidl_typesupport_cpp)

# 5. Executable 4: Advanced Scenario Controller
add_executable(advanced_scenario_controller src/advanced_scenario_controller.cpp)
target_link_libraries(advanced_scenario_controller rclcpp::rclcpp std_msgs::std_msgs)
add_dependencies(advanced_scenario_controller ${PROJECT_NAME})
target_link_libraries(advanced_scenario_controller ${PROJECT_NAME}__rosidl_typesupport_cpp)

# 6. Executable 5: Visualizer (for RViz2)
add_executable(visualizer src/visualizer.cpp)
target_link_libraries(visualizer rclcpp::rclcpp visualization_msgs::visualization_msgs geometry_msgs::geometry_msgs)
add_dependencies(visualizer ${PROJECT_NAME})
target_link_libraries(visualizer ${PROJECT_NAME}__rosidl_typesupport_cpp)

# 7. Install all executables
install(TARGETS
  vehicle_simulator
  speed_controller
  scenario_controller
  advanced_scenario_controller
  visualizer
  DESTINATION lib/${PROJECT_NAME}
)

# 8. Install the launch directory
install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
