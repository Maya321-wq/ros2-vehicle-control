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
        
        # Advanced Scenario Controller
        Node(
            package='vehicle_control',
            executable='advanced_scenario_controller',
            name='advanced_scenario_controller',
            output='screen',
            parameters=[
                {'target_speed': 30.0},
                {'yellow_speed': 15.0},
                {'kp_throttle': 0.04},
                {'kp_brake': 0.08},
                {'emergency_brake_value': 1.0}
            ]
        )
        
    ])
