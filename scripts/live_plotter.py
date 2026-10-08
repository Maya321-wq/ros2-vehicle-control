#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from vehicle_control.msg import VehicleState, ScenarioState
import matplotlib.pyplot as plt
import datetime

class LivePlotter(Node):
    def __init__(self):
        super().__init__('live_plotter')
        
        self.time_data = []
        self.speed_data = []
        self.steering_data = []
        self.start_time = self.get_clock().now()
        self.update_counter = 0

        self.prev_light = -1
        self.prev_obstacle = False
        self.drawn_red = False
        self.drawn_green = False
        self.drawn_obs = False

        self.state_sub = self.create_subscription(VehicleState, '/vehicle_state', self.state_callback, 10)
        self.scenario_sub = self.create_subscription(ScenarioState, '/scenario_state', self.scenario_callback, 10)

        plt.ion() 
        self.fig, (self.ax1, self.ax2) = plt.subplots(2, 1, figsize=(12, 8), sharex=True)
        
        self.line_speed, = self.ax1.plot([], [], 'b-', linewidth=2, label='Speed (km/h)')
        self.ax1.axhline(y=30.0, color='green', linestyle='--', label='Target Speed')
        self.ax1.set_ylabel('Speed (km/h)')
        self.ax1.set_title('Live Autonomous Vehicle Scenario')
        self.ax1.legend(loc='upper right')
        self.ax1.grid(True, linestyle=':', alpha=0.7)

        self.line_steering, = self.ax2.plot([], [], 'orange', linewidth=2, label='Steering')
        self.ax2.axhline(y=0.0, color='black', linestyle='-', linewidth=0.5)
        self.ax2.set_ylabel('Steering (-1.0 to 1.0)')
        self.ax2.set_xlabel('Time (seconds)')
        self.ax2.legend(loc='upper right')
        self.ax2.grid(True, linestyle=':', alpha=0.7)

        self.get_logger().info("Live Plotter started! Will auto-save with a unique timestamp.")

    def state_callback(self, msg):
        current_time = (self.get_clock().now() - self.start_time).nanoseconds / 1e9
        self.time_data.append(current_time)
        self.speed_data.append(msg.speed)
        self.steering_data.append(msg.steering)
        
        # AUTO-SAVE TRIGGER
        if current_time > 35.0:
            self.get_logger().info("Scenario complete! Saving final graph and exiting...")
            self.save_final_plot()
            self.destroy_node()
            rclpy.shutdown()
            return

        self.update_plot()

    def scenario_callback(self, msg):
        current_time = (self.get_clock().now() - self.start_time).nanoseconds / 1e9

        if msg.traffic_light != self.prev_light:
            if msg.traffic_light == 2: 
                self.ax1.axvline(x=current_time, color='red', linestyle='--', alpha=0.8, label='RED Light' if not self.drawn_red else "")
                self.drawn_red = True
            elif msg.traffic_light == 0: 
                self.ax1.axvline(x=current_time, color='green', linestyle='--', alpha=0.5, label='GREEN Light' if not self.drawn_green else "")
                self.drawn_green = True
            self.prev_light = msg.traffic_light

        if msg.obstacle_detected and not self.prev_obstacle:
            self.ax1.axvline(x=current_time, color='black', linestyle=':', linewidth=3, label='Obstacle Detected' if not self.drawn_obs else "")
            self.drawn_obs = True
            self.prev_obstacle = True
        elif not msg.obstacle_detected:
            self.prev_obstacle = False

    def update_plot(self):
        self.update_counter += 1
        if self.update_counter % 5 == 0:
            self.line_speed.set_data(self.time_data, self.speed_data)
            self.line_steering.set_data(self.time_data, self.steering_data)
            self.ax1.relim()
            self.ax1.autoscale_view()
            self.ax2.relim()
            self.ax2.autoscale_view()
            self.fig.canvas.draw()
            self.fig.canvas.flush_events()

    def save_final_plot(self):
        # Generate a unique filename using the current date and time!
        timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        output_path = f"/home/maya_akl/ros2_ws/results/live_experiment_{timestamp}.png"
        
        self.fig.savefig(output_path, dpi=300)
        self.get_logger().info(f"SUCCESS! Graph automatically saved to: {output_path}")
        plt.close(self.fig)

def main(args=None):
    rclpy.init(args=args)
    plotter = LivePlotter()
    try:
        rclpy.spin(plotter)
    except KeyboardInterrupt:
        plotter.save_final_plot()
        plotter.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()
