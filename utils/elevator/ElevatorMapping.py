
from typing import Callable
from pathlib import Path
class ElevatorMapping:
    def __init__(self, elevatorcsv:str, x_size: int, y_size: int):
        assert Path(elevatorcsv).exists(), f"CSV {elevatorcsv} does not exist"
        self.x_size = x_size
        self.y_size = y_size
        self.elevatorcsv = elevatorcsv
        xy_elevator_coords = []
        with open(self.elevatorcsv, newline='') as file:
            lines = file.readlines()
            assert lines[0].lower().rstrip('\r\n') == "x,y", "X,Y columns not present at top of csv file"
            for linenum in range(1,len(lines)):
                x,y=lines[linenum].split(',')
                try:
                    x = int(x)
                    y = int(y)
                except ValueError:
                    raise ValueError(f"Invalid int on line {linenum} of {self.elevatorcsv}")
                assert(x>=0 and x < self.x_size), f"Out of bounds x value {x} on line {linenum}"
                assert(y>=0 and y < self.y_size), f"Out of bounds y value {y} on line {linenum}"
                assert (x,y) not in xy_elevator_coords, f"Duplicate elevator {str(x,y)} on line {linenum}" 
                xy_elevator_coords.append((x,y))
        self.elevator_coords = xy_elevator_coords


    def create_mapping(self, func: Callable[[int,int,int,int,list[tuple[int,int]]], tuple[int,int]]):
        xy_elevator_mapping = []
        for x in range(self.x_size):
            xy_elevator_mapping.append([])
            for y in range(self.y_size):
                xy_elevator_mapping[x].append(func(x,y,self.x_size,self.y_size,self.elevator_coords))

        self.mapping = xy_elevator_mapping 

    def visualize_mapping(self):
        assert hasattr(self,"mapping"), "Must call create_mapping first"
        letter_dict={}
        symbols="abcdefghijklmnopqrstuvwyzабвгдеёжзийклмнопрстуфхцчшщъыьэюяabcdefghijklmnopqrstuvwyzабвгдеёжзийклмнопрстуфхцчшщъыьэюяabcdefghijklmnopqrstuvwyzабвгдеёжзийклмнопрстуфхцчшщъыьэюя"
        assert len(self.elevator_coords) <= len(symbols), \
            "You have more symbols than there are letters. " +\
            "Consider using accented letters or another script with uppercase/lowercase letters"
        for i in range(len(self.elevator_coords)):
            letter_dict[str(self.elevator_coords[i])]=symbols[i]
        print("O",end="")
        for y in range(self.y_size):
            print("",end="\r\n ")
            for x in range(self.x_size):
                location = str((x,y))
                elevator_location = str(self.mapping[x][y])
                if location in letter_dict:
                    print(letter_dict[location].upper(), end="")
                elif elevator_location in letter_dict:
                    print(letter_dict[elevator_location], end="")
                else:
                    print("X", end="")

    def export_elev_mapping_to_config(self, configfile: str, configvarname: str="elevatormapping"):
        assert hasattr(self,"mapping"), "Must call create_mapping first"
        assert Path(configfile).exists(), f"Config file {configfile} does not exist"
        str_to_add = f"\r\n{configvarname}="
        str_to_add+="{"
        for x in range(self.x_size):
            for y in range(self.y_size):
                coords_x, coords_y = self.mapping[x][y]
                str_to_add+=f"{coords_x},{coords_y}"
                if (not((y == self.y_size-1) and (x == self.x_size-1))):
                    str_to_add+=","
        str_to_add+="}"
        with open(configfile, "a") as f:
            f.write(str_to_add)
    def export_elev_coords_to_config(self, configfile: str, configvarname: str="elevatorcoords"):
        assert hasattr(self,"mapping"), "Must call create_mapping first"
        assert Path(configfile).exists(), f"Config file {configfile} does not exist"
        str_to_add = f"\r\n{configvarname}="
        str_to_add+="{"
        for i in range(len(self.elevator_coords)):
            coords_x, coords_y = self.elevator_coords[i]
            str_to_add+=f"{coords_x},{coords_y}"
            if i != len(self.elevator_coords) - 1:
                str_to_add+=","
        str_to_add+="}"
        with open(configfile, "a") as f:
            f.write(str_to_add)