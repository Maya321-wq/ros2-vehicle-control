#include <chrono>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iomanip>
#include <memory>
#include <stdexcept>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "vehicle_control/topics.hpp"
#include "vehicle_control/msg/scenario_state.hpp"
#include "vehicle_control/msg/vehicle_state.hpp"
#include "vehicle_control/msg/vehicle_control.hpp"

using std::placeholders::_1;

class ExperimentLogger : public rclcpp::Node
{
public:
    ExperimentLogger()
        : Node("experiment_logger"),
          current_speed_(0.0),
          throttle_(0.0),
          brake_(0.0),
          steering_(0.0),
          light_state_(vehicle_control::msg::ScenarioState::GREEN),
          obstacle_(false)
    {
        const std::string output_file =
            this->declare_parameter<std::string>("output_file", "experiment_data.csv");
        file_.open(output_file, std::ios::out | std::ios::app);
        if (!file_.is_open()) {
            const std::string error = "Failed to open CSV output file: " + output_file;
            RCLCPP_FATAL(this->get_logger(), "%s", error.c_str());
            throw std::runtime_error(error);
        }

        if (file_.tellp() == std::streampos(0)) {
            file_ << "time_seconds,speed_kmh,throttle,brake,steering,traffic_light,obstacle\n";
        }

        // 2. Create Subscribers for all relevant topics
        state_sub_ = this->create_subscription<vehicle_control::msg::VehicleState>(
            vehicle_control::topics::kVehicleState, 10,
            std::bind(&ExperimentLogger::state_callback, this, _1));

        control_sub_ = this->create_subscription<vehicle_control::msg::VehicleControl>(
            vehicle_control::topics::kVehicleControl, 10,
            std::bind(&ExperimentLogger::control_callback, this, _1));

        scenario_sub_ = this->create_subscription<vehicle_control::msg::ScenarioState>(
            vehicle_control::topics::kScenarioState, 10,
            std::bind(&ExperimentLogger::scenario_callback, this, _1));

        // Record the start time so we can calculate elapsed time
        start_time_ = this->now();

        RCLCPP_INFO(
            this->get_logger(), "Experiment Logger started. Writing to %s",
            output_file.c_str());
    }

    ~ExperimentLogger()
    {
        // Close the file when the node shuts down
        if (file_.is_open()) {
            file_.close();
        }
    }

private:
    // --- CALLBACKS (They just update our "latest value" cache) ---

    void state_callback(const vehicle_control::msg::VehicleState::SharedPtr msg)
    {
        current_speed_ = msg->speed;
        
        // The vehicle state updates at 10Hz. This is our "heartbeat".
        // Every time we get a new state, we write a full row to the CSV.
        write_csv_row();
    }

    void control_callback(const vehicle_control::msg::VehicleControl::SharedPtr msg)
    {
        throttle_ = msg->throttle;
        brake_ = msg->brake;
        steering_ = msg->steering;
    }

    void scenario_callback(const vehicle_control::msg::ScenarioState::SharedPtr msg)
    {
        light_state_ = msg->traffic_light;
        obstacle_ = msg->obstacle_detected;
    }

    // --- FILE WRITING LOGIC ---

    void write_csv_row()
    {
        // Calculate elapsed time in seconds
        double elapsed = (this->now() - start_time_).seconds();

        // Format the obstacle as TRUE/FALSE for readability
        const char * obs_str = obstacle_ ? "TRUE" : "FALSE";

        // Write the data, separated by commas
        file_ << std::fixed << std::setprecision(2) 
              << elapsed << ","
              << current_speed_ << ","
              << throttle_ << ","
              << brake_ << ","
              << steering_ << ","
              << light_name(light_state_) << ","
              << obs_str << "\n";
              
        // IMPORTANT: Flush the buffer so data isn't lost if the program crashes
        file_.flush(); 
    }

    static const char * light_name(std::uint8_t light_state)
    {
        switch (light_state) {
            case vehicle_control::msg::ScenarioState::GREEN:
                return "GREEN";
            case vehicle_control::msg::ScenarioState::YELLOW:
                return "YELLOW";
            case vehicle_control::msg::ScenarioState::RED:
                return "RED";
            default:
                return "UNKNOWN";
        }
    }

    // --- MEMBER VARIABLES ---
    std::ofstream file_;
    rclcpp::Time start_time_;

    // "Cache" for the latest values
    float current_speed_;
    float throttle_;
    float brake_;
    float steering_;
    std::uint8_t light_state_;
    bool obstacle_;

    // Subscribers
    rclcpp::Subscription<vehicle_control::msg::VehicleState>::SharedPtr state_sub_;
    rclcpp::Subscription<vehicle_control::msg::VehicleControl>::SharedPtr control_sub_;
    rclcpp::Subscription<vehicle_control::msg::ScenarioState>::SharedPtr scenario_sub_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ExperimentLogger>());
    rclcpp::shutdown();
    return 0;
}