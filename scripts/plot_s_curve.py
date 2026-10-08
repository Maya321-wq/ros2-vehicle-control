import csv
import matplotlib.pyplot as plt

time_data = []
speed_data = []
steering_data = []

csv_path = "/home/maya_akl/ros2_ws/results/experiment_data.csv"

with open(csv_path, 'r') as file:
    reader = csv.DictReader(file)
    for row in reader:
        time_data.append(float(row['time_seconds']))
        speed_data.append(float(row['speed_kmh']))
        steering_data.append(float(row['steering']))

fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 8), sharex=True)

ax1.plot(time_data, speed_data, label='Vehicle Speed (km/h)', color='blue', linewidth=2)
ax1.axhline(y=30.0, color='green', linestyle='--', label='Target Speed (30 km/h)')
ax1.set_ylabel('Speed (km/h)', fontsize=12)
ax1.set_title('S-Curve Scenario', fontsize=16)
ax1.legend(loc='best')
ax1.grid(True, linestyle=':', alpha=0.7)

ax2.plot(time_data, steering_data, label='Steering Angle', color='orange', linewidth=2)
ax2.axhline(y=0.0, color='black', linestyle='-', linewidth=0.5)
ax2.set_ylabel('Steering (-1.0 to 1.0)', fontsize=12)
ax2.set_xlabel('Time (seconds)', fontsize=12)
ax2.legend(loc='best')
ax2.grid(True, linestyle=':', alpha=0.7)

plt.tight_layout()
output_path = "/home/maya_akl/ros2_ws/results/s_curve_plot.png"
plt.savefig(output_path, dpi=300)
print(f"Success! S-Curve Graph saved to: {output_path}")
