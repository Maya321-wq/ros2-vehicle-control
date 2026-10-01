#include <cmath>
#include <functional>
#include <memory>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "visualization_msgs/msg/marker.hpp"
#include "vehicle_control/msg/vehicle_state.hpp"

using std::placeholders::_1;

class Visualizer : public rclcpp::Node
{
public:
    Visualizer()
        : Node("visualizer")
    {
        state_subscription_ =
            this->create_subscription<vehicle_control::msg::VehicleState>(
                "/vehicle_state", 10,
                std::bind(&Visualizer::state_callback, this, _1));

        marker_publisher_ =
            this->create_publisher<visualization_msgs::msg::Marker>(
                "/vehicle_markers", 10);

        RCLCPP_INFO(this->get_logger(),
            "Visualizer started. Open RViz2 and add Marker from /vehicle_markers topic.");
    }

private:
    void state_callback(const vehicle_control::msg::VehicleState::SharedPtr msg)
    {
        // Store last 200 positions for the path trail
        positions_.push_back({msg->position_x, msg->position_y});
        if (positions_.size() > 200) {
            positions_.erase(positions_.begin());
        }

        // ===== MARKER 1: PATH TRAIL (blue line) =====
        visualization_msgs::msg::Marker path_marker;
        path_marker.header.frame_id = "map";
        path_marker.header.stamp = this->now();
        path_marker.ns = "vehicle_path";
        path_marker.id = 0;
        path_marker.type = visualization_msgs::msg::Marker::LINE_STRIP;
        path_marker.action = visualization_msgs::msg::Marker::ADD;
        path_marker.scale.x = 0.3;  // line width
        path_marker.color.r = 0.0f;
        path_marker.color.g = 0.5f;
        path_marker.color.b = 1.0f;
        path_marker.color.a = 1.0f;

        for (const auto& pos : positions_) {
            geometry_msgs::msg::Point p;
            p.x = pos.first;
            p.y = pos.second;
            p.z = 0.1;
            path_marker.points.push_back(p);
        }
        marker_publisher_->publish(path_marker);

        // ===== MARKER 2: VEHICLE ARROW (red, shows heading) =====
        visualization_msgs::msg::Marker arrow;
        arrow.header.frame_id = "map";
        arrow.header.stamp = this->now();
        arrow.ns = "vehicle_arrow";
        arrow.id = 1;
        arrow.type = visualization_msgs::msg::Marker::ARROW;
        arrow.action = visualization_msgs::msg::Marker::ADD;

        geometry_msgs::msg::Point start;
        start.x = msg->position_x;
        start.y = msg->position_y;
        start.z = 0.2;
        arrow.points.push_back(start);

        geometry_msgs::msg::Point end;
        end.x = msg->position_x + 3.0 * std::cos(msg->yaw);
        end.y = msg->position_y + 3.0 * std::sin(msg->yaw);
        end.z = 0.2;
        arrow.points.push_back(end);

        arrow.scale.x = 0.4;  // shaft diameter
        arrow.scale.y = 0.8;  // head diameter
        arrow.scale.z = 0.0;
        arrow.color.r = 1.0f;
        arrow.color.g = 0.2f;
        arrow.color.b = 0.2f;
        arrow.color.a = 1.0f;
        marker_publisher_->publish(arrow);

        // ===== MARKER 3: VEHICLE BODY (green circle) =====
        visualization_msgs::msg::Marker body;
        body.header.frame_id = "map";
        body.header.stamp = this->now();
        body.ns = "vehicle_body";
        body.id = 2;
        body.type = visualization_msgs::msg::Marker::CYLINDER;
        body.action = visualization_msgs::msg::Marker::ADD;
        body.pose.position.x = msg->position_x;
        body.pose.position.y = msg->position_y;
        body.pose.position.z = 0.5;
        body.pose.orientation.w = 1.0;
        body.scale.x = 2.0;  // diameter
        body.scale.y = 2.0;
        body.scale.z = 1.0;
        body.color.r = 0.2f;
        body.color.g = 1.0f;
        body.color.b = 0.2f;
        body.color.a = 0.8f;
        marker_publisher_->publish(body);

        // ===== MARKER 4: SPEED TEXT =====
        visualization_msgs::msg::Marker text;
        text.header.frame_id = "map";
        text.header.stamp = this->now();
        text.ns = "vehicle_text";
        text.id = 3;
        text.type = visualization_msgs::msg::Marker::TEXT_VIEW_FACING;
        text.action = visualization_msgs::msg::Marker::ADD;
        text.pose.position.x = msg->position_x;
        text.pose.position.y = msg->position_y;
        text.pose.position.z = 2.5;
        text.scale.z = 1.0;  // text height
        text.color.r = 1.0f;
        text.color.g = 1.0f;
        text.color.b = 1.0f;
        text.color.a = 1.0f;
        char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "%.1f km/h", msg->speed);
        text.text = buffer;
        marker_publisher_->publish(text);
    }

    rclcpp::Subscription<vehicle_control::msg::VehicleState>::SharedPtr state_subscription_;
    rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr marker_publisher_;
    std::vector<std::pair<float, float>> positions_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Visualizer>());
    rclcpp::shutdown();
    return 0;
}
