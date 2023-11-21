# ML-TCP
This project is for the course at UIC ECE 533.

Project goal:
Implimentating a TCP Congestion Control with Reinforcement Learning over NS3 

NOTE: to run this project check the ns3 requirements 
NOTE: directories are assumed to have ns-3-dev/ as root (when mentioned in this readme)

HOW TO USE:
cd ns-3-dev
./ns3 configure --enable-examples
./ns3 run seventh.cc

The seventh.cc file has been modified to incorporate our own custom congestion control algorithm. 
the seventh example:
/examples/tutorial/seventh.cc

results of the congestion window are saved in root as 
seventh.cwnd


The beef of our project resides in the TCP class created. We created custom files for a TCP model in the ns3 library. This is a custom TCP class, configured to run in the ns3 library.

Our project files regarding ns3 changes are:

src/internet/model/tcp-project-ACC.cc
src/internet/model/tcp-project-ACC.h

There we modify the IncreaseWindow() function to be acting as our RL algorithm. That is consulting a q-table to make the appropriate cwnd size. All the code for the project exists within these two files and have been the workflow of the project. As well as using different mechanisms of visualizing the cwnd plot.


