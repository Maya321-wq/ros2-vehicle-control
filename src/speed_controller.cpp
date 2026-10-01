#include <algorithm>
#include <functional>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float32_multi_array.hpp"

using std::placeholders::_1;

class SpeedController : public rclcpp::Node
{
public:
    SpeedController()
        : Node("speed_controller")
    {
        // Declare and read the target speed parameter
        this->declare_parameter<double>("target_speed", 30.0);
        target_speed_kmh_ = this->get_parameter("target_speed").as_double();

        // Subscribe to the full vehicle state (not just speed)
        state_subscription_ =
            this->create_subscription<std_msgs::msg::Float32MultiArray>(
                "/vehicle_state",
                10,
                std::bind(&SpeedController::state_callback, this, _1));

        // Publish control commands
        control_publisher_ =
            this->create_publisher<std_msgs::msg::Float32MultiArray>(
                "/vehicle_control", 10);

        RCLCPP_INFO(
            this->get_logger(),
            "Speed Controller started. Target speed: %.1f km/h",
            target_speed_kmh_);
    }

private:
    void state_callback(
        const std_msgs::msg::Float32MultiArray::SharedPtr msg)
    {
        if (msg->data.size() < 1) {
            RCLCPP_WARN(this->get_logger(), "State message must contain at least 1 value.");
            return;
        }

        // Extract speed from the first element of the state array
        const double current_speed_kmh = msg->data[0];
        const double error = target_speed_kmh_ - current_speed_kmh;

        float throttle = 0.0f;
        float brake = 0.0f;
        float steering = 0.0f;  // Not used yet, but included for structure

        if (error > 0.5) {
            throttle = static_cast<float>(
                std::clamp(0.04 * error, 0.0, 0.6));
        }
        else if (error < -0.5) {
            brake = static_cast<float>(
                std::clamp(0.08 * (-error), 0.0, 0.5));
        }

        std_msgs::msg::Float32MultiArray control_message;
        control_message.data = {throttle, brake, steering};
        control_publisher_->publish(control_message);

        RCLCPP_INFO(
            this->get_logger(),
            "Current: %.2f km/h | Target: %.2f | Error: %.2f | Throttle: %.2f | Brake: %.2f",
            current_speed_kmh,
            target_speed_kmh_,
            error,
            throttle,
            brake);
    }

    double target_speed_kmh_;

    rclcpp::Subscription<std_msgs::msg::Float32MultiArray>::SharedPtr state_subscription_;
    rclcpp::Publisher<std_msgs::msg::Float32MultiArray>::SharedPtr control_publisher_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SpeedController>());
    rclcpp::shutdown();
    return 0;
}
