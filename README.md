# Redwood Project

## Overview
This repository aims to streamline the process of configuring and testing ATLAS grid computing environment.

## Prerequisites
Before you begin, ensure you have the following packages installed:

- **SimGrid v3.35** 
- **Boost**

These packages are essential for building and running the simulation.

## Build Instructions
Follow these steps to build the project:
   ```bash
   mkdir build
   cd build
   cmake ..
   make -j
   ```

## Run Instructions

   ```bash
   ./atlas-grid-simulator -J ../data/data.json
   ```

