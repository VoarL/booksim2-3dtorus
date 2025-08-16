#!/bin/bash

# Sweep script for unitorus topology run python plot.py afterwards
# Usage: ./sweep_unitorus.sh

BOOKSIM_PATH="./booksim"
CONFIG_BASE="config_unitorus_sweep.config"
OUTPUT_FILE="results_unitorus.csv"

# Injection rate parameters 
MIN_INJECTION=0.01
MAX_INJECTION=0.5  
INJECTION_STEP=0.01

# Generate injection rates array 
INJECTION_RATES=($(seq $MIN_INJECTION $INJECTION_STEP $MAX_INJECTION))

# Traffic patterns to test
TRAFFIC_PATTERNS=("uniform")

# Network sizes with their elevator mappings 
declare -A SIZE_ELEVATOR_MAP
# comment out the unused one
#diagonal:
#SIZE_ELEVATOR_MAP["4_4_3"]="0,0,1,1,0,0,0,0,1,1,1,1,2,2,1,1,2,2,2,2,2,2,3,3,0,0,3,3,3,3,3,3"
#SIZE_ELEVATOR_MAP["8_8_3"]="0,0,1,1,2,2,3,3,0,0,0,0,0,0,0,0,1,1,1,1,2,2,3,3,4,4,1,1,1,1,1,1,2,2,2,2,2,2,3,3,4,4,5,5,2,2,2,2,3,3,3,3,3,3,3,3,4,4,5,5,6,6,3,3,4,4,4,4,4,4,4,4,4,4,5,5,6,6,7,7,0,0,5,5,5,5,5,5,5,5,5,5,6,6,7,7,0,0,1,1,6,6,6,6,6,6,6,6,6,6,7,7,0,0,1,1,2,2,7,7,7,7,7,7,7,7,7,7"
#SIZE_ELEVATOR_MAP["16_16_3"]="0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,5,5,5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13,6,6,6,6,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,8,8,9,9,10,10,11,11,12,12,13,13,14,14,7,7,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,9,10,10,11,11,12,12,13,13,14,14,15,15,0,0,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,10,10,11,11,12,12,13,13,14,14,15,15,0,0,1,1,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,11,12,12,13,13,14,14,15,15,0,0,1,1,2,2,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,12,12,13,13,14,14,15,15,0,0,1,1,2,2,3,3,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,13,13,14,14,15,15,0,0,1,1,2,2,3,3,4,4,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,14,14,15,15,0,0,1,1,2,2,3,3,4,4,5,5,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,15,15,0,0,1,1,2,2,3,3,4,4,5,5,6,6,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15"
#tiling 4x4 tiles
#SIZE_ELEVATOR_MAP["8_8_3"]="3,3,3,3,3,3,3,3,3,7,3,7,3,7,3,7,3,3,3,3,3,3,3,3,3,7,3,7,3,7,3,7,3,3,3,3,3,3,3,3,3,7,3,7,3,7,3,7,3,3,3,3,3,3,3,3,3,7,3,7,3,7,3,7,7,3,7,3,7,3,7,3,7,7,7,7,7,7,7,7,7,3,7,3,7,3,7,3,7,7,7,7,7,7,7,7,7,3,7,3,7,3,7,3,7,7,7,7,7,7,7,7,7,3,7,3,7,3,7,3,7,7,7,7,7,7,7,7"
#SIZE_ELEVATOR_MAP["16_16_3"]="3,3,3,3,3,3,3,3,3,7,3,7,3,7,3,7,3,11,3,11,3,11,3,11,3,15,3,15,3,15,3,15,3,3,3,3,3,3,3,3,3,7,3,7,3,7,3,7,3,11,3,11,3,11,3,11,3,15,3,15,3,15,3,15,3,3,3,3,3,3,3,3,3,7,3,7,3,7,3,7,3,11,3,11,3,11,3,11,3,15,3,15,3,15,3,15,3,3,3,3,3,3,3,3,3,7,3,7,3,7,3,7,3,11,3,11,3,11,3,11,3,15,3,15,3,15,3,15,7,3,7,3,7,3,7,3,7,7,7,7,7,7,7,7,7,11,7,11,7,11,7,11,7,15,7,15,7,15,7,15,7,3,7,3,7,3,7,3,7,7,7,7,7,7,7,7,7,11,7,11,7,11,7,11,7,15,7,15,7,15,7,15,7,3,7,3,7,3,7,3,7,7,7,7,7,7,7,7,7,11,7,11,7,11,7,11,7,15,7,15,7,15,7,15,7,3,7,3,7,3,7,3,7,7,7,7,7,7,7,7,7,11,7,11,7,11,7,11,7,15,7,15,7,15,7,15,11,3,11,3,11,3,11,3,11,7,11,7,11,7,11,7,11,11,11,11,11,11,11,11,11,15,11,15,11,15,11,15,11,3,11,3,11,3,11,3,11,7,11,7,11,7,11,7,11,11,11,11,11,11,11,11,11,15,11,15,11,15,11,15,11,3,11,3,11,3,11,3,11,7,11,7,11,7,11,7,11,11,11,11,11,11,11,11,11,15,11,15,11,15,11,15,11,3,11,3,11,3,11,3,11,7,11,7,11,7,11,7,11,11,11,11,11,11,11,11,11,15,11,15,11,15,11,15,15,3,15,3,15,3,15,3,15,7,15,7,15,7,15,7,15,11,15,11,15,11,15,11,15,15,15,15,15,15,15,15,15,3,15,3,15,3,15,3,15,7,15,7,15,7,15,7,15,11,15,11,15,11,15,11,15,15,15,15,15,15,15,15,15,3,15,3,15,3,15,3,15,7,15,7,15,7,15,7,15,11,15,11,15,11,15,11,15,15,15,15,15,15,15,15,15,3,15,3,15,3,15,3,15,7,15,7,15,7,15,7,15,11,15,11,15,11,15,11,15,15,15,15,15,15,15,15"
#tiling 2x2 tiles
#SIZE_ELEVATOR_MAP["4_4_3"]="1,1,1,1,1,3,1,3,1,1,1,1,1,3,1,3,3,1,3,1,3,3,3,3,3,1,3,1,3,3,3,3"
#SIZE_ELEVATOR_MAP["8_8_3"]="1,1,1,1,1,3,1,3,1,5,1,5,1,7,1,7,1,1,1,1,1,3,1,3,1,5,1,5,1,7,1,7,3,1,3,1,3,3,3,3,3,5,3,5,3,7,3,7,3,1,3,1,3,3,3,3,3,5,3,5,3,7,3,7,5,1,5,1,5,3,5,3,5,5,5,5,5,7,5,7,5,1,5,1,5,3,5,3,5,5,5,5,5,7,5,7,7,1,7,1,7,3,7,3,7,5,7,5,7,7,7,7,7,1,7,1,7,3,7,3,7,5,7,5,7,7,7,7"
#SIZE_ELEVATOR_MAP["16_16_3"]="1,1,1,3,1,5,1,7,1,9,1,11,1,13,1,15,3,1,3,3,3,5,3,7,3,9,3,11,3,13,3,15,5,1,5,3,5,5,5,7,5,9,5,11,5,13,5,15,7,1,7,3,7,5,7,7,7,9,7,11,7,13,7,15,9,1,9,3,9,5,9,7,9,9,9,11,9,13,9,15,11,1,11,3,11,5,11,7,11,9,11,11,11,13,11,15,13,1,13,3,13,5,13,7,13,9,13,11,13,13,13,15,15,1,15,3,15,5,15,7,15,9,15,11,15,13,15,15"
#checkerboard
SIZE_ELEVATOR_MAP["4_4_3"]="0,0,1,1,0,2,1,3,2,0,1,1,2,2,1,3,2,0,3,1,2,2,3,3,0,0,3,1,0,2,3,3"
SIZE_ELEVATOR_MAP["8_8_3"]="0,0,1,1,0,2,1,3,0,4,1,5,0,6,1,7,2,0,1,1,2,2,1,3,2,4,1,5,2,6,1,7,2,0,3,1,2,2,3,3,2,4,3,5,2,6,3,7,4,0,3,1,4,2,3,3,4,4,3,5,4,6,3,7,4,0,5,1,4,2,5,3,4,4,5,5,4,6,5,7,6,0,5,1,6,2,5,3,6,4,5,5,6,6,5,7,6,0,7,1,6,2,7,3,6,4,7,5,6,6,7,7,0,0,7,1,0,2,7,3,0,4,7,5,0,6,7,7"
SIZE_ELEVATOR_MAP["16_16_3"]="0,0,1,1,0,2,1,3,0,4,1,5,0,6,1,7,0,8,1,9,0,10,1,11,0,12,1,13,0,14,1,15,2,0,1,1,2,2,1,3,2,4,1,5,2,6,1,7,2,8,1,9,2,10,1,11,2,12,1,13,2,14,1,15,2,0,3,1,2,2,3,3,2,4,3,5,2,6,3,7,2,8,3,9,2,10,3,11,2,12,3,13,2,14,3,15,4,0,3,1,4,2,3,3,4,4,3,5,4,6,3,7,4,8,3,9,4,10,3,11,4,12,3,13,4,14,3,15,4,0,5,1,4,2,5,3,4,4,5,5,4,6,5,7,4,8,5,9,4,10,5,11,4,12,5,13,4,14,5,15,6,0,5,1,6,2,5,3,6,4,5,5,6,6,5,7,6,8,5,9,6,10,5,11,6,12,5,13,6,14,5,15,6,0,7,1,6,2,7,3,6,4,7,5,6,6,7,7,6,8,7,9,6,10,7,11,6,12,7,13,6,14,7,15,8,0,7,1,8,2,7,3,8,4,7,5,8,6,7,7,8,8,7,9,8,10,7,11,8,12,7,13,8,14,7,15,8,0,9,1,8,2,9,3,8,4,9,5,8,6,9,7,8,8,9,9,8,10,9,11,8,12,9,13,8,14,9,15,10,0,9,1,10,2,9,3,10,4,9,5,10,6,9,7,10,8,9,9,10,10,9,11,10,12,9,13,10,14,9,15,10,0,11,1,10,2,11,3,10,4,11,5,10,6,11,7,10,8,11,9,10,10,11,11,10,12,11,13,10,14,11,15,12,0,11,1,12,2,11,3,12,4,11,5,12,6,11,7,12,8,11,9,12,10,11,11,12,12,11,13,12,14,11,15,12,0,13,1,12,2,13,3,12,4,13,5,12,6,13,7,12,8,13,9,12,10,13,11,12,12,13,13,12,14,13,15,14,0,13,1,14,2,13,3,14,4,13,5,14,6,13,7,14,8,13,9,14,10,13,11,14,12,13,13,14,14,13,15,14,0,15,1,14,2,15,3,14,4,15,5,14,6,15,7,14,8,15,9,14,10,15,11,14,12,15,13,14,14,15,15,0,0,15,1,0,2,15,3,0,4,15,5,0,6,15,7,0,8,15,9,0,10,15,11,0,12,15,13,0,14,15,15"

# Get list of sizes
SIZES=($(echo "${!SIZE_ELEVATOR_MAP[@]}" | tr ' ' '\n' | sort))

# VC counts to test  
VC_COUNTS=(8)

# Vertical topologies to test 
VERTICAL_TOPOLOGIES=("mesh" "torus")

# Create CSV header 
echo "Traffic,InjectionRate,Size,VCs,VerticalTopology,AvgLatency,Throughput" > $OUTPUT_FILE

for traffic in "${TRAFFIC_PATTERNS[@]}"; do
    for size in "${SIZES[@]}"; do
        for vcs in "${VC_COUNTS[@]}"; do
            for vertical_topo in "${VERTICAL_TOPOLOGIES[@]}"; do 
                echo "Testing: Traffic=$traffic, Size=$size, VCs=$vcs, VerticalTopo=$vertical_topo"
                
                for rate in "${INJECTION_RATES[@]}"; do
                    echo "  Injection rate: $rate"
                    
                    # Create temporary config file
                    TEMP_CONFIG="temp_config_${traffic}_${size}_${vcs}_${vertical_topo}.config"
                    
                    # Copy base config and modify parameters
                    cp $CONFIG_BASE $TEMP_CONFIG
                    
                    # Convert underscore back to comma for dim_sizes
                    SIZE_WITH_COMMAS=${size//_/,}
                    
                    # Get the corresponding elevator mapping for this size 
                    ELEVATOR_COORDS=${SIZE_ELEVATOR_MAP[$size]}
                    
                    # Update parameters using sed 
                    sed -i "s/traffic = .*/traffic = $traffic;/" $TEMP_CONFIG
                    sed -i "s/injection_rate = .*/injection_rate = $rate;/" $TEMP_CONFIG
                    sed -i "s/dim_sizes = .*/dim_sizes = {${SIZE_WITH_COMMAS}};/" $TEMP_CONFIG
                    sed -i "s/num_vcs = .*/num_vcs = $vcs;/" $TEMP_CONFIG
                    sed -i "s/vertical_topology = .*/vertical_topology = $vertical_topo;/" $TEMP_CONFIG
                    sed -i "s/elevator_mapping_coords = .*/elevator_mapping_coords = {${ELEVATOR_COORDS}};/" $TEMP_CONFIG
                    
                    # Run simulation and capture all output
                    FULL_OUTPUT=$($BOOKSIM_PATH $TEMP_CONFIG 2>&1)
                    
                    # Extract final results - try both formats (KEPT: your working extraction logic)
                    # Format 1: with "(1 samples)" - when sampling is enabled
                    LATENCY_WITH_SAMPLES=$(echo "$FULL_OUTPUT" | grep "Packet latency average.*samples)" | tail -1 | awk '{print $5}')
                    THROUGHPUT_WITH_SAMPLES=$(echo "$FULL_OUTPUT" | grep "Accepted packet rate average.*samples)" | tail -1 | awk '{print $6}')
                    
                    # Format 2: without samples - when sampling is disabled, look for final results after "Overall Traffic Statistics"
                    if [ -z "$LATENCY_WITH_SAMPLES" ] || [ -z "$THROUGHPUT_WITH_SAMPLES" ]; then
                        FINAL_SECTION=$(echo "$FULL_OUTPUT" | sed -n '/Overall Traffic Statistics/,$p')
                        LATENCY=$(echo "$FINAL_SECTION" | grep "Packet latency average" | head -1 | awk '{print $5}')
                        THROUGHPUT=$(echo "$FINAL_SECTION" | grep "Accepted packet rate average" | head -1 | awk '{print $6}')
                    else
                        LATENCY=$LATENCY_WITH_SAMPLES
                        THROUGHPUT=$THROUGHPUT_WITH_SAMPLES
                    fi
                    
                    # Check if we got valid results
                    if [ -z "$LATENCY" ] || [ -z "$THROUGHPUT" ] || [ "$LATENCY" = "-nan" ] || [ "$THROUGHPUT" = "-nan" ]; then
                        echo "    FAILED: No valid final results found"
                        LATENCY="inf"
                        THROUGHPUT="0"
                    else
                        echo "    SUCCESS: Latency=$LATENCY, Throughput=$THROUGHPUT"
                    fi
                    
                    # Write to CSV with quoted size field to handle commas 
                    echo "$traffic,$rate,\"${SIZE_WITH_COMMAS}\",$vcs,$vertical_topo,$LATENCY,$THROUGHPUT" >> $OUTPUT_FILE
                    
                    # Clean up
                    rm $TEMP_CONFIG
                    
                    # Exit early if simulation becomes unstable
                    if [ "$LATENCY" = "inf" ]; then
                        echo "    Stopping sweep for this configuration due to failure"
                        break
                    fi
                done
            done  
        done
    done
done

echo "Sweep completed. Results saved to $OUTPUT_FILE"