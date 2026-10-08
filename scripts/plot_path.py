import rclpy
from rclpy.node import Node
from vehicle_control.msg import VehicleState
import matplotlib.pyplot as plt

class PathPlotter(Node):
    def __init__(self):
        super().__init__('path_plotter')
        self.subscription = self.create_subscription(
            VehicleState, 'vehicle_state', self.state_callback, 10)
        
        self.x_data = []
        self.y_data = []
        
        # Setup matplotlib
        plt.ion()  # Interactive mode
        self.fig, self.ax = plt.subplots()
        self.line, = self.ax.plot([], [], 'b.-', label='Vehicle Path')
        self.car_dot, = self.ax.plot([], [], 'ro', markersize=8, label='Current Position')
        self.ax.set_xlabel('Position X (m)')
        self.ax.set_ylabel('Position Y (m)')
        self.ax.set_title('Vehicle 2D Path (CPU Rendered)')
        self.ax.legend()
        self.ax.grid(True)

    def state_callback(self, msg):
        self.x_data.append(msg.position_x)
        self.y_data.append(msg.position_y)
        
        # Keep only the last 200 points for performance
        if len(self.x_data) > 200:
            self.x_data.pop(0)
            self.y_data.pop(0)
            
        # Update plot
        self.line.set_data(self.x_data, self.y_data)
        self.car_dot.set_data([msg.position_x], [msg.position_y])
        
        # Auto-scale the view
        self.ax.relim()
        self.ax.autoscale_view()
        plt.pause(0.1)

def main():
    rclpy.init()
    plotter = PathPlotter()
    try:
        rclpy.spin(plotter)
    except KeyboardInterrupt:
        pass
    plotter.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
