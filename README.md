# Airline Check-in System (ACS) Simulator

A multi-threaded task scheduler and system simulation built in C using POSIX threads (`pthreads`). 

This project simulates an airline check-in counter to demonstrate advanced operating system concepts, including concurrent programming, thread synchronization, and priority-based task scheduling.

## 🚀 Key Features & Concepts
* **Multi-threading:** Uses `pthreads` to simulate independent, concurrently arriving customers.
* **Thread Synchronization:** Implements **Mutexes** and **Condition Variables** to prevent race conditions and ensure thread-safe access to shared resources (queues and clerks).
* **Priority Scheduling:** Manages two distinct queues with priority logic:
  * **Business Class (High Priority):** Served first whenever a clerk is available.
  * **Economy Class (Standard Priority):** Served FIFO only when the Business queue is empty.
* **Resource Management:** Dynamically schedules 5 simulated check-in clerks to handle varying customer loads and service times without busy-waiting (utilizing condition variables to sleep/wake).
* **Performance Analytics:** Tracks and outputs relative machine time for events and calculates average wait times for different customer tiers.

## 🛠️ Tech Stack
* **Language:** C
* **Libraries:** `<pthread.h>`, standard C libraries
* **Environment:** Linux / Unix
* **Build Tool:** Make

## 🏗️ System Architecture
* **Clerks (Resources):** 5 available clerks handling customer processing.
* **Queues:** 
  * `Queue 1`: Business Class
  * `Queue 0`: Economy Class
* **Simulation Time:** Real-time simulation where 1 simulation unit equals 0.1 seconds (10ths of a second) using `usleep()`.

## ⚙️ Installation & Usage

### Prerequisites
* A Linux/Unix environment (or WSL on Windows)
* GCC Compiler
* Make

### Building the Project
Clone the repository and compile the program using the included Makefile:
```bash
git clone [https://github.com/yourusername/airline-checkin-simulator.git](https://github.com/yourusername/airline-checkin-simulator.git)
cd airline-checkin-simulator
make
