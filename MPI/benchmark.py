#!/usr/bin/env python3
"""
Benchmark script for MPI Pi estimation programs.
Runs sequential, parallel, and spawned versions with varying worker counts and dataset sizes.
Measures execution time and stores results for analysis.
"""

import subprocess
import re
import json
import os
from datetime import datetime
from pathlib import Path
from collections import defaultdict

# Configuration
TOTAL_POINTS_LIST = [10000000, 50000000, 100000000]  # Dataset sizes
WORKER_COUNTS = [1, 2, 4, 6, 8]  # Number of processes/workers
NUM_RUNS = 2  # Number of runs per configuration
BIN_DIR = "bin"
RESULTS_DIR = "results"
TIMESTAMP = datetime.now().strftime("%Y%m%d_%H%M%S")

# Regular expression to extract execution time from program output
TIME_REGEX = r"Execution Time:\s+([\d.]+)\s+seconds"

def run_command(cmd, timeout=300):
    """Execute a shell command and return stdout+stderr and return code."""
    try:
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=timeout)
        return result.stdout + result.stderr, result.returncode
    except subprocess.TimeoutExpired:
        return "TIMEOUT", -1
    except Exception as e:
        return str(e), -1

def extract_execution_time(output):
    """Extract execution time from program output."""
    match = re.search(TIME_REGEX, output)
    if match:
        try:
            return float(match.group(1))
        except ValueError:
            return None
    return None

def benchmark_sequential(total_points, num_runs):
    """Benchmark sequential version."""
    print(f"  Sequential (total_points={total_points:,})...")
    times = []
    
    for run in range(num_runs):
        cmd = f"make TOTAL_POINTS={total_points} -B sequential > /dev/null 2>&1 && ./{BIN_DIR}/sequential"
        output, code = run_command(cmd)
        
        if code != 0:
            print(f"    Run {run + 1}/{num_runs}: FAILED")
            continue
        
        exe_time = extract_execution_time(output)
        if exe_time is not None:
            times.append(exe_time)
            print(f"    Run {run + 1}/{num_runs}: {exe_time:.6f}s")
        else:
            print(f"    Run {run + 1}/{num_runs}: Failed to extract time")
    
    if times:
        avg_time = sum(times) / len(times)
        print(f"  Average: {avg_time:.6f}s")
        return avg_time
    return None

def benchmark_parallel(total_points, worker_counts, num_runs):
    """Benchmark parallel (MPI static) version."""
    print(f"  Parallel (total_points={total_points:,})...")
    results = {}
    
    for workers in worker_counts:
        print(f"    {workers} process(es)...", end=" ")
        times = []
        
        for run in range(num_runs):
            cmd = f"make TOTAL_POINTS={total_points} -B parallel > /dev/null 2>&1 && mpirun --oversubscribe -np {workers} ./{BIN_DIR}/parallel"
            output, code = run_command(cmd)
            
            if code != 0:
                continue
            
            exe_time = extract_execution_time(output)
            if exe_time is not None:
                times.append(exe_time)
        
        if times:
            avg_time = sum(times) / len(times)
            results[workers] = avg_time
            print(f"avg={avg_time:.6f}s")
        else:
            print("FAILED")
    
    return results

def benchmark_spawned(total_points, worker_counts, num_runs):
    """Benchmark spawned (MPI dynamic) version."""
    print(f"  Spawned (total_points={total_points:,})...")
    results = {}
    
    for workers in worker_counts:
        print(f"    {workers} worker(s)...", end=" ")
        times = []
        
        for run in range(num_runs):
            cmd = f"make TOTAL_POINTS={total_points} -B spawned spawned_worker > /dev/null 2>&1 && mpirun --oversubscribe -np 1 ./{BIN_DIR}/spawned {workers}"
            output, code = run_command(cmd)
            
            if code != 0:
                continue
            
            exe_time = extract_execution_time(output)
            if exe_time is not None:
                times.append(exe_time)
        
        if times:
            avg_time = sum(times) / len(times)
            results[workers] = avg_time
            print(f"avg={avg_time:.6f}s")
        else:
            print("FAILED")
    
    return results

def save_results(all_results):
    """Save benchmark results to JSON file."""
    Path(RESULTS_DIR).mkdir(parents=True, exist_ok=True)
    
    results_file = os.path.join(RESULTS_DIR, f"benchmark_{TIMESTAMP}.json")
    with open(results_file, 'w') as f:
        json.dump(all_results, f, indent=2)
    
    print(f"\n✓ Results saved to: {results_file}")
    return results_file

def print_summary(all_results):
    """Print a summary table of results."""
    print("\n" + "="*80)
    print("BENCHMARK SUMMARY")
    print("="*80)
    
    for total_points in TOTAL_POINTS_LIST:
        print(f"\nDataset Size: {total_points:,} points")
        print("-" * 80)
        
        seq_time = all_results['sequential'].get(total_points)
        if seq_time:
            print(f"  Sequential:  {seq_time:.6f}s")
        
        parallel_results = all_results['parallel'].get(total_points, {})
        if parallel_results:
            print(f"  Parallel (MPI Static):")
            for workers in sorted(parallel_results.keys()):
                time = parallel_results[workers]
                speedup = seq_time / time if seq_time else 0
                efficiency = (speedup / workers * 100) if seq_time else 0
                print(f"    {workers:2d} worker(s): {time:.6f}s  (speedup: {speedup:.2f}x, efficiency: {efficiency:.1f}%)")
        
        spawned_results = all_results['spawned'].get(total_points, {})
        if spawned_results:
            print(f"  Spawned (MPI Dynamic):")
            for workers in sorted(spawned_results.keys()):
                time = spawned_results[workers]
                speedup = seq_time / time if seq_time else 0
                efficiency = (speedup / workers * 100) if seq_time else 0
                print(f"    {workers:2d} worker(s): {time:.6f}s  (speedup: {speedup:.2f}x, efficiency: {efficiency:.1f}%)")

def main():
    print("\n" + "="*80)
    print("MPI Pi Estimation - Comprehensive Benchmark")
    print("="*80)
    print(f"\nConfiguration:")
    print(f"  Dataset sizes: {TOTAL_POINTS_LIST}")
    print(f"  Worker counts: {WORKER_COUNTS}")
    print(f"  Runs per config: {NUM_RUNS}")
    print(f"  Timestamp: {TIMESTAMP}\n")
    
    # Verify binaries exist
    if not os.path.exists(BIN_DIR):
        print(f"ERROR: {BIN_DIR}/ directory not found. Please run 'make all' first.")
        return
    
    all_results = {
        'sequential': {},
        'parallel': {},
        'spawned': {}
    }
    
    # Benchmark all dataset sizes
    for total_points in TOTAL_POINTS_LIST:
        print(f"\n{'='*80}")
        print(f"Benchmarking with {total_points:,} total points")
        print(f"{'='*80}\n")
        
        # Sequential
        print("Sequential Version:")
        seq_time = benchmark_sequential(total_points, NUM_RUNS)
        if seq_time:
            all_results['sequential'][total_points] = seq_time
        
        # Parallel
        print("\nParallel Version (MPI Static):")
        par_results = benchmark_parallel(total_points, WORKER_COUNTS, NUM_RUNS)
        if par_results:
            all_results['parallel'][total_points] = par_results
        
        # Spawned
        print("\nSpawned Version (MPI Dynamic):")
        spawn_results = benchmark_spawned(total_points, WORKER_COUNTS, NUM_RUNS)
        if spawn_results:
            all_results['spawned'][total_points] = spawn_results
    
    # Save and display results
    results_file = save_results(all_results)
    print_summary(all_results)
    
    print("\n" + "="*80)
    print("Benchmark Complete!")
    print(f"Results file: {results_file}")
    print(f"Run 'python3 plot_results.py {results_file}' to generate plots")
    print("="*80 + "\n")

if __name__ == "__main__":
    main()
