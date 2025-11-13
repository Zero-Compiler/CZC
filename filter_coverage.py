#!/usr/bin/env python3
"""Filter out compiler-generated virtual destructors from LCOV coverage data."""
import sys
import re

def filter_lcov(input_file, output_file):
    """Remove virtual destructor entries (D0Ev, D1Ev, D2Ev) from LCOV file."""
    with open(input_file, 'r') as f:
        content = f.read()
    
    # Split by source file sections
    sections = re.split(r'(SF:.*?\n)', content)
    
    output_sections = []
    total_filtered = 0
    
    for i, section in enumerate(sections):
        if section.startswith('SF:'):
            # This is a source file marker
            output_sections.append(section)
        elif section.strip():
            # This is content of a source file
            lines = section.split('\n')
            filtered_lines = []
            fnf_adjust = 0  # Functions found adjustment
            fnh_adjust = 0  # Functions hit adjustment
            
            for line in lines:
                # Filter out destructor entries
                if (line.startswith('FN:') or line.startswith('FNDA:') or line.startswith('FNA:')) and re.search(r'D[012]Ev', line):
                    total_filtered += 1
                    fnf_adjust += 1
                    # Check if this destructor was hit
                    if line.startswith('FNA:'):
                        parts = line.split(',')
                        if len(parts) >= 2 and int(parts[1]) > 0:
                            fnh_adjust += 1
                    continue
                
                # Update FNF count
                if line.startswith('FNF:'):
                    old_count = int(line.split(':')[1])
                    new_count = old_count - fnf_adjust
                    filtered_lines.append(f'FNF:{new_count}')
                    continue
                
                # Update FNH count
                if line.startswith('FNH:'):
                    old_count = int(line.split(':')[1])
                    new_count = old_count - fnh_adjust
                    filtered_lines.append(f'FNH:{new_count}')
                    continue
                
                filtered_lines.append(line)
            
            output_sections.append('\n'.join(filtered_lines))
    
    with open(output_file, 'w') as f:
        f.write(''.join(output_sections))
    
    print(f"Filtered {total_filtered} destructor entries")

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: filter_coverage.py <input.info> <output.info>")
        sys.exit(1)
    
    filter_lcov(sys.argv[1], sys.argv[2])
