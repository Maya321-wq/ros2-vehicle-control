#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/bool.hpp"
#include "vehicle_control/msg/vehicle_state.hpp"
#include "vehicle_control/msg/vehicle_control.hpp"

using std::placeholders::_1;

class AdvancedScenarioController : public rclcpp::Node
{
public:
    AdvancedScenarioController()
        : Node("advanced_scenario_controller"),
          current_speed_(0.0),
          traffic_light_state_("GREEN"),
          obstacle_detected_(false)
    {
        // Parameters
        this->declare_parameter<double>("target_speed", 30.0);
        this->declare_parameter<double>("yellow_speed", 15.0);
        this->declare_parameter<double>("kp_throttle", 0.04);
        this->declare_parameter<double>("kp_brake", 0.08);
        this->declare_parameter<double>("emergency_brake_value", 1.0);

        target_speed_ = this->get_parameter("target_speed").as_double();
        yellow_speed_ = this->get_parameter("yellow_speed").as_double();
        kp_throttle_ = this->get_parameter("kp_throttle").as_double();
        kp_brake_ = this->get_parameter("kp_brake").as_double();
        emergency_brake_value_ = this->get_parameter("emergency_brake_value").as_double();

        // Subscribe to vehicle state
        state_subscription_ =
            this->create_subscription<vehicle_control::msg::VehicleState>(
                "/vehicle_state", 10,
                std::bind(&AdvancedScenarioController::state_callback, this, _1));

        // Subscribe to traffic light state
        traffic_light_subscription_ =
            this->create_subscription<std_msgs::msg::String>(
                "/traffic_light", 10,
                std::bind(&AdvancedScenarioController::traffic_light_callback, this, _1));

        // Subscribe to obstacle detection
        obstacle_subscription_ =
            this->create_subscription<std_msgs::msg::Bool>(
                "/obstacle_detected", 10,
                std::bind(&AdvancedScenarioController::obstacle_callback, this, _1));

        // Publish control commands
        control_publisher_ =
            this->create_publisher<vehicle_control::msg::VehicleControl>(
                "/vehicle_control", 10);

        // Timer to run the control logic at 10 Hz
        control_timer_ = this->create_wall_timer(
            std::chrono::milliseconds(100),
            std::bind(&AdvancedScenarioController::run_control, this));

        RCLCPP_INFO(this->get_logger(),
            "Advanced Scenario Controller started. Target: %.1f km/h, Yellow: %.1f km/h",
            target_speed_, yellow_speed_);
    }

private:
    void state_callback(const vehicle_control::msg::VehicleState::SharedPtr msg)
    {
        current_speed_ = msg->speed;
    }

    void traffic_light_callback(const std_msgs::msg::String::SharedPtr msg)
    {
        traffic_light_state_ = msg->data;
        RCLCPP_INFO(this->get_logger(), "Traffic light changed to: %s", traffic_light_state_.c_str());
    }

    void obstacle_callback(const std_msgs::msg::Bool::SharedPtr msg)
    {
        obstacle_detected_ = msg->data;
        if (obstacle_detected_) {
            RCLCPP_WARN(this->get_logger(), "OBSTACLE DETECTED! Emergency braking!");
        }
    }

    void run_control()
    {
        float throttle = 0.0f;
        float brake = 0.0f;
        float steering = 0.0f;

        // EMERGENCY BRAKING (HIGHEST PRIORITY)
        if (obstacle_detected_) {
            brake = static_cast<float>(emergency_brake_value_);
            throttle = 0.0f;
            
            vehicle_control::msg::VehicleControl control_msg;
            control_msg.throttle = throttle;
            control_msg.brake = brake;
            control_msg.steering = steering;
            control_publisher_->publish(control_msg);
            
            RCLCPP_WARN(this->get_logger(),
                "EMERGENCY BRAKE | Speed: %.2f | Brake: %.2f",
                current_speed_, brake);
            return;
        }

        // TRAFFIC LIGHT LOGIC
        double current_target = target_speed_;
        
        if (traffic_light_state_ == "RED") {
            current_target = 0.0;
        } else if (traffic_light_state_ == "YELLOW") {
            current_target = yellow_speed_;
        } else {
            current_target = target_speed_;
        }

        // LONGITUDINAL CONTROL (P-Controller)
        double error = current_target - current_speed_;

        if (error > 0.5) {
            throttle = static_cast<float>(
                std::clamp(kp_throttle_ * error, 0.0, 0.8));
        } else if (error < -0.5) {
            brake = static_cast<float>(
                std::clamp(kp_brake_ * (-error), 0.0, 0.8));
        }

        vehicle_control::msg::VehicleControl control_msg;
        control_msg.throttle = throttle;
        control_msg.brake = brake;
        control_msg.steering = steering;
        control_publisher_->publish(control_msg);

        RCLCPP_INFO(this->get_logger(),
            "Light: %s | Target: %.1f | Speed: %.2f | T: %.2f | B: %.2f",
            traffic_light_state_.c_str(), current_target, current_speed_, throttle, brake);
    }

    // Parameters
    double target_speed_;
    double yellow_speed_;
    double kp_throttle_;
    double kp_brake_;
    double emergency_brake_value_;

    // State
    double current_speed_;
    std::string traffic_light_state_;
    bool obstacle_detected_;

    // ROS interfaces
    rclcpp::Subscription<vehicle_control::msg::VehicleState>::SharedPtr state_subscription_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr traffic_light_subscription_;
    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr obstacle_subscription_;
    rclcpp::Publisher<vehicle_control::msg::VehicleControl>::SharedPtr control_publisher_;
    rclcpp::TimerBase::SharedPtr control_timer_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<AdvancedScenarioController>());
    rclcpp::shutdown();
    return 0;
}
