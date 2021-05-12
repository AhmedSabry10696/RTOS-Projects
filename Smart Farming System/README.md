#   Smart Farming System

A real time system based on FreeRTOS and Atmega32 controlling:

1 - water pump based on the humidity sensor data.

2 - Cooler and heater based on the temperature sensor data.

3 - also configure the threshold for both temperature and humidity in run-time by PC (terminal interfacing)

## Tasks 

the system consist of 6 tasks:

![Tasksdiagram](./img/Tasksdiagram.png)

## State Machines for some tasks:

### Terminal task state machine:

![Tasksdiagram](./img/terminalStateMachine.png)


### Display task state machine:

![Tasksdiagram](./img/displayStateMachine.png)


### SysCheck task state machine:

![Tasksdiagram](./img/syscheckStateMachine.png)


### Control task state machine:

![Tasksdiagram](./img/controlStateMachine.png)


### Simulation Video
