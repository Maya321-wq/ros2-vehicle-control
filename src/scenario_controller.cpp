#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "vehicle_control/msg/vehicle_state.hpp"
#include "vehicle_control/msg/vehicle_control.hpp"

using std::placeholders::_1;

class ScenarioController : public rclcpp::Node
{
public:
    ScenarioController()
        : Node("scenario_controller"),
          current_speed_(0.0),
          scenario_phase_("ACCELERATE"),
          steering_phase_("STRAIGHT"),
          scenario_start_time_(this->now())
    {
        // ===== TUNABLE PARAMETERS =====
        this->declare_parameter<double>("target_speed", 30.0);
        this->declare_parameter<double>("kp_throttle", 0.04);
        this->declare_parameter<double>("kp_brake", 0.08);
        this->declare_parameter<double>("max_cruise_time", 10.0);
        this->declare_parameter<double>("stop_time", 15.0);

        target_speed_ = this->get_parameter("target_speed").as_double();
        kp_throttle_ = this->get_parameter("kp_throttle").as_double();
        kp_brake_ = this->get_parameter("kp_brake").as_double();
        max_cruise_time_ = this->get_parameter("max_cruise_time").as_double();
        stop_time_ = this->get_parameter("stop_time").as_double();

        // Subscribe to vehicle state
        state_subscription_ =
            this->create_subscription<vehicle_control::msg::VehicleState>(
                "/vehicle_state", 10,
                std::bind(&ScenarioController::state_callback, this, _1));

        // Publish control commands
        control_publisher_ =
            this->create_publisher<vehicle_control::msg::VehicleControl>(
                "/vehicle_control", 10);

        // Timer to run the scenario logic at 10 Hz
        scenario_timer_ = this->create_wall_timer(
            std::chrono::milliseconds(100),
            std::bind(&ScenarioController::run_scenario, this));

        RCLCPP_INFO(this->get_logger(),
            "Scenario Controller started. Target: %.1f km/h, Kp_t: %.2f, Kp_b: %.2f",
            target_speed_, kp_throttle_, kp_brake_);
    }

private:
    void state_callback(const vehicle_control::msg::VehicleState::SharedPtr msg)
    {
        current_speed_ = msg->speed;
    }

    void run_scenario()
    {
        double elapsed = (this->now() - scenario_start_time_).seconds();

        float throttle = 0.0f;
        float brake = 0.0f;
        float steering = 0.0f;

        // ===== LONGITUDINAL SCENARIO =====
        if (elapsed < 5.0) {
            scenario_phase_ = "ACCELERATE";
            double error = target_speed_ - current_speed_;
            if (error > 0.5) {
                throttle = static_cast<float>(
                    std::clamp(kp_throttle_ * error, 0.0, 0.8));
            }
        }
        else if (elapsed < max_cruise_time_) {
            scenario_phase_ = "CRUISE";
            double error = target_speed_ - current_speed_;
            if (error > 0.5) {
                throttle = static_cast<float>(
                    std::clamp(kp_throttle_ * error, 0.0, 0.6));
            } else if (error < -0.5) {
                brake = static_cast<float>(
                    std::clamp(kp_brake_ * (-error), 0.0, 0.5));
            }
        }
        else if (elapsed < stop_time_) {
            scenario_phase_ = "BRAKE";
            brake = 0.8f;
        }
        else {
            scenario_phase_ = "STOPPED";
            brake = 1.0f;
            throttle = 0.0f;
        }

        // ===== STEERING SCENARIO =====
        if (elapsed >= 6.0 && elapsed < 8.0) {
            steering_phase_ = "TURN_LEFT";
            steering = 0.5f;
        } else if (elapsed >= 8.0 && elapsed < 10.0) {
            steering_phase_ = "STRAIGHT";
            steering = 0.0f;
        } else if (elapsed >= 10.0 && elapsed < 12.0) {
            steering_phase_ = "TURN_RIGHT";
            steering = -0.5f;
        } else {
            steering_phase_ = "STRAIGHT";
            steering = 0.0f;
        }

        // Publish control command
        vehicle_control::msg::VehicleControl control_msg;
        control_msg.throttle = throttle;
        control_msg.brake = brake;
        control_msg.steering = steering;
        control_publisher_->publish(control_msg);

        RCLCPP_INFO(this->get_logger(),
            "Time: %.1fs | Phase: %s | Steer: %s | Speed: %.2f | T: %.2f | B: %.2f | S: %.2f",
            elapsed, scenario_phase_.c_str(), steering_phase_.c_str(),
            current_speed_, throttle, brake, steering);
    }

    // Parameters
    double target_speed_;
    double kp_throttle_;
    double kp_brake_;
    double max_cruise_time_;
    double stop_time_;

    // State (Order matches initializer list!)
    double current_speed_;
    std::string scenario_phase_;
    std::string steering_phase_;
    rclcpp::Time scenario_start_time_;

    // ROS interfaces
    rclcpp::Subscription<vehicle_control::msg::VehicleState>::SharedPtr state_subscription_;
    rclcpp::Publisher<vehicle_control::msg::VehicleControl>::SharedPtr control_publisher_;
    rclcpp::TimerBase::SharedPtr scenario_timer_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ScenarioController>());
    rclcpp::shutdown();
    return 0;
}
