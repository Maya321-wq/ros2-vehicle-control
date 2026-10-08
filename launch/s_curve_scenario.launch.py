import os
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(package='vehicle_control', executable='vehicle_simulator', name='vehicle_simulator', output='screen'),
        Node(package='vehicle_control', executable='advanced_scenario_controller', name='advanced_scenario_controller', output='screen'),
        # Notice we pass the DIFFERENT parameter here!
        Node(package='vehicle_control', executable='auto_scenario_manager', name='auto_scenario_manager', 
             parameters=[{'scenario_type': 's_curve'}], output='screen'),
        Node(package='vehicle_control', executable='experiment_logger', name='experiment_logger', output='screen')
    ])
