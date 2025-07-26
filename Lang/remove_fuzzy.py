#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import argparse
import polib
from pathlib import Path

def remove_fuzzy_inplace(po_file_path, target_context):
    """
    Removes fuzzy flag from entries with specific context in-place.
    """
    try:
        # Load file
        po = polib.pofile(po_file_path)
    except Exception as e:
        print(f"❌ Error loading {po_file_path}: {e}")
        return False, 0, 0
    
    # Counters
    processed_count = 0
    removed_fuzzy_count = 0
    
    # Process entries with target context
    for entry in po:
        if entry.msgctxt == target_context and 'fuzzy' in entry.flags:
            processed_count += 1
            
            # Check if msgid has changed (if previous_msgid exists)
            if entry.previous_msgid and entry.previous_msgid != entry.msgid:
                # msgid changed - keep fuzzy flag
                continue
            
            # Remove fuzzy flag and clear previous values
            entry.flags.remove('fuzzy')
            
            # Remove all previous values (lines #| ...)
            entry.previous_msgid = None
            entry.previous_msgstr = None
            entry.previous_msgctxt = None
            entry.previous_msgid_plural = None
            
            removed_fuzzy_count += 1
    
    # Save file only if there were changes
    if removed_fuzzy_count > 0:
        try:
            po.save(po_file_path)
            return True, processed_count, removed_fuzzy_count
        except Exception as e:
            print(f"❌ Error saving {po_file_path}: {e}")
            return False, processed_count, 0
    
    return True, processed_count, removed_fuzzy_count

def find_po_files(directory, recursive=True):
    """Find all .po files in directory"""
    path = Path(directory)
    
    if recursive:
        return list(path.rglob("*.po"))
    else:
        return list(path.glob("*.po"))

def main():
    parser = argparse.ArgumentParser(
        description="Recursive processing of .po files to remove fuzzy flags (in-place)"
    )
    parser.add_argument("directory", nargs="?", default=".",
                       help="Directory to search for .po files (default: current)")
    parser.add_argument("-c", "--context", default="settings.menu",
                       help="Context to process (default: settings.menu)")
    parser.add_argument("--no-recursive", action="store_true",
                       help="Don't search files recursively")
    parser.add_argument("--dry-run", action="store_true",
                       help="Show what would be done but don't save")
    parser.add_argument("-v", "--verbose", action="store_true",
                       help="Verbose output")
    parser.add_argument("--exclude", action="append", default=[],
                       help="Exclude directories/files (can be specified multiple times)")
    
    args = parser.parse_args()
    
    # Check directory
    if not os.path.isdir(args.directory):
        print(f"❌ Error: Directory '{args.directory}' not found!")
        sys.exit(1)
    
    print(f"🔍 Searching for .po files in: {os.path.abspath(args.directory)}")
    print(f"🔄 Recursive: {'Yes' if not args.no_recursive else 'No'}")
    print(f"🏷️  Context: {args.context}")
    
    if args.dry_run:
        print("🧪 DRY RUN mode - changes will not be saved")
    
    if args.exclude:
        print(f"🚫 Exclusions: {', '.join(args.exclude)}")
    
    print("=" * 60)
    
    # Find all .po files
    po_files = find_po_files(args.directory, not args.no_recursive)
    
    # Filter exclusions
    if args.exclude:
        filtered_files = []
        for po_file in po_files:
            exclude = False
            for exclusion in args.exclude:
                if exclusion in str(po_file):
                    exclude = True
                    break
            if not exclude:
                filtered_files.append(po_file)
        po_files = filtered_files
    
    if not po_files:
        print("❌ No .po files found for processing!")
        sys.exit(1)
    
    print(f"📁 Found {len(po_files)} files to process")
    print("-" * 60)
    
    # Statistics
    total_processed = 0
    total_removed = 0
    success_count = 0
    error_count = 0
    unchanged_count = 0
    
    # Process files
    for po_file in sorted(po_files):
        relative_path = os.path.relpath(po_file, args.directory)
        
        if args.verbose or args.dry_run:
            print(f"🔄 {relative_path}")
        
        if args.dry_run:
            # In dry-run mode just show what we found
            try:
                po = polib.pofile(po_file)
                fuzzy_with_context = sum(1 for entry in po 
                                       if entry.msgctxt == args.context and 'fuzzy' in entry.flags)
                if fuzzy_with_context > 0:
                    print(f"   Found {fuzzy_with_context} fuzzy entries with context '{args.context}'")
                else:
                    print(f"   No fuzzy entries with context '{args.context}'")
            except Exception as e:
                print(f"   ❌ Error reading: {e}")
            continue
        
        # Process file
        success, processed, removed = remove_fuzzy_inplace(po_file, args.context)
        
        if success:
            success_count += 1
            total_processed += processed
            total_removed += removed
            
            if removed > 0:
                status = f"✅ {relative_path} - removed {removed} fuzzy flags"
                print(status)
            else:
                unchanged_count += 1
                if args.verbose:
                    print(f"⚪ {relative_path} - no changes")
        else:
            error_count += 1
    
    # Final statistics
    print("=" * 60)
    if not args.dry_run:
        print(f"✅ Successfully processed: {success_count} files")
        print(f"🔧 Changed: {success_count - unchanged_count} files")
        print(f"⚪ Unchanged: {unchanged_count} files")
        print(f"❌ Errors: {error_count}")
        print(f"📊 Total fuzzy flags removed: {total_removed}")
    else:
        print("Dry run completed. Use without --dry-run to apply changes.")
    
    if error_count > 0:
        sys.exit(1)

if __name__ == "__main__":
    main()