import math
# Here is a sample mapper function. A mapper function is responsible for assigning a pair of coordinates
# of an elevator to each X,Y pair. That X,Y pair will navigate to those elevator coordinates whenever it needs
# to switch to a different layer.
#
# This specific function will map to the router that is the least number of hops away(ie shortest taxicab distance),
# And will search only in increasing X/Y direction(as its for unidirectional toruses)
# but you may implement your own mapper function if least number of hops does not suit your needs.
def find_nearest_elev_unidir_taxicab_prioritize_Y(x: int,y: int,x_size: int, y_size: int, elevators: list[tuple[int,int]]) -> tuple[int,int]:
    max_dist =(x_size-1)+(y_size-1)
    for tcab_dist in range(max_dist):
        for i in range(tcab_dist+1):
            el_x = (x+i)%x_size
            el_y = (y+tcab_dist-i)%y_size
            if (el_x,el_y) in elevators:
                return((el_x,el_y))

# Same as find_nearest_elev_unidir_taxicab_prioritize_Y, but searches middle out,
# prioritizing more diagonal routes over straighter ones
def find_nearest_elev_unidir_taxicab_middle_out(x: int,y: int,x_size: int, y_size: int, elevators: list[tuple[int,int]]) -> tuple[int,int]:
    max_dist =(x_size-1)+(y_size-1)
    for tcab_dist in range(max_dist+1):
        for i in range(tcab_dist+1):
            p = (i+1)>>1
            if (i%2==0):
                el_x = (x+math.ceil(tcab_dist/2)+p)%x_size
                el_y = (y+math.floor(tcab_dist/2)-p)%y_size
            else:
                el_x = (x+math.ceil(tcab_dist/2)-p)%x_size
                el_y = (y+math.floor(tcab_dist/2)+p)%y_size
            if (el_x,el_y) in elevators:
                return((el_x,el_y))
        