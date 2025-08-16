import argparse
from ElevatorMapping import ElevatorMapping
from user_mapper_functions import *
parser = argparse.ArgumentParser()

parser.add_argument('elevatorcsv', help="CSV file containing List of X,Y pairs of elevators")
parser.add_argument('configfile', help="Name of config file to write output mapping to")
parser.add_argument('X',help="X width of torus-cake", type=int)
parser.add_argument('Y', help="Y width of torus-cake", type=int)


args = parser.parse_args()
x_size = args.X
y_size = args.Y

# Reference your desired mapper function from user_mapper_functions.py here
user_mapper_function = find_nearest_elev_unidir_taxicab_middle_out

# Create, visualize and export mapping
my_mapping = ElevatorMapping(args.elevatorcsv, x_size, y_size)
my_mapping.create_mapping(user_mapper_function)
my_mapping.visualize_mapping()
my_mapping.export_elev_mapping_to_config(args.configfile)
my_mapping.export_elev_coords_to_config(args.configfile)



            
