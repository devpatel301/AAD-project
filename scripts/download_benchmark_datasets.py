import requests
import os
import gzip
import io
import networkx as nx # Using networkx to easily handle graph conversion
from typing import Dict

# --- 1. Dataset Configuration ---

# Dictionary mapping a unique ID to the download URL and the file type.
DATASET_CONFIG: Dict[str, Dict[str, str]] = {
    # SNAP datasets (Compressed text files, require conversion to DIMACS)
    "SNAP_ca-GrQc":          {"url": "https://snap.stanford.edu/data/ca-GrQc.txt.gz", "type": "snap"},
    "SNAP_email-Eu-core":    {"url": "https://snap.stanford.edu/data/email-Eu-core.txt.gz", "type": "snap"},
    "SNAP_socfb-Bingham82":  {"url": "https://snap.stanford.edu/data/socfb-Bingham82.txt.gz", "type": "snap"},

    # DIMACS datasets (Confirmed URLs, already in DIMACS format)
    "DIMACS_C125.9":         {"url": "https://iridia.ulb.ac.be/~fmascia/files/DIMACS/C125.9.clq", "type": "dimacs"},
    "DIMACS_C250.9":         {"url": "https://iridia.ulb.ac.be/~fmascia/files/DIMACS/C250.9.clq", "type": "dimacs"},
    "DIMACS_brock200_1":     {"url": "https://iridia.ulb.ac.be/~fmascia/files/DIMACS/brock200_1.clq", "type": "dimacs"},
    "DIMACS_brock400_2":     {"url": "https://iridia.ulb.ac.be/~fmascia/files/DIMACS/brock400_2.clq", "type": "dimacs"},
    "DIMACS_hamming6-4":     {"url": "https://iridia.ulb.ac.be/~fmascia/files/DIMACS/hamming6-4.clq", "type": "dimacs"},
    "DIMACS_gen200_p0.9_44": {"url": "https://iridia.ulb.ac.be/~fmascia/files/DIMACS/gen200_p0.9_44.clq", "type": "dimacs"},
    "DIMACS_p_hat300-1":     {"url": "https://iridia.ulb.ac.be/~fmascia/files/DIMACS/p_hat300-1.clq", "type": "dimacs"},
    "DIMACS_p_hat500-1":     {"url": "https://iridia.ulb.ac.be/~fmascia/files/DIMACS/p_hat500-1.clq", "type": "dimacs"},
    "DIMACS_keller4":        {"url": "https://iridia.ulb.ac.be/~fmascia/files/DIMACS/keller4.clq", "type": "dimacs"},

    # BHOSLIB datasets (Confirmed URLs, already in DIMACS format)
    "BHOSLIB_frb30-15-01":   {"url": "https://iridia.ulb.ac.be/~fmascia/files/BHOSLIB/frb30-15-1.clq", "type": "dimacs"},
    "BHOSLIB_frb35-17-01":   {"url": "https://iridia.ulb.ac.be/~fmascia/files/BHOSLIB/frb35-17-1.clq", "type": "dimacs"},
}

def convert_snap_to_dimacs(raw_content: bytes, dataset_name: str) -> str:
    """
    Decompresses SNAP data (edge list format) and converts it to DIMACS format.
    The DIMACS format is: p edge N M \n e u v ...
    """
    # 1. Decompress the content
    with gzip.GzipFile(fileobj=io.BytesIO(raw_content)) as gz:
        data = gz.read().decode('utf-8')

    # 2. Build the graph and ensure DIMACS 1-based indexing
    G = nx.Graph()
    max_node_id = 0
    edges = set()
    
    for line in data.splitlines():
        # Skip comment lines common in SNAP data
        if line.startswith(('#', 'c', '%')):
            continue
        
        try:
            # SNAP format is typically "u v"
            u, v = map(int, line.split()[:2])
            
            # DIMACS format requires node IDs >= 1. SNAP often uses 0-based indexing.
            # We enforce 1-based indexing by adding 1 to all IDs.
            u_dimacs = u + 1
            v_dimacs = v + 1
            
            # Store edges uniquely as a tuple of (smaller_id, larger_id)
            edges.add(tuple(sorted((u_dimacs, v_dimacs))))
            
            max_node_id = max(max_node_id, u_dimacs, v_dimacs)
            
        except ValueError:
            # Skip lines that don't conform to the expected "u v" format
            continue
        except IndexError:
            # Skip empty lines or lines with too few values
            continue
            
    # 3. Format into DIMACS string
    
    # N is the total number of nodes (max_node_id if 1-based)
    N = max_node_id
    # M is the number of unique undirected edges
    M = len(edges)
    
    dimacs_lines = [
        f"c Converted {dataset_name} from SNAP format to DIMACS format.",
        f"c Nodes (N): {N}, Edges (M): {M}",
        f"p edge {N} {M}"
    ]
    
    # Add all unique edges
    for u, v in edges:
        dimacs_lines.append(f"e {u} {v}")
    
    return "\n".join(dimacs_lines)

def download_datasets(config: Dict[str, Dict[str, str]], output_dir: str = "max_clique_datasets"):
    """
    Downloads and converts all datasets based on the configuration.
    """
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
        print(f"Created output directory: {output_dir}\n")

    for dataset_id, properties in config.items():
        url = properties["url"]
        data_type = properties["type"]
        
        # Clean the ID to get the desired filename format (e.g., C125.9, ca-GrQc)
        dataset_name = dataset_id.split('_', 1)[1]
        output_filename = os.path.join(output_dir, f"{dataset_name}.txt")
        
        print(f"[{dataset_id}] Downloading...")
        
        try:
            r = requests.get(url, timeout=60)
            r.raise_for_status() # Raise an exception for bad status codes (4xx or 5xx)

            content = ""
            if data_type == "dimacs":
                # DIMACS/BHOSLIB files are already in the correct text format
                content = r.text
                print(f"[{dataset_id}] Downloaded raw DIMACS content.")
            elif data_type == "snap":
                # SNAP files need decompression and conversion
                content = convert_snap_to_dimacs(r.content, dataset_name)
                print(f"[{dataset_id}] Decompressed and successfully converted to DIMACS format.")
            
            # Save the final content to the .txt file
            with open(output_filename, 'w') as f:
                f.write(content)
            
            print(f"[{dataset_id}] Saved successfully to: {output_filename}\n")

        except requests.exceptions.RequestException as e:
            print(f"ERROR: Failed to download {dataset_id}. Error: {e}\n")
        except Exception as e:
            print(f"UNEXPECTED ERROR during processing {dataset_id}: {e}\n")

# --- Execution ---
if __name__ == "__main__":
    try:
        # Check for and install the networkx library if it's missing (required for robust SNAP conversion)
        import subprocess
        try:
            import networkx
        except ImportError:
            print("The 'networkx' library is required for robust SNAP file conversion.")
            print("Installing networkx (you may need to enter your password for elevated privileges)...")
            subprocess.check_call(['pip', 'install', 'networkx'])
            print("networkx installed successfully.\n")

        download_datasets(DATASET_CONFIG)
        print("\n\n✅ All datasets have been downloaded, formatted, and saved as .txt files in the 'max_clique_datasets' directory.")
        print("You can now proceed with your maximum clique benchmarking.")

    except Exception as e:
        print(f"\nFATAL ERROR during script execution: {e}")
