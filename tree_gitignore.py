#!/usr/bin/env python3
"""
Simple tree generator that respects .gitignore patterns.
Mimics `eza --tree --git-ignore` for systems without eza.
"""

import os
import fnmatch
import sys

IGNORED = set()
GITIGNORE_PATTERNS = []

def load_gitignore(path=".gitignore"):
    """Load .gitignore patterns."""
    if not os.path.exists(path):
        return
    with open(path, "r") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            GITIGNORE_PATTERNS.append(line)

def is_ignored(rel_path):
    """Check if a path should be ignored based on .gitignore patterns."""
    # Normalize path separators
    rel_path = rel_path.replace(os.sep, "/")
    parts = rel_path.split("/")
    
    for pattern in GITIGNORE_PATTERNS:
        # Handle directory patterns (ending with /)
        is_dir_pattern = pattern.endswith("/")
        clean_pattern = pattern.rstrip("/")
        
        # Handle absolute patterns (starting with /)
        is_absolute = clean_pattern.startswith("/")
        clean_pattern = clean_pattern.lstrip("/")
        
        if is_absolute:
            # Match from root only
            if is_dir_pattern:
                # Check if any prefix matches
                for i in range(1, len(parts) + 1):
                    prefix = "/".join(parts[:i])
                    if fnmatch.fnmatch(prefix, clean_pattern):
                        return True
            else:
                if fnmatch.fnmatch(rel_path, clean_pattern):
                    return True
                # Also match intermediate directories for directory containment
                for i in range(1, len(parts) + 1):
                    prefix = "/".join(parts[:i])
                    if fnmatch.fnmatch(prefix, clean_pattern):
                        return True
        else:
            # Match any part of the path
            if is_dir_pattern:
                # Directory pattern: match any directory component
                for i in range(1, len(parts) + 1):
                    prefix = "/".join(parts[:i])
                    if fnmatch.fnmatch(prefix, clean_pattern):
                        return True
                    # Also match basename at any level
                    if fnmatch.fnmatch(parts[i-1], clean_pattern):
                        return True
            else:
                # File pattern: match full path or any component
                if fnmatch.fnmatch(rel_path, clean_pattern):
                    return True
                if fnmatch.fnmatch(os.path.basename(rel_path), clean_pattern):
                    return True
                # Check if any parent directory matches (for directory containment)
                for i in range(1, len(parts)):
                    prefix = "/".join(parts[:i])
                    if fnmatch.fnmatch(prefix, clean_pattern):
                        return True
                    if fnmatch.fnmatch(parts[i-1], clean_pattern):
                        return True
    
    return False

def tree(dir_path=".", prefix=""):
    """Generate tree output."""
    try:
        entries = sorted(os.listdir(dir_path))
    except PermissionError:
        return
    
    # Filter out ignored entries
    filtered = []
    for entry in entries:
        full = os.path.join(dir_path, entry)
        rel = os.path.relpath(full, ".")
        if rel.startswith("./"):
            rel = rel[2:]
        if not is_ignored(rel):
            filtered.append(entry)
    
    for i, entry in enumerate(filtered):
        full = os.path.join(dir_path, entry)
        rel = os.path.relpath(full, ".")
        if rel.startswith("./"):
            rel = rel[2:]
        
        is_last = (i == len(filtered) - 1)
        connector = "└── " if is_last else "├── "
        print(f"{prefix}{connector}{entry}")
        
        if os.path.isdir(full):
            extension = "    " if is_last else "│   "
            tree(full, prefix + extension)

if __name__ == "__main__":
    load_gitignore()
    print(".")
    tree(".")
