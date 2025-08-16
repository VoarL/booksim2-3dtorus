import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

df = pd.read_csv("results_unitorus.csv")

# Handle infinite latency values (failed simulations)
df['AvgLatency'] = pd.to_numeric(df['AvgLatency'], errors='coerce')
df['Throughput'] = pd.to_numeric(df['Throughput'], errors='coerce')

# Cap latency at 150 for better visualization
df.loc[df['AvgLatency'] > 150, 'AvgLatency'] = 150
df.loc[df['AvgLatency'] == np.inf, 'AvgLatency'] = 150

# Get the actual max injection rate from data
max_injection = df['InjectionRate'].max()

# Automatically detect the most common VC value to use
vc_counts = df['VCs'].value_counts()
FIXED_VC = vc_counts.index[0]  # Use the most frequent VC value
print(f"Using VC count: {FIXED_VC}")

df_filtered = df[df['VCs'] == FIXED_VC]

# Get all unique network sizes
network_sizes = sorted(df_filtered['Size'].unique())
print(f"Found network sizes: {network_sizes}")

if len(network_sizes) == 0:
    print("Error: No data found! Check your CSV file.")
    exit(1)

# MAIN PLOT: Uniform traffic with network size comparison
uniform_data = df_filtered[df_filtered['Traffic'] == 'uniform']

if len(uniform_data) == 0:
    print("Error: No uniform traffic data found!")
    exit(1)

fig, axes = plt.subplots(1, len(network_sizes), figsize=(5*len(network_sizes), 5))
if len(network_sizes) == 1:
    axes = [axes]  # Make it iterable for single subplot

for i, size in enumerate(network_sizes):
    ax = axes[i]
    
    size_data = uniform_data[uniform_data['Size'] == size]
    
    # Plot mesh
    mesh_data = size_data[size_data['VerticalTopology'] == 'mesh']
    if len(mesh_data) > 0:
        ax.plot(mesh_data['InjectionRate'], mesh_data['AvgLatency'], 
               marker='o', linewidth=3, markersize=8, 
               label='Mesh', linestyle='-', color='blue')
    
    # Plot torus
    torus_data = size_data[size_data['VerticalTopology'] == 'torus']
    if len(torus_data) > 0:
        ax.plot(torus_data['InjectionRate'], torus_data['AvgLatency'], 
               marker='s', linewidth=3, markersize=8, 
               label='Torus', linestyle='--', color='red')
    
    ax.set_title(f'Network Size {size}', fontsize=14)
    ax.set_xlabel('Injection Rate', fontsize=12)
    ax.set_ylabel('Average Packet Latency (cycles)', fontsize=12)
    ax.set_ylim(0, 150)
    ax.set_xlim(0.01, max_injection)  # Use your actual min injection rate
    ax.grid(True, alpha=0.3)
    ax.legend(fontsize=12)

plt.suptitle(f'Mesh vs Torus Elevator Checkerboard Pattern ({FIXED_VC} VCs, Uniform)', fontsize=16, y=0.98)
plt.tight_layout()
plt.savefig("uniform_mesh_vs_torus.png", dpi=300, bbox_inches='tight')
plt.show()

# SEPARATE PLOTS for other traffic patterns
other_traffic_patterns = [t for t in df_filtered['Traffic'].unique() if t != 'uniform']

for traffic in other_traffic_patterns:
    traffic_data = df_filtered[df_filtered['Traffic'] == traffic]
    
    if len(traffic_data) == 0:
        print(f"No data found for {traffic} traffic, skipping...")
        continue
    
    fig, axes = plt.subplots(1, len(network_sizes), figsize=(5*len(network_sizes), 5))
    if len(network_sizes) == 1:
        axes = [axes]
    
    for i, size in enumerate(network_sizes):
        ax = axes[i]
        
        size_data = traffic_data[traffic_data['Size'] == size]
        
        # Plot mesh
        mesh_data = size_data[size_data['VerticalTopology'] == 'mesh']
        if len(mesh_data) > 0:
            ax.plot(mesh_data['InjectionRate'], mesh_data['AvgLatency'], 
                   marker='o', linewidth=3, markersize=8, 
                   label='Mesh', linestyle='-', color='blue')
        
        # Plot torus
        torus_data = size_data[size_data['VerticalTopology'] == 'torus']
        if len(torus_data) > 0:
            ax.plot(torus_data['InjectionRate'], torus_data['AvgLatency'], 
                   marker='s', linewidth=3, markersize=8, 
                   label='Torus', linestyle='--', color='red')
        
        ax.set_title(f'Network Size {size}', fontsize=14)
        ax.set_xlabel('Injection Rate', fontsize=12)
        ax.set_ylabel('Average Packet Latency (cycles)', fontsize=12)
        ax.set_ylim(0, 200)
        ax.set_xlim(0.01, max_injection)
        ax.grid(True, alpha=0.3)
        ax.legend(fontsize=12)
    
    plt.suptitle(f'{traffic.title()} Traffic: Mesh vs Torus ({FIXED_VC} VCs)', fontsize=16, y=0.98)
    plt.tight_layout()
    plt.savefig(f"{traffic}_mesh_vs_torus.png", dpi=300, bbox_inches='tight')
    plt.show()

print(f"Plots saved:")
print(f"- uniform_mesh_vs_torus.png (main plot)")
for traffic in other_traffic_patterns:
    if traffic in df_filtered['Traffic'].unique():
        print(f"- {traffic}_mesh_vs_torus.png")