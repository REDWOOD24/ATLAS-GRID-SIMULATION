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
  ./atlas-grid-simulator ../data/site_conn_info.json ../data/site_info.json
   ```

## Initializing Sites (Sample)

## Site Details

### AGLT2

**CPUs:** 1775

**Disk Information:**

- **CALIBDISK:** 152.00 GiB
- **DATADISK:** 6332.00 GiB
- **LOCALGROUPDISK:** 197.00 GiB
- **SCRATCHDISK:** 96.00 GiB

---

### Notes

- Information obtained from site data dumps.




## Job Submission Report (Sample Job)

## Job Details
- **Job ID:** Job-2-subJob-5
- **FLOPs to be Executed:** 454,128

### Files to be Read
| File Path                            | Size (Bytes) |
|--------------------------------------|--------------|
| /input/user.input.00000182.root      | 9,989,963    |
| /input/user.input.00000192.root      | 9,983,035    |
| /input/user.input.00000202.root      | 10,000,405   |
| /input/user.input.00000212.root      | 9,989,740    |
| /input/user.input.0000022.root       | 10,007,622   |

### Files to be Written
| File Path                            | Size (Bytes) |
|--------------------------------------|--------------|
| /output/user.output.00000242.root    | 9,994,251    |
| /output/user.output.00000252.root    | 10,002,685   |
| /output/user.output.00000262.root    | 9,998,317    |
| /output/user.output.00000272.root    | 9,999,734    |

### Resource Usage
- **Cores Used:** 99
- **Disks Used:** LOCALGROUPDISK

### Hosts
- **Read Host:** GR-12-TEIKAV_cpu-42
- **Write Host:** GR-12-TEIKAV_cpu-42
- **Compute Host:** GR-12-TEIKAV_cpu-42

## SimGrid Job Output

[GR-12-TEIKAV_cpu-42:Job-2-subJob-5:(29) 45616.281987] [ATLAS_SIMULATION/INFO] Finished reading file '/local/input/user.input.00000182.root' of size '9989963' on host 'GR-12-TEIKAV_cpu-42'
[GR-12-TEIKAV_cpu-42:Job-2-subJob-5:(29) 91200.922929] [ATLAS_SIMULATION/INFO] Finished reading file '/local/input/user.input.00000192.root' of size '9983035' on host 'GR-12-TEIKAV_cpu-42'
[GR-12-TEIKAV_cpu-42:Job-2-subJob-5:(29) 136864.872146] [ATLAS_SIMULATION/INFO] Finished reading file '/local/input/user.input.00000202.root' of size '10000405' on host 'GR-12-TEIKAV_cpu-42'
[GR-12-TEIKAV_cpu-42:Job-2-subJob-5:(29) 182480.137557] [ATLAS_SIMULATION/INFO] Finished reading file '/local/input/user.input.00000212.root' of size '9989740' on host 'GR-12-TEIKAV_cpu-42'
[GR-12-TEIKAV_cpu-42:Job-2-subJob-5:(29) 228177.041616] [ATLAS_SIMULATION/INFO] Finished reading file '/local/input/user.input.0000022.root' of size '10007622' on host 'GR-12-TEIKAV_cpu-42'
[GR-12-TEIKAV_cpu-42:Job-2-subJob-5:(29) 228177.041965] [ATLAS_SIMULATION/INFO] Finished executing '454128.000000' flops' on host 'GR-12-TEIKAV_cpu-42'
[GR-12-TEIKAV_cpu-42:Job-2-subJob-5:(29) 315083.586959] [ATLAS_SIMULATION/INFO] Finished writing file '/local/output/user.output.00000242.root' of size '9994251' on host 'GR-12-TEIKAV_cpu-42'
[GR-12-TEIKAV_cpu-42:Job-2-subJob-5:(29) 402063.473064] [ATLAS_SIMULATION/INFO] Finished writing file '/local/output/user.output.00000252.root' of size '10002685' on host 'GR-12-TEIKAV_cpu-42'
[GR-12-TEIKAV_cpu-42:Job-2-subJob-5:(29) 489005.344459] [ATLAS_SIMULATION/INFO] Finished writing file '/local/output/user.output.00000262.root' of size '9998317' on host 'GR-12-TEIKAV_cpu-42'
[GR-12-TEIKAV_cpu-42:Job-2-subJob-5:(29) 575959.544538] [ATLAS_SIMULATION/INFO] Finished writing file '/local/output/user.output.00000272.root' of size '9999734' on host 'GR-12-TEIKAV_cpu-42'