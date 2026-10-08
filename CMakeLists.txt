cmake_minimum_required(VERSION 3.10)
project(vehicle_control)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)

# Generate the package's public ROS interfaces.
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/VehicleState.msg"
  "msg/VehicleControl.msg"
  "msg/ScenarioState.msg"
)

# Keep the common build setup in one place so adding a node stays mechanical.
function(add_vehicle_node target source)
  add_executable(${target} ${source})
  target_include_directories(${target} PRIVATE include)
  target_link_libraries(
    ${target}
    rclcpp::rclcpp
    ${PROJECT_NAME}__rosidl_typesupport_cpp
  )
  add_dependencies(${target} ${PROJECT_NAME})
endfunction()

add_vehicle_node(vehicle_simulator src/nodes/simulation/vehicle_simulator.cpp)
add_vehicle_node(speed_controller src/nodes/control/speed_controller.cpp)
add_vehicle_node(scenario_controller src/nodes/control/scenario_controller.cpp)
add_vehicle_node(
  advanced_scenario_controller
  src/nodes/control/advanced_scenario_controller.cpp
)
add_vehicle_node(auto_scenario_manager src/nodes/scenario/auto_scenario_manager.cpp)
add_vehicle_node(experiment_logger src/nodes/observability/experiment_logger.cpp)
add_vehicle_node(visualizer src/nodes/visualization/visualizer.cpp)
target_link_libraries(
  visualizer
  visualization_msgs::visualization_msgs
  geometry_msgs::geometry_msgs
)

install(TARGETS
  vehicle_simulator
  speed_controller
  scenario_controller
  advanced_scenario_controller
  visualizer
  auto_scenario_manager
  experiment_logger
  DESTINATION lib/${PROJECT_NAME}
)

# Keep runtime compositions and settings available after installation.
install(DIRECTORY
  launch
  config
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
