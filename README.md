# Redwood Project

## Overview
This repository aims to streamline the process of configuring and testing ATLAS grid computing environment.

## Prerequisites
Before you begin, ensure you have the following packages installed:

- **SimGrid v3.35** 
- **Boost**

These packages are essential for building and running the simulation.

## Local Build Instructions
Follow these steps to build the project locally:
   ```bash
   git clone https://github.com/REDWOOD24/ATLAS-GRID-SIMULATION.git
   cd ATLAS-GRID-SIMULATION
   mkdir build
   cd build
   cmake ..
   make -j
   ```

## LXPLUS Build Instructions
Follow these steps to build the project on lxplus:
   ```bash
   git clone https://github.com/REDWOOD24/ATLAS-GRID-SIMULATION.git
   cd ATLAS-GRID-SIMULATION
   source lxplus_setup.sh
   git clone https://github.com/simgrid/simgrid.git
   cd simgrid
   git fetch --all --tags --prune
   git checkout tags/v3.35 -b test
   mkdir build
   cd build
   cmake ..
   make -j
   cd ../../
   mkdir build
   cd build
   cmake -Dlxplus=ON ..
   make -j
   ```

## Run Instructions

   ```bash
  ./atlas-grid-simulator ../data/site_conn_info.json ../data/site_info.json
   ```

## Platform

Basic Layout of the ATLAS Grid implemented in the simulation.

```bash
  +---------+                      +---------+                    +---------+
  | AGLT2   |                      | BNL-    |                    | OTHER   |
  |         |                      | ATLAS   |                    | SITES   |
  +---------+                      +---------+                    +---------+
    |  |  |                        |  |   |                       | |  |
    |  |  | [Link: To CPU Nodes]   |  |   | [Link: To CPU Nodes]  | |  | [Link: To CPU Nodes]
    |  |_ |_ _ _                   |  |   |_ _               _ _ _| |  |_ _
    v    v      v                  v  v       v             v       v      v
  +----+ +----+ +----+         +----+ +----+ +----+       +----+ +----+ +----+
  |CPU1| |CPU2| |... |         |CPU1| |CPU2| |... |       |CPU1| |CPU2| |... |
  +----+ +----+ +----+         +----+ +----+ +----+       +----+ +----+ +----+
    ^                            ^                            ^
    | [Link: To Other Sites]     | [Link: To Other Sites]     | [Link: To Other Sites]
    |                            |                            |
    |                            |                            |
    |                            |                            |
    |                            |                            |
 [Link: AGLT2 <--> BNL]       [Link: BNL <--> OTHER]       [Link: OTHER <--> AGLT2]
    |                            |                            |
    v                            v                            v
  +---------+                   +---------+                  +---------+
  | BNL-    |                   | OTHER   |                  | AGLT2   |
  | ATLAS   |                   | SITES   |                  |         |
  +---------+                   +---------+                  +---------+
    |                            |                            |
    | [CPU1 Gateway]             | [CPU1 Gateway]             | [CPU1 Gateway]
    |                            |                            |
    v                            v                            v
  +----+ +----+ +----+         +----+ +----+ +----+       +----+ +----+ +----+
  |CPU1| |CPU2| |... |         |CPU1| |CPU2| |... |       |CPU1| |CPU2| |... |
  +----+ +----+ +----+         +----+ +----+ +----+       +----+ +----+ +----+
```




## Initializing Sites (Sample Site)

## Site Details

### INFN-ROMA3

**CPUs:**  37 ±(3) 

**Cores:** 32

**Disk Information:**

- **CERN-PROD_DATADISK:**        625703 GiB
- **INFN-ROMA3_DATADISK:**       54 GiB
- **INFN-ROMA3_LOCALGROUPDISK:** 973 GiB
- **INFN-ROMA3_SCRATCHDISK:**    86 GiB

---

### Notes

- Information obtained from site data dumps.
- CPUs based off GFLOPS obtained from data dumps from site, estimating 500 GFLOPS per core per cpu.
- Other estimates such as latency for connections, disk read and write bandwidths, cpu speed based on estimates.

## Job Manager

 - Job has 3 parts -> (Read, Compute, Write).
 - Set sizes of files to be read and written from a gaussian distribution (mean and stddev estimated).
 - Estimate GFLOPS for job.


## PANDA Dispatcher 

    +-------------------+ 
    | Job Manager       |
    +-------------------+
                        \ Jobs
                         \
                          +------------+     +---------------------------------+       +---------+ 
                          | Algorithm  | --> | Sub-Jobs allocated to Resources |  -->  | Output  |
                          +------------+     +---------------------------------+       +---------+ 
                         /
                        / Resources
    +-------------------+ 
    | Platform          |
    +-------------------+ 
    
    
## Sample Sub-Job

## Sub-Job Details
- **Sub-Job ID:** Job-2-subJob-3
- **FLOPs to be Executed:** 312,646

### Files to be Read
| File Path                            | Size (Bytes) |
|--------------------------------------|--------------|
| /input/user.input.2.00000109.root    | 10,005,685   |
| /input/user.input.2.0000011.root     | 10,001,028   |
| /input/user.input.2.00000110.root    | 9,989,540    |
| /input/user.input.2.00000111.root    | 9,994,482    |

### Files to be Written
| File Path                            | Size (Bytes) |
|--------------------------------------|--------------|
| /output/user.output.2.00000111.root  | 9,996,252    |
| /output/user.output.2.00000112.root  | 9,995,969    |
| /output/user.output.2.00000113.root  | 10,008,864   |
| /output/user.output.2.00000114.root  | 9,998,290    |
| /output/user.output.2.00000115.root  | 10,000,298   |


### Resource Usage
- **Cores Used:** 32
- **Disks Used:** CERN-PROD_DATADISK

### Hosts
- **Read Host:**    INFN-ROMA3_cpu-12
- **Write Host:**   INFN-ROMA3_cpu-12
- **Compute Host:** INFN-ROMA3_cpu-12



## SimGrid Output for Sub-Job

[INFN-ROMA3_cpu-12:Job-2-subJob-3:(34) 50279.824121] [ATLAS_SIMULATION/INFO] Finished reading file '/data/input/user.input.2.00000109.root' of size '10005685' on host 'INFN-ROMA3_cpu-12'

[INFN-ROMA3_cpu-12:Job-2-subJob-3:(34) 100536.249206] [ATLAS_SIMULATION/INFO] Finished reading file '/data/input/user.input.2.0000011.root' of size '10001028' on host 'INFN-ROMA3_cpu-12'

[INFN-ROMA3_cpu-12:Job-2-subJob-3:(34) 150734.937680] [ATLAS_SIMULATION/INFO] Finished reading file '/data/input/user.input.2.00000110.root' of size '9989540' on host 'INFN-ROMA3_cpu-12'

[INFN-ROMA3_cpu-12:Job-2-subJob-3:(34) 200958.467982] [ATLAS_SIMULATION/INFO] Finished reading file '/data/input/user.input.2.00000111.root' of size '9994482' on host 'INFN-ROMA3_cpu-12'

[INFN-ROMA3_cpu-12:Job-2-subJob-3:(34) 200958.468178] [ATLAS_SIMULATION/INFO] Finished executing '312646.000000' flops' on host 'INFN-ROMA3_cpu-12'

[INFN-ROMA3_cpu-12:Job-2-subJob-3:(34) 310807.396399] [ATLAS_SIMULATION/INFO] Finished writing file '/data/output/user.output.2.00000111.root' of size '9996252' on host 'INFN-ROMA3_cpu-12'

[INFN-ROMA3_cpu-12:Job-2-subJob-3:(34) 420653.248286] [ATLAS_SIMULATION/INFO] Finished writing file '/data/output/user.output.2.00000112.root' of size '9995969' on host 'INFN-ROMA3_cpu-12'

[INFN-ROMA3_cpu-12:Job-2-subJob-3:(34) 530640.754354] [ATLAS_SIMULATION/INFO] Finished writing file '/data/output/user.output.2.00000113.root' of size '10008864' on host 'INFN-ROMA3_cpu-12'

[INFN-ROMA3_cpu-12:Job-2-subJob-3:(34) 640512.065964] [ATLAS_SIMULATION/INFO] Finished writing file '/data/output/user.output.2.00000114.root' of size '9998290' on host 'INFN-ROMA3_cpu-12'

[INFN-ROMA3_cpu-12:Job-2-subJob-3:(34) 750405.459849] [ATLAS_SIMULATION/INFO] Finished writing file '/data/output/user.output.2.00000115.root' of size '10000298' on host 'INFN-ROMA3_cpu-12'
[790672.163145]

[host_energy/INFO] Energy consumption of host INFN-ROMA3_cpu-12: 50603018.491319 Joules

