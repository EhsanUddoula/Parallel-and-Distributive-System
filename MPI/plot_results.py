#!/usr/bin/env python3
"""
Plot results from MPI Pi estimation benchmarks.
Generates comparison plots for execution time vs number of workers.
"""

import json
import sys
import os
from pathlib import Path

try:
    import matplotlib.pyplot as plt
    import numpy as np
except ImportError:
    print("ERROR: matplotlib or numpy not found.")
    print("Install with: pip3 install matplotlib numpy")
    sys.exit(1)

# Configuration
RESULTS_DIR = "results"
PLOTS_DIR = "plots"
COLORS = {
    'sequential': '#1f77b4',
    'parallel': '#ff7f0e',
    'spawned': '#2ca02c'
}
MARKERS = {
    'sequential': 'o',
    'parallel': 's',
    'spawned': '^'
}

def load_results(results_file):
    """Load results from JSON file."""
    if not os.path.exists(results_file):
        print(f"ERROR: Results file '{results_file}' not found.")
        sys.exit(1)
    
    with open(results_file, 'r') as f:
        return json.load(f)

def generate_plots(results, output_dir):
    """Generate comparison plots from benchmark results."""
    Path(output_dir).mkdir(parents=True, exist_ok=True)
    
    # Extract data
    total_points_list = sorted(results['sequential'].keys())
    if isinstance(total_points_list[0], str):
        total_points_list = sorted([int(x) for x in total_points_list])
    
    # Create figure with subplots
    num_datasets = len(total_points_list)
    fig, axes = plt.subplots(1, num_datasets, figsize=(6 * num_datasets, 5))
    if num_datasets == 1:
        axes = [axes]
    
    fig.suptitle('MPI Pi Estimation - Execution Time vs Number of Workers', fontsize=14, fontweight='bold')
    
    # Plot for each dataset size
    for idx, total_points in enumerate(total_points_list):
        ax = axes[idx]
        total_points_key = str(total_points)
        
        # Sequential baseline
        seq_time = results['sequential'].get(total_points_key)
        if seq_time:
            ax.axhline(y=seq_time, color=COLORS['sequential'], linestyle='--', linewidth=2, label='Sequential', alpha=0.7)
        
        # Parallel results
        parallel_data = results['parallel'].get(total_points_key, {})
        if parallel_data:
            workers = sorted([int(w) for w in parallel_data.keys()])
            times = [parallel_data[str(w)] for w in workers]
            ax.plot(workers, times, color=COLORS['parallel'], marker=MARKERS['parallel'], 
                   linewidth=2, markersize=8, label='Parallel (Static MPI)', alpha=0.8)
        
        # Spawned results
        spawned_data = results['spawned'].get(total_points_key, {})
        if spawned_data:
            workers = sorted([int(w) for w in spawned_data.keys()])
            times = [spawned_data[str(w)] for w in workers]
            ax.plot(workers, times, color=COLORS['spawned'], marker=MARKERS['spawned'], 
                   linewidth=2, markersize=8, label='Spawned (Dynamic MPI)', alpha=0.8)
        
        # Formatting
        ax.set_xlabel('Number of Workers', fontsize=11)
        ax.set_ylabel('Execution Time (seconds)', fontsize=11)
        ax.set_title(f'Total Points: {total_points:,}', fontsize=12, fontweight='bold')
        ax.grid(True, alpha=0.3)
        ax.legend(fontsize=10)
        if parallel_data or spawned_data:
            ax.set_xticks(workers if workers else [1, 2, 4, 6, 8])
    
    plt.tight_layout()
    output_file = os.path.join(output_dir, "execution_time_comparison.png")
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"✓ Plot saved: {output_file}")
    plt.close()

def generate_speedup_plots(results, output_dir):
    """Generate speedup plots comparing all versions."""
    Path(output_dir).mkdir(parents=True, exist_ok=True)
    
    # Extract data
    total_points_list = sorted(results['sequential'].keys())
    if isinstance(total_points_list[0], str):
        total_points_list = sorted([int(x) for x in total_points_list])
    
    num_datasets = len(total_points_list)
    fig, axes = plt.subplots(1, num_datasets, figsize=(6 * num_datasets, 5))
    if num_datasets == 1:
        axes = [axes]
    
    fig.suptitle('MPI Pi Estimation - Speedup vs Number of Workers', fontsize=14, fontweight='bold')
    
    for idx, total_points in enumerate(total_points_list):
        ax = axes[idx]
        total_points_key = str(total_points)
        
        seq_time = results['sequential'].get(total_points_key)
        if not seq_time:
            continue
        
        # Parallel speedup
        parallel_data = results['parallel'].get(total_points_key, {})
        if parallel_data:
            workers = sorted([int(w) for w in parallel_data.keys()])
            speedups = [seq_time / parallel_data[str(w)] for w in workers]
            ax.plot(workers, speedups, color=COLORS['parallel'], marker=MARKERS['parallel'], 
                   linewidth=2, markersize=8, label='Parallel (Static MPI)', alpha=0.8)
        
        # Spawned speedup
        spawned_data = results['spawned'].get(total_points_key, {})
        if spawned_data:
            workers = sorted([int(w) for w in spawned_data.keys()])
            speedups = [seq_time / spawned_data[str(w)] for w in workers]
            ax.plot(workers, speedups, color=COLORS['spawned'], marker=MARKERS['spawned'], 
                   linewidth=2, markersize=8, label='Spawned (Dynamic MPI)', alpha=0.8)
        
        # Ideal speedup (linear)
        if workers:
            max_workers = max(workers)
            ideal_workers = list(range(1, max_workers + 1))
            ax.plot(ideal_workers, ideal_workers, 'k--', linewidth=1.5, label='Ideal (Linear)', alpha=0.5)
        
        # Formatting
        ax.set_xlabel('Number of Workers', fontsize=11)
        ax.set_ylabel('Speedup', fontsize=11)
        ax.set_title(f'Total Points: {total_points:,}', fontsize=12, fontweight='bold')
        ax.grid(True, alpha=0.3)
        ax.legend(fontsize=10)
        if workers:
            ax.set_xticks(workers)
    
    plt.tight_layout()
    output_file = os.path.join(output_dir, "speedup_comparison.png")
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"✓ Plot saved: {output_file}")
    plt.close()

def generate_efficiency_plots(results, output_dir):
    """Generate parallel efficiency plots."""
    Path(output_dir).mkdir(parents=True, exist_ok=True)
    
    # Extract data
    total_points_list = sorted(results['sequential'].keys())
    if isinstance(total_points_list[0], str):
        total_points_list = sorted([int(x) for x in total_points_list])
    
    num_datasets = len(total_points_list)
    fig, axes = plt.subplots(1, num_datasets, figsize=(6 * num_datasets, 5))
    if num_datasets == 1:
        axes = [axes]
    
    fig.suptitle('MPI Pi Estimation - Parallel Efficiency vs Number of Workers', fontsize=14, fontweight='bold')
    
    for idx, total_points in enumerate(total_points_list):
        ax = axes[idx]
        total_points_key = str(total_points)
        
        seq_time = results['sequential'].get(total_points_key)
        if not seq_time:
            continue
        
        # Parallel efficiency
        parallel_data = results['parallel'].get(total_points_key, {})
        if parallel_data:
            workers = sorted([int(w) for w in parallel_data.keys()])
            efficiencies = [(seq_time / parallel_data[str(w)] / w * 100) for w in workers]
            ax.plot(workers, efficiencies, color=COLORS['parallel'], marker=MARKERS['parallel'], 
                   linewidth=2, markersize=8, label='Parallel (Static MPI)', alpha=0.8)
        
        # Spawned efficiency
        spawned_data = results['spawned'].get(total_points_key, {})
        if spawned_data:
            workers = sorted([int(w) for w in spawned_data.keys()])
            efficiencies = [(seq_time / spawned_data[str(w)] / w * 100) for w in workers]
            ax.plot(workers, efficiencies, color=COLORS['spawned'], marker=MARKERS['spawned'], 
                   linewidth=2, markersize=8, label='Spawned (Dynamic MPI)', alpha=0.8)
        
        # Ideal efficiency (100%)
        if workers:
            ax.axhline(y=100, color='k', linestyle='--', linewidth=1.5, label='Ideal (100%)', alpha=0.5)
        
        # Formatting
        ax.set_xlabel('Number of Workers', fontsize=11)
        ax.set_ylabel('Parallel Efficiency (%)', fontsize=11)
        ax.set_title(f'Total Points: {total_points:,}', fontsize=12, fontweight='bold')
        ax.set_ylim([0, 120])
        ax.grid(True, alpha=0.3)
        ax.legend(fontsize=10)
        if workers:
            ax.set_xticks(workers)
    
    plt.tight_layout()
    output_file = os.path.join(output_dir, "efficiency_comparison.png")
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"✓ Plot saved: {output_file}")
    plt.close()

def generate_summary_table(results, output_dir):
    """Generate a text summary of all results."""
    Path(output_dir).mkdir(parents=True, exist_ok=True)
    
    total_points_list = sorted(results['sequential'].keys())
    if isinstance(total_points_list[0], str):
        total_points_list = sorted([int(x) for x in total_points_list])
    
    summary_file = os.path.join(output_dir, "summary.txt")
    with open(summary_file, 'w') as f:
        f.write("="*100 + "\n")
        f.write("MPI Pi Estimation - Benchmark Results Summary\n")
        f.write("="*100 + "\n\n")
        
        for total_points in total_points_list:
            total_points_key = str(total_points)
            f.write(f"\nTotal Points: {total_points:,}\n")
            f.write("-"*100 + "\n")
            
            seq_time = results['sequential'].get(total_points_key)
            if seq_time:
                f.write(f"Sequential Time: {seq_time:.6f}s\n\n")
            
            # Parallel results
            parallel_data = results['parallel'].get(total_points_key, {})
            if parallel_data:
                f.write("Parallel (Static MPI):\n")
                f.write(f"{'Workers':<10} {'Time (s)':<15} {'Speedup':<12} {'Efficiency (%)':<15}\n")
                f.write("-"*100 + "\n")
                for w in sorted([int(x) for x in parallel_data.keys()]):
                    time = parallel_data[str(w)]
                    speedup = seq_time / time if seq_time else 0
                    efficiency = (speedup / w * 100) if seq_time else 0
                    f.write(f"{w:<10} {time:<15.6f} {speedup:<12.4f} {efficiency:<15.2f}\n")
                f.write("\n")
            
            # Spawned results
            spawned_data = results['spawned'].get(total_points_key, {})
            if spawned_data:
                f.write("Spawned (Dynamic MPI):\n")
                f.write(f"{'Workers':<10} {'Time (s)':<15} {'Speedup':<12} {'Efficiency (%)':<15}\n")
                f.write("-"*100 + "\n")
                for w in sorted([int(x) for x in spawned_data.keys()]):
                    time = spawned_data[str(w)]
                    speedup = seq_time / time if seq_time else 0
                    efficiency = (speedup / w * 100) if seq_time else 0
                    f.write(f"{w:<10} {time:<15.6f} {speedup:<12.4f} {efficiency:<15.2f}\n")
                f.write("\n")
        
        f.write("="*100 + "\n")
        f.write("Metrics Explanation:\n")
        f.write("  Speedup = Sequential Time / Parallel Time\n")
        f.write("  Efficiency = (Speedup / Number of Workers) * 100%\n")
        f.write("  Ideal speedup is linear (speedup = number of workers, efficiency = 100%)\n")
        f.write("="*100 + "\n")
    
    print(f"✓ Summary saved: {summary_file}")

def main():
    if len(sys.argv) < 2:
        # Find the latest results file
        if not os.path.exists(RESULTS_DIR):
            print(f"ERROR: No results directory found. Run 'python3 benchmark.py' first.")
            sys.exit(1)
        
        results_files = sorted(Path(RESULTS_DIR).glob("benchmark_*.json"))
        if not results_files:
            print(f"ERROR: No benchmark results found in {RESULTS_DIR}/")
            sys.exit(1)
        
        results_file = str(results_files[-1])
        print(f"Using latest results file: {results_file}")
    else:
        results_file = sys.argv[1]
    
    print("\n" + "="*80)
    print("Generating Plots from Benchmark Results")
    print("="*80 + "\n")
    
    results = load_results(results_file)
    
    print("Generating execution time comparison plots...")
    generate_plots(results, PLOTS_DIR)
    
    print("Generating speedup comparison plots...")
    generate_speedup_plots(results, PLOTS_DIR)
    
    print("Generating parallel efficiency plots...")
    generate_efficiency_plots(results, PLOTS_DIR)
    
    print("Generating summary table...")
    generate_summary_table(results, PLOTS_DIR)
    
    print("\n" + "="*80)
    print("Plot Generation Complete!")
    print(f"Plots saved to: {PLOTS_DIR}/")
    print("="*80 + "\n")

if __name__ == "__main__":
    main()
