#!/usr/bin/env python3
"""
Convert SNAP format files to DIMACS format.

SNAP format:
  # <comment>
  <vertex1> <vertex2>

DIMACS format:
  c <comment>
  p edge <num_vertices> <num_edges>
  e <vertex1> <vertex2>
"""

import os
import sys

def convert_snap_to_dimacs(input_file, output_file):
    """Convert a SNAP format file to DIMACS format."""
    edges = []
    comments = []
    vertices = set()
    
    with open(input_file, 'r') as f:
        for line in f:
            line = line.strip()
            
            if not line:
                continue
            
            if line.startswith('#'):
                # Comment line - convert '#' to 'c'
                comments.append('c' + line[1:])
            else:
                # Edge line: <u> <v>
                parts = line.split()
                if len(parts) >= 2:
                    try:
                        u = int(parts[0])
                        v = int(parts[1])
                        
                        # DIMACS requires 1-indexed vertices
                        # SNAP can be 0-indexed, so we'll remap
                        vertices.add(u)
                        vertices.add(v)
                        edges.append((u, v))
                    except ValueError:
                        # Skip lines that don't contain valid integers
                        continue
    
    # If vertices are 0-indexed, remap to 1-indexed
    min_vertex = min(vertices) if vertices else 0
    if min_vertex == 0:
        # Remap: add 1 to all vertex IDs
        edges = [(u + 1, v + 1) for u, v in edges]
        vertices = {v + 1 for v in vertices}
    
    num_vertices = len(vertices)
    num_edges = len(edges)
    
    # Write to output file in DIMACS format
    with open(output_file, 'w') as f:
        # Write comments
        if comments:
            for comment in comments:
                f.write(comment + '\n')
        else:
            # Add default comment if none exist
            f.write(f'c Converted from SNAP format\n')
        
        # Add metadata comment
        f.write(f'c Vertices: {num_vertices}, Edges: {num_edges}\n')
        
        # Write problem line
        f.write(f'p edge {num_vertices} {num_edges}\n')
        
        # Write edges
        for u, v in edges:
            f.write(f'e {u} {v}\n')
    
    return num_vertices, num_edges

def main():
    # Directories to convert
    directories = [
        'datasets/real_world',
        'datasets/synthetic'
    ]
    
    print("Converting SNAP format files to DIMACS format...\n")
    
    total_converted = 0
    
    for directory in directories:
        if not os.path.exists(directory):
            print(f"Warning: Directory '{directory}' not found, skipping...")
            continue
        
        # Get all .txt files in directory
        files = [f for f in os.listdir(directory) if f.endswith('.txt')]
        
        if not files:
            print(f"No .txt files found in '{directory}'")
            continue
        
        print(f"\n{'='*70}")
        print(f"Processing: {directory}")
        print(f"{'='*70}")
        
        for filename in sorted(files):
            input_path = os.path.join(directory, filename)
            
            # Check if file is in SNAP format (no 'p edge' line)
            is_snap = True
            with open(input_path, 'r') as f:
                for line in f:
                    if line.startswith('p edge'):
                        is_snap = False
                        break
            
            if is_snap:
                # Create temporary file
                temp_path = input_path + '.tmp'
                
                try:
                    num_vertices, num_edges = convert_snap_to_dimacs(input_path, temp_path)
                    
                    # Replace original file with converted one
                    os.replace(temp_path, input_path)
                    
                    print(f"✓ {filename:35s} {num_vertices:7d} vertices, {num_edges:8d} edges")
                    total_converted += 1
                    
                except Exception as e:
                    print(f"✗ {filename:35s} Error: {e}")
                    if os.path.exists(temp_path):
                        os.remove(temp_path)
            else:
                print(f"- {filename:35s} Already in DIMACS format, skipped")
    
    print(f"\n{'='*70}")
    print(f"Conversion complete: {total_converted} files converted to DIMACS format")
    print(f"{'='*70}")

if __name__ == '__main__':
    main()
