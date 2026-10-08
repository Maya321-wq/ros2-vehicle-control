#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "vehicle_control/msg/scenario_state.hpp"

class AutoScenarioManager : public rclcpp::Node
{
public:
    AutoScenarioManager()
        : Node("auto_scenario_manager"),
          start_time_(this->now()),
          last_logged_phase_(-1.0)
    {
        // 1. DECLARE THE PARAMETER
        this->declare_parameter<std::string>("scenario_type", "traffic_light");
        scenario_type_ = this->get_parameter("scenario_type").as_string();

        scenario_pub_ = this->create_publisher<vehicle_control::msg::ScenarioState>(
            "/scenario_state", 10);

        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(1000),
            std::bind(&AutoScenarioManager::scenario_update_callback, this));

        RCLCPP_INFO(this->get_logger(), "Auto Scenario Manager started. Running scenario: %s", scenario_type_.c_str());
    }

private:
    void scenario_update_callback()
    {
        double elapsed = (this->now() - start_time_).seconds();

        vehicle_control::msg::ScenarioState state_msg;
        state_msg.traffic_light = 0; // Default GREEN
        state_msg.obstacle_detected = false;
        state_msg.target_steering = 0.0f; // Default Straight

        double current_phase = 0.0;

        // 2. SWITCH BETWEEN SCENARIOS BASED ON PARAMETER
        if (scenario_type_ == "s_curve") {
            // --- S-CURVE TIMELINE ---
            if (elapsed < 10.0) {
                current_phase = 1.0; // Straight
                state_msg.target_steering = 0.0f;
            } else if (elapsed < 15.0) {
                current_phase = 2.0; // Left Turn
                state_msg.target_steering = 0.5f;
            } else if (elapsed < 20.0) {
                current_phase = 3.0; // Right Turn
                state_msg.target_steering = -0.5f;
            } else {
                current_phase = 4.0; // Straight
                state_msg.target_steering = 0.0f;
            }
        } else {
            // --- TRAFFIC LIGHT & OBSTACLE TIMELINE (ORIGINAL) ---
            if (elapsed < 8.0) {
                current_phase = 1.0; 
                state_msg.traffic_light = 0; // GREEN
            } else if (elapsed < 15.0) {
                current_phase = 2.0; 
                state_msg.traffic_light = 2; // RED
            } else if (elapsed < 22.0) {
                current_phase = 3.0; 
                state_msg.traffic_light = 0; // GREEN
            } else if (elapsed < 28.0) {
                current_phase = 4.0; 
                state_msg.obstacle_detected = true;
            } else {
                current_phase = 5.0; 
                state_msg.obstacle_detected = false;
            }
        }

        // 3. LOGGING
        if (current_phase != last_logged_phase_) {
            RCLCPP_INFO(this->get_logger(), 
                ">>> [%s] Time=%.1fs | Light=%d | Obstacle=%s | Steering=%.2f", 
                scenario_type_.c_str(), elapsed, state_msg.traffic_light, 
                state_msg.obstacle_detected ? "TRUE" : "FALSE", state_msg.target_steering);
            last_logged_phase_ = current_phase;
        }

        scenario_pub_->publish(state_msg);
    }

    std::string scenario_type_;
    rclcpp::Time start_time_;
    double last_logged_phase_;
    rclcpp::Publisher<vehicle_control::msg::ScenarioState>::SharedPtr scenario_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<AutoScenarioManager>());
    rclcpp::shutdown();
    return 0;
}
