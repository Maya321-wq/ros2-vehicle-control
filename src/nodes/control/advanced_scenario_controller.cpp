#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "vehicle_control/msg/vehicle_state.hpp"
#include "vehicle_control/msg/vehicle_control.hpp"
#include "vehicle_control/msg/scenario_state.hpp"

using std::placeholders::_1;

class AdvancedScenarioController : public rclcpp::Node
{
public:
    AdvancedScenarioController()
        : Node("advanced_scenario_controller"),
          current_speed_(0.0),
          traffic_light_state_(0), // 0=GREEN, 1=YELLOW, 2=RED
          obstacle_detected_(false),
          target_steering_(0.0f)
    {
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

        state_subscription_ = this->create_subscription<vehicle_control::msg::VehicleState>(
            "/vehicle_state", 10, std::bind(&AdvancedScenarioController::state_callback, this, _1));

        scenario_subscription_ = this->create_subscription<vehicle_control::msg::ScenarioState>(
            "/scenario_state", 10, std::bind(&AdvancedScenarioController::scenario_callback, this, _1));

        control_publisher_ = this->create_publisher<vehicle_control::msg::VehicleControl>(
            "/vehicle_control", 10);

        control_timer_ = this->create_wall_timer(
            std::chrono::milliseconds(100),
            std::bind(&AdvancedScenarioController::run_control, this));

        RCLCPP_INFO(this->get_logger(), "Advanced Scenario Controller started. Target: %.1f km/h", target_speed_);
    }

private:
    void state_callback(const vehicle_control::msg::VehicleState::SharedPtr msg)
    {
        current_speed_ = msg->speed;
    }

    void scenario_callback(const vehicle_control::msg::ScenarioState::SharedPtr msg)
    {
        traffic_light_state_ = msg->traffic_light;
        obstacle_detected_ = msg->obstacle_detected;
        target_steering_ = msg->target_steering; // <--- NEW: Read steering from scenario
    }

    void run_control()
    {
        float throttle = 0.0f;
        float brake = 0.0f;
        float steering = target_steering_; // <--- NEW: Use the scenario's steering

        // EMERGENCY BRAKING (HIGHEST PRIORITY)
        if (obstacle_detected_) {
            brake = static_cast<float>(emergency_brake_value_);
            throttle = 0.0f;
            steering = 0.0f; // Straighten out during emergency
            
            vehicle_control::msg::VehicleControl control_msg;
            control_msg.throttle = throttle;
            control_msg.brake = brake;
            control_msg.steering = steering;
            control_publisher_->publish(control_msg);
            return;
        }

        // TRAFFIC LIGHT LOGIC
        double current_target = target_speed_;
        if (traffic_light_state_ == 2) { // RED
            current_target = 0.0;
        } else if (traffic_light_state_ == 1) { // YELLOW
            current_target = yellow_speed_;
        }

        // LONGITUDINAL CONTROL (P-Controller)
        double error = current_target - current_speed_;

        if (error > 0.5) {
            throttle = static_cast<float>(std::clamp(kp_throttle_ * error, 0.0, 0.8));
        } else if (error < -0.5) {
            brake = static_cast<float>(std::clamp(kp_brake_ * (-error), 0.0, 0.8));
        }

        vehicle_control::msg::VehicleControl control_msg;
        control_msg.throttle = throttle;
        control_msg.brake = brake;
        control_msg.steering = steering;
        control_publisher_->publish(control_msg);
    }

    double target_speed_;
    double yellow_speed_;
    double kp_throttle_;
    double kp_brake_;
    double emergency_brake_value_;

    double current_speed_;
    uint8_t traffic_light_state_;
    bool obstacle_detected_;
    float target_steering_; // <--- NEW

    rclcpp::Subscription<vehicle_control::msg::VehicleState>::SharedPtr state_subscription_;
    rclcpp::Subscription<vehicle_control::msg::ScenarioState>::SharedPtr scenario_subscription_;
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
