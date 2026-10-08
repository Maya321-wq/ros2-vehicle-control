#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "vehicle_control/topics.hpp"
#include "vehicle_control/msg/vehicle_state.hpp"
#include "vehicle_control/msg/vehicle_control.hpp"

using std::placeholders::_1;

class VehicleSimulator : public rclcpp::Node
{
public:
    VehicleSimulator()
        : Node("vehicle_simulator"),
          speed_kmh_(0.0),
          position_x_(0.0),
          position_y_(0.0),
          yaw_(0.0),
          throttle_(0.0),
          brake_(0.0),
          steering_(0.0)
    {
        // Publish the full vehicle state using custom message
        state_publisher_ =
            this->create_publisher<vehicle_control::msg::VehicleState>(
                vehicle_control::topics::kVehicleState, 10);

        // Receive control commands using custom message (FIXED!)
        control_subscription_ =
            this->create_subscription<vehicle_control::msg::VehicleControl>(
                vehicle_control::topics::kVehicleControl,
                10,
                std::bind(&VehicleSimulator::control_callback, this, _1));

        // Update the simulated vehicle every 0.1 seconds
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(100),
            std::bind(&VehicleSimulator::update_vehicle, this));

        RCLCPP_DEBUG(
            this->get_logger(),
            "Vehicle Simulator started. Initial state: speed=0, pos=(0,0), yaw=0");
    }

private:
    void control_callback(
        const vehicle_control::msg::VehicleControl::SharedPtr msg)
    {
        throttle_ = std::clamp(msg->throttle, 0.0f, 1.0f);
        brake_ = std::clamp(msg->brake, 0.0f, 1.0f);
        steering_ = std::clamp(msg->steering, -1.0f, 1.0f);
    }

    void update_vehicle()
    {
        const double dt = 0.1;  // 100 ms

        // ===== SPEED DYNAMICS =====
        const double acceleration = 12.0 * throttle_;
        const double deceleration = 20.0 * brake_;
        const double natural_drag = 0.3;

        speed_kmh_ += (acceleration - deceleration - natural_drag) * dt;
        speed_kmh_ = std::clamp(speed_kmh_, 0.0, 80.0);

        // Convert km/h to m/s for position calculation
        const double speed_ms = speed_kmh_ / 3.6;

        // ===== YAW DYNAMICS =====
        const double max_yaw_rate = 0.5;
        yaw_ += steering_ * max_yaw_rate * dt;

        // Keep yaw in range [-pi, pi]
        yaw_ = std::fmod(yaw_ + M_PI, 2 * M_PI);
        if (yaw_ < 0) yaw_ += 2 * M_PI;
        yaw_ -= M_PI;

        // ===== POSITION DYNAMICS =====
        position_x_ += speed_ms * std::cos(yaw_) * dt;
        position_y_ += speed_ms * std::sin(yaw_) * dt;

        // ===== PUBLISH VEHICLE STATE =====
        vehicle_control::msg::VehicleState state_message;
        state_message.speed = static_cast<float>(speed_kmh_);
        state_message.position_x = static_cast<float>(position_x_);
        state_message.position_y = static_cast<float>(position_y_);
        state_message.yaw = static_cast<float>(yaw_);
        state_message.steering = static_cast<float>(steering_);

        state_publisher_->publish(state_message);

        RCLCPP_DEBUG(
            this->get_logger(),
            "Speed: %.2f km/h | Pos: (%.2f, %.2f) m | Yaw: %.2f rad | Steer: %.2f",
            speed_kmh_, position_x_, position_y_, yaw_, steering_);
    }

    // Vehicle state variables
    double speed_kmh_;
    double position_x_;
    double position_y_;
    double yaw_;

    // Control inputs
    float throttle_;
    float brake_;
    float steering_;

    rclcpp::Publisher<vehicle_control::msg::VehicleState>::SharedPtr state_publisher_;
    rclcpp::Subscription<vehicle_control::msg::VehicleControl>::SharedPtr control_subscription_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<VehicleSimulator>());
    rclcpp::shutdown();
    return 0;
}
