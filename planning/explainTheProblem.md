# The Project 

FRIEND:

Building a robotic arm to clean tool 
specifically main Chanmber 

Main Chamber is box, 

which will have some expensive parts in it like stage and lens

## Tech Stack 

1. ROS2
   1. Robotic oprating system just topic and message pipeline
      1. NODE -- PUBLISH STUFF, SUB STUFF
      2. ACTION , GOAL 
      3. SERVICE 
      4. CONFIGUTION 
2. Gazebo
   1. Physic Simulation software 
3. Rviz2
   1. Robotic visulization 
4. Moveit
   1. moveit is tos2 library that plan and execute robot paths, with coliding with anything
5. c++
   1. most library in ros and moveit support only c++ 

## concepts to read
1. urdfs file
   1. What is robot in simulation 
      1. Link: One sturdy part
      2. Joint: is connect for link that can move
      3. collistion: this is area around joint that can collide 
      4. visulizaion: is the visual presentation of a link
      5. physic: motion , friction , lub, surface matrial 
2. srdf files
   1. movement of arm 
   2. groups that move together
   3. all contstraint 
3. moveit planning sequences 
   1. somehow this define robot movement with respect to other joints


# DEMO

1. Make a cuboid with screws and a moveable part in it 

2. arm with open the gate and replace the part 


