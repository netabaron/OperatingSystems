# Vehicle Licensing Center Simulation

## About The Project
This project is a multithreaded simulation of a vehicle licensing center pipeline, implemented in C for Linux environments. It demonstrates advanced operating system concepts, specifically concurrent programming using POSIX threads (`pthreads`) and complex synchronization mechanisms.

## Key Technical Features
* **Multithreading:** Simulates vehicles and station workers (mechanics, inspectors) as independent, concurrently running threads.
* **Resource Synchronization:** Utilizes Mutexes and Semaphores to manage shared resources safely. For example, mechanics must acquire a shared diagnostic tablet and a torque wrench before processing a vehicle.
* **Priority Queues & Starvation Prevention:** Manages dynamic waiting queues with VIP priorities. To prevent resource starvation, regular vehicles waiting over 400ms are automatically upgraded to VIP status.
* **Pipeline Blocking:** Handles inter-station dependencies, such as Station 1 workers blocking and holding resources if the waiting queue for Station 2 is at full capacity (bounded buffer problem).

## System Architecture
1. **Station 1 (Mechanics):** 3 concurrent workers sharing a limited pool of tools (2 tablets, 2 wrenches).
2. **Station 2 (Air Pollution):** 2 inspectors processing vehicles from a limited-size queue.
3. **Checkout Station:** A single synchronized payment register handling the final exit step.