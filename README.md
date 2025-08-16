

Python Elevator Location Generation
=========================================
To generate elevator locations into a booksim config file:
- Edit the sample_elevators.csv file, or create your own csv and put elevators in your desired coordinates
- run `python3 map.py <name of aforementioned CSV file> <name of booksim config file for results to be written to> <X width> <Y width>`

As a test, you should be able to run `python3 map.py sample_elevators.csv <name of arbritrary text file here> 8 8` from `utils/elevator`, with the existing values in `utils/elevator/sample_elevators.csv`, and it should print out this visual diagram:
```
O
 Abcdeaaa
 bBcdefbb
 ccCdefgc
 dddDefgh
 aeeeEfgh
 abfffFgh
 abcgggGh
 abcdhhhH
```
Where the uppercase letters represent elevators, and the lowercase letters map represent non-elevator switches that map to the elevator with that corresponding letter. Feel free to hotswap the mapping function in `user_mapper_function.py`. More details about how to do that in the file.

Network Details
===============
The current implementation uses a DOR router and a {Unidirectional Torus or Bidirectional Mesh}. Here are visualizations of the individual nodes (with elevator and without) for a Unidirectional Mesh:
![alt text](https://github.com/VoarL/booksim2-3dtorus/blob/master/img/drawing-nodes.png "Nodes")
These can be combined into 2D layers of our network, where some of the nodes will have elevators. Here is an example of a layer with elevators across the diagonal:
![alt text](https://github.com/VoarL/booksim2-3dtorus/blob/master/img/drawing-torus-elev.png "2D layer")
Finally, the 2D layers can be combined into a 3D structure:
![alt text](https://github.com/VoarL/booksim2-3dtorus/blob/master/img/drawing-torus-elev-stack.png "3D network")




This repo is a fork of [booksim2](https://github.com/booksim/booksim2). See the README for that below. 

BookSim Interconnection Network Simulator
=========================================

BookSim is a cycle-accurate interconnection network simulator.
Originally developed for and introduced with the [Principles and Practices of Interconnection Networks](http://cva.stanford.edu/books/ppin/) book, its functionality has since been continuously extended.
The current major release, BookSim 2.0, supports a wide range of topologies such as mesh, torus and flattened butterfly networks, provides diverse routing algorithms and includes numerous options for customizing the network's router microarchitecture.

---

If you use BookSim in your research, we would appreciate the following citation in any publications to which it has contributed:

Nan Jiang, Daniel U. Becker, George Michelogiannakis, James Balfour, Brian Towles, John Kim and William J. Dally. A Detailed and Flexible Cycle-Accurate Network-on-Chip Simulator. In *Proceedings of the 2013 IEEE International Symposium on Performance Analysis of Systems and Software*, 2013.
