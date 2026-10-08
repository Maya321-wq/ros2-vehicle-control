from launch import LaunchDescription
from ament_index_python.packages import get_package_share_directory
from launch_ros.actions import Node
import os

def generate_launch_description():
    controller_parameters = os.path.join(
        get_package_share_directory('vehicle_control'),
        'config',
        'controller.yaml',
    )
    return LaunchDescription([
        # 1. The Physics Engine
        Node(
            package='vehicle_control',
            executable='vehicle_simulator',
            name='vehicle_simulator',
            output='screen'
        ),
        
        # 2. The Brain (Controller)
        Node(
            package='vehicle_control',
            executable='advanced_scenario_controller',
            name='advanced_scenario_controller',
            output='screen',
            parameters=[controller_parameters]
        ),
        
        # 3. The Director (Scenario Manager)
        Node(
            package='vehicle_control',
            executable='auto_scenario_manager',
            name='auto_scenario_manager',
            output='screen'
        ),
        
        # 4. The Recorder (Logger)
        Node(
            package='vehicle_control',
            executable='experiment_logger',
            name='experiment_logger',
            output='screen'
        ),

        # Publish vehicle state as RViz markers.
        Node(
            package='vehicle_control',
            executable='visualizer',
            name='visualizer',
            output='screen'
        )
    ])
    