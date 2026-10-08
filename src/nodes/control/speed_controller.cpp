#include <algorithm>
#include <functional>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "vehicle_control/topics.hpp"
#include "vehicle_control/msg/vehicle_state.hpp"
#include "vehicle_control/msg/vehicle_control.hpp"

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

        // Subscribe to the same typed vehicle state used by the simulator.
        state_subscription_ =
            this->create_subscription<vehicle_control::msg::VehicleState>(
                vehicle_control::topics::kVehicleState,
                10,
                std::bind(&SpeedController::state_callback, this, _1));

        // Publish control commands
        control_publisher_ =
            this->create_publisher<vehicle_control::msg::VehicleControl>(
                vehicle_control::topics::kVehicleControl, 10);

        RCLCPP_INFO(
            this->get_logger(),
            "Speed Controller started. Target speed: %.1f km/h",
            target_speed_kmh_);
    }

private:
    void state_callback(
        const vehicle_control::msg::VehicleState::SharedPtr msg)
    {
        const double current_speed_kmh = msg->speed;
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

        vehicle_control::msg::VehicleControl control_message;
        control_message.throttle = throttle;
        control_message.brake = brake;
        control_message.steering = steering;
        control_publisher_->publish(control_message);

        RCLCPP_DEBUG(
            this->get_logger(),
            "Current: %.2f km/h | Target: %.2f | Error: %.2f | Throttle: %.2f | Brake: %.2f",
            current_speed_kmh,
            target_speed_kmh_,
            error,
            throttle,
            brake);
    }

    double target_speed_kmh_;

    rclcpp::Subscription<vehicle_control::msg::VehicleState>::SharedPtr state_subscription_;
    rclcpp::Publisher<vehicle_control::msg::VehicleControl>::SharedPtr control_publisher_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SpeedController>());
    rclcpp::shutdown();
    return 0;
}
