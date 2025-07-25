import os
import json
import pandas as pd
import time
import random

# ------------------------ Helper Functions ------------------------

def update_cfg(path, parameterValueDict):
    """
    Update the configuration file with the new parameters.
    """
    with open(path, 'r') as f:
        data = json.load(f)
    for parameter in parameterValueDict:
        data[parameter] = parameterValueDict[parameter]
    with open(path, 'w') as f:
        json.dump(data, f, indent=4)

def update_site_info(path, site, cpu_speed):
    """
    Update the site information in the JSON file.
    """
    with open(path, 'r') as f:
        data = json.load(f)
    data[site]['CPUSpeed'] = cpu_speed
    with open(path, 'w') as f:
        json.dump(data, f, indent=4)

def run_simulation(site, cpu_min_max, speed_precision, CPUSpeed, run_tag):
    """
    Run the simulator after updating the configuration and site info.
    
    Returns:
        single_core_mean_abs_error, multi_core_mean_abs_error
    """
    parameterValueDict = {
        "Num_of_Jobs": 10,
        "cpu_min_max": cpu_min_max,
        "cpu_speed_precision": speed_precision,
        "Sites": [site],
        "Output_DB": f"/home/sairam/ATLASGRIDV2/ATLAS-GRID-SIMULATION/output/NET2_jobs_output_{run_tag}.db",
        "Input_Job_CSV": "/home/sairam/ATLASGRID/ATLAS-GRID-SIMULATION/data/NET2_jobs_jan.csv"
    }
    update_cfg(config_path, parameterValueDict)
    update_site_info(site_info_path, site, CPUSpeed)
    
    # Run the simulator command.
    os.system(command)
    time.sleep(2)  # Allow time for the simulation to complete.
    
    # Read the output CSV file generated by the simulator.
    output_file_csv = f"/home/sairam/ATLASGRIDV2/ATLAS-GRID-SIMULATION/output/NET2_jobs_output_{run_tag}.csv"
    try:
        df_raw = pd.read_csv(output_file_csv)
    except Exception as e:
        print(f"Error reading output file for run {run_tag}: {e}")
        df_raw = None

    # Initialize error metrics.
    single_core_mean_abs_error = float('inf')
    multi_core_mean_abs_error = float('inf')

    if df_raw is not None:
        # Process single-core jobs.
        df_single = df_raw[(df_raw['STATUS'] == "finished") & (df_raw['CORES'] == 1)]
        if not df_single.empty:
            df_single['error'] = df_single['CPU_CONSUMPTION_TIME'] - df_single['EXECUTION_TIME']
            df_single['absolute_error'] = df_single['error'].abs()
            single_core_mean_abs_error = df_single['absolute_error'].mean()
        
        # Process multi-core jobs (with 8 cores).
        df_multi = df_raw[(df_raw['STATUS'] == "finished") & (df_raw['CORES'] == 8)]
        if not df_multi.empty:
            df_multi['error'] = (df_multi['CPU_CONSUMPTION_TIME'] - df_multi['EXECUTION_TIME']) / 8
            df_multi['absolute_error'] = df_multi['error'].abs()
            multi_core_mean_abs_error = df_multi['absolute_error'].mean()
    
    # Clean up the output file.
    output_file = f"/home/sairam/ATLASGRIDV2/ATLAS-GRID-SIMULATION/output/NET2_jobs_output_{run_tag}.db"
    if os.path.exists(output_file):
        os.remove(output_file)
        print(f"Deleted output file: {output_file}")
    
    return single_core_mean_abs_error, multi_core_mean_abs_error

# ------------------------ Global Settings ------------------------

sites = ["NET2_Amherst"]
config_path = "/home/sairam/ATLASGRIDV2/ATLAS-GRID-SIMULATION/config-files/config.json"
site_info_path = "/home/sairam/ATLASGRIDV2/ATLAS-GRID-SIMULATION/data/site_info_cpu.json"
command = "/home/sairam/ATLASGRIDV2/ATLAS-GRID-SIMULATION/build/atlas-grid-simulator -c /home/sairam/ATLASGRIDV2/ATLAS-GRID-SIMULATION/config-files/config.json"

# Load site_info which should include "CPUCount" for each site.
with open(site_info_path, 'r') as f:
    site_info = json.load(f)

# ------------------------ Random Search Implementation ------------------------

def random_search(num_trials):
    """
    Randomly search the parameter space for calibration.
    For each trial:
      - min_val is chosen randomly from [1, 8]
      - range_offset is chosen to ensure that cpu_min_max's upper bound does not exceed 9.
      - speed_precision is chosen randomly from the set {5,6,7,8,9,10,11,12}.
    The function runs the simulation and computes the average error from single-core and multi-core results.
    The best configuration (with the lowest average error) is printed out.
    """
    best_error = float('inf')
    best_config = None
    possible_precisions = [5, 6, 7, 8, 9, 10, 11, 12]
    
    for trial in range(num_trials):
        # Sample min_val from [1, 8].
        min_val = random.randint(1, 8)
        # Calculate the maximum possible range_offset so that the upper bound <= 9.
        max_offset = 8 - min_val  # since upper_bound = min_val + 1 + range_offset <= 9  ==> range_offset <= 8 - min_val
        range_offset = random.randint(0, max_offset)
        upper_bound = min_val + 1 + range_offset
        
        cpu_min_max = [min_val, upper_bound]
        speed_precision = random.choice(possible_precisions)
        
        site = "NET2_Amherst"
        # Generate a CPUSpeed ordering based on the current configuration.
        CPUSpeed = [
            random.randint(min_val, upper_bound) * (10 ** speed_precision)
            for _ in range(site_info[site]['CPUCount'])
        ]
        
        run_tag = f"random_{trial}"
        single_err, multi_err = run_simulation(site, cpu_min_max, speed_precision, CPUSpeed, run_tag)
        avg_error = (single_err + multi_err) / 2.0
        
        print(f"Trial {trial}: cpu_min_max={cpu_min_max}, speed_precision={speed_precision} --> Avg Error: {avg_error}")
        
        if avg_error < best_error:
            best_error = avg_error
            best_config = {
                "trial": trial,
                "cpu_min_max": cpu_min_max,
                "speed_precision": speed_precision,
                "avg_error": avg_error
            }
    
    print("\nBest configuration found:")
    print(best_config)

# ------------------------ Main Routine ------------------------

if __name__ == '__main__':
    num_trials = 30  # Adjust number of random trials as needed.
    random_search(num_trials)
