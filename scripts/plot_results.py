import csv
import matplotlib.pyplot as plt
import os

# 1. Read the CSV data
time_data = []
speed_data = []
light_data = []
obstacle_data = []

csv_path = "/home/maya_akl/ros2_ws/results/experiment_data.csv"

with open(csv_path, 'r') as file:
    reader = csv.DictReader(file)
    for row in reader:
        time_data.append(float(row['time_seconds']))
        speed_data.append(float(row['speed_kmh']))
        light_data.append(row['traffic_light'])
        obstacle_data.append(row['obstacle'] == 'TRUE')

# 2. Setup the Plot
plt.figure(figsize=(12, 6))
plt.plot(time_data, speed_data, label='Vehicle Speed (km/h)', color='blue', linewidth=2)
plt.axhline(y=30.0, color='green', linestyle='--', label='Target Speed (30 km/h)')

# 3. Add Shaded Regions for Events
for i in range(len(time_data) - 1):
    if light_data[i] == 'RED':
        plt.axvspan(time_data[i], time_data[i+1], color='red', alpha=0.3, label='RED Light' if i == 0 else "")

for i in range(len(time_data) - 1):
    if obstacle_data[i]:
        plt.axvspan(time_data[i], time_data[i+1], color='orange', alpha=0.5, label='Obstacle' if i == 0 else "")

# 4. Format the Graph
plt.title('Autonomous Vehicle Scenario Experiment', fontsize=16)
plt.xlabel('Time (seconds)', fontsize=12)
plt.ylabel('Speed (km/h)', fontsize=12)
plt.grid(True, linestyle=':', alpha=0.7)
plt.legend(loc='best')
plt.tight_layout()

# 5. SAVE TO THE NEW RESULTS FOLDER
output_path = "/home/maya_akl/ros2_ws/results/experiment_plot.png"
plt.savefig(output_path, dpi=300)
print(f"Success! Graph saved to: {output_path}")
