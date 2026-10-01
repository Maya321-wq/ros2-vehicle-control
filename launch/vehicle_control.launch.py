from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        
        # Vehicle Simulator
        Node(
            package='vehicle_control',
            executable='vehicle_simulator',
            name='vehicle_simulator',
            output='screen'
        ),
        
        # Scenario Controller (replaces the simple speed_controller)
        Node(
            package='vehicle_control',
            executable='scenario_controller',
            name='scenario_controller',
            output='screen',
            parameters=[
                {'target_speed': 30.0},
                {'kp_throttle': 0.04},
                {'kp_brake': 0.08},
                {'max_cruise_time': 10.0},
                {'stop_time': 15.0}
            ]
        )
        
    ])
