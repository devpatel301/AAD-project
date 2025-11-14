#!/usr/bin/env python3
"""
Analysis script for Maximum Clique benchmark results
"""

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import sys
import os

def load_results(csv_file):
    """Load benchmark results from CSV"""
    if not os.path.exists(csv_file):
        print(f"Error: Results file not found: {csv_file}")
        sys.exit(1)
    
    df = pd.read_csv(csv_file)
    print(f"Loaded {len(df)} benchmark results")
    print(f"Datasets: {df['dataset'].nunique()}")
    print(f"Algorithms: {df['algorithm'].nunique()}")
    return df

def print_summary(df):
    """Print summary statistics"""
    print("\n" + "="*70)
    print("BENCHMARK SUMMARY")
    print("="*70)
    
    for dataset in df['dataset'].unique():
        print(f"\nDataset: {dataset}")
        ds_data = df[df['dataset'] == dataset]
        
        print(f"  Graph properties:")
        print(f"    Vertices: {ds_data['num_vertices'].iloc[0]}")
        print(f"    Edges: {ds_data['num_edges'].iloc[0]}")
        print(f"    Density: {ds_data['density'].iloc[0]:.6f}")
        
        print(f"\n  Algorithm Results:")
        for _, row in ds_data.iterrows():
            status = "✓" if row['valid'] else "✗"
            print(f"    {status} {row['algorithm']:25s} | "
                  f"Clique: {row['clique_size']:3d} | "
                  f"Time: {row['time_ms']:10.2f} ms")

def plot_clique_sizes(df, output_dir):
    """Plot comparison of clique sizes found by each algorithm"""
    plt.figure(figsize=(12, 6))
    
    pivot = df.pivot(index='dataset', columns='algorithm', values='clique_size')
    pivot.plot(kind='bar', figsize=(12, 6))
    
    plt.title('Maximum Clique Size by Algorithm', fontsize=14, fontweight='bold')
    plt.ylabel('Clique Size', fontsize=12)
    plt.xlabel('Dataset', fontsize=12)
    plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    plt.tight_layout()
    
    output_file = os.path.join(output_dir, 'clique_sizes.png')
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"Saved: {output_file}")
    plt.close()

def plot_execution_times(df, output_dir):
    """Plot execution time comparison (log scale)"""
    plt.figure(figsize=(12, 6))
    
    pivot_time = df.pivot(index='dataset', columns='algorithm', values='time_ms')
    pivot_time.plot(kind='bar', figsize=(12, 6), logy=True)
    
    plt.title('Execution Time by Algorithm (Log Scale)', fontsize=14, fontweight='bold')
    plt.ylabel('Time (ms, log scale)', fontsize=12)
    plt.xlabel('Dataset', fontsize=12)
    plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    plt.tight_layout()
    
    output_file = os.path.join(output_dir, 'execution_times.png')
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"Saved: {output_file}")
    plt.close()

def plot_time_vs_quality(df, output_dir):
    """Plot time vs clique size tradeoff"""
    plt.figure(figsize=(10, 8))
    
    for dataset in df['dataset'].unique():
        ds_data = df[df['dataset'] == dataset]
        plt.scatter(ds_data['time_ms'], ds_data['clique_size'], 
                   label=dataset, s=100, alpha=0.7)
        
        # Annotate algorithm names
        for _, row in ds_data.iterrows():
            plt.annotate(row['algorithm'], 
                        (row['time_ms'], row['clique_size']),
                        fontsize=8, alpha=0.7)
    
    plt.xscale('log')
    plt.xlabel('Execution Time (ms, log scale)', fontsize=12)
    plt.ylabel('Clique Size', fontsize=12)
    plt.title('Quality vs Speed Tradeoff', fontsize=14, fontweight='bold')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    
    output_file = os.path.join(output_dir, 'quality_vs_speed.png')
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"Saved: {output_file}")
    plt.close()

def plot_speedup_comparison(df, output_dir):
    """Plot relative speedup compared to slowest algorithm"""
    plt.figure(figsize=(12, 6))
    
    # Calculate speedup relative to slowest algorithm for each dataset
    speedups = []
    
    for dataset in df['dataset'].unique():
        ds_data = df[df['dataset'] == dataset].copy()
        max_time = ds_data['time_ms'].max()
        ds_data['speedup'] = max_time / ds_data['time_ms']
        speedups.append(ds_data)
    
    speedup_df = pd.concat(speedups)
    pivot = speedup_df.pivot(index='dataset', columns='algorithm', values='speedup')
    
    pivot.plot(kind='bar', figsize=(12, 6), logy=True)
    plt.title('Relative Speedup (compared to slowest)', fontsize=14, fontweight='bold')
    plt.ylabel('Speedup Factor (log scale)', fontsize=12)
    plt.xlabel('Dataset', fontsize=12)
    plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    plt.axhline(y=1, color='r', linestyle='--', alpha=0.5, label='Baseline')
    plt.tight_layout()
    
    output_file = os.path.join(output_dir, 'speedup_comparison.png')
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"Saved: {output_file}")
    plt.close()

def generate_latex_table(df, output_dir):
    """Generate LaTeX table for paper/report"""
    latex = []
    latex.append("\\begin{table}[htbp]")
    latex.append("\\centering")
    latex.append("\\caption{Maximum Clique Benchmark Results}")
    latex.append("\\begin{tabular}{lrrrrr}")
    latex.append("\\hline")
    latex.append("Algorithm & Dataset & Vertices & Clique & Time (ms) & Valid \\\\")
    latex.append("\\hline")
    
    for _, row in df.iterrows():
        latex.append(f"{row['algorithm']} & {row['dataset']} & "
                    f"{row['num_vertices']} & {row['clique_size']} & "
                    f"{row['time_ms']:.2f} & "
                    f"{'✓' if row['valid'] else '✗'} \\\\")
    
    latex.append("\\hline")
    latex.append("\\end{tabular}")
    latex.append("\\end{table}")
    
    output_file = os.path.join(output_dir, 'results_table.tex')
    with open(output_file, 'w') as f:
        f.write('\n'.join(latex))
    
    print(f"Saved: {output_file}")

def main():
    if len(sys.argv) < 2:
        print("Usage: python analyze_results.py <results.csv>")
        print("Example: python analyze_results.py results/benchmark_results.csv")
        sys.exit(1)
    
    csv_file = sys.argv[1]
    output_dir = os.path.dirname(csv_file) or 'results'
    
    # Load results
    df = load_results(csv_file)
    
    # Print summary
    print_summary(df)
    
    # Generate plots
    print("\n" + "="*70)
    print("GENERATING VISUALIZATIONS")
    print("="*70 + "\n")
    
    plot_clique_sizes(df, output_dir)
    plot_execution_times(df, output_dir)
    plot_time_vs_quality(df, output_dir)
    plot_speedup_comparison(df, output_dir)
    
    # Generate LaTeX table
    print("\n" + "="*70)
    print("GENERATING LATEX TABLE")
    print("="*70 + "\n")
    generate_latex_table(df, output_dir)
    
    print("\n" + "="*70)
    print("ANALYSIS COMPLETE")
    print("="*70)
    print(f"\nAll outputs saved to: {output_dir}/")

if __name__ == '__main__':
    main()
