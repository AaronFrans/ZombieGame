# ZombieGame

Welcome to my Zombie Game AI! This is an artificial intelligence system I made. The goal of this is Ai is to traverse a generated level and try to survive as long as possible.

## Features

- **AI Behavior:** The AI uses a primitive Blackboard to determine what choices to make
- **Pathfinding:** The survivor uses a simple spyral pattern to explaore the world. Once it finds a house it will clear that house and take any usefull items.
- **Sensory Perception:** My AI can see everyting inside the FOV cone, it can also remember where it has been, but it has no other info about the world.

#Research

I used this project to try out multiple steering behaviours, grouped steering behaviours, pathfinding algorithms and AI decision making. 

## Steering Behaviours
- Seek
- Arrive
- Wander
- Pursuit
- Evade

## Group Steering Behaviours
- Cohesion
- Separation
- VelocityMatch

## Pathfinding Algorithms
- AStar

##  AI Decision Making
- Behaviour Trees
- Finite State Machines
- Influence Maps
