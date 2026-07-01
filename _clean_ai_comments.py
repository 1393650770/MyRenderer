#!/usr/bin/env python3
"""Remove all [AI], [AI:BEGIN], [AI:END] comment lines from source files."""

import re
import os
import glob

ROOT = r'd:\Project\GameDevelop\MyRenderer'

# Patterns for matching [AI] markers in comments
# Group 1: full-line comment with [AI] marker
# Group 2: inline comment with [AI] marker

def process_file(filepath):
    with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
        lines = f.readlines()

    new_lines = []
    changed = False

    for line in lines:
        stripped = line.lstrip()

        # Check for C++ style: // -- [AI...]
        # Full-line comment: line starts (after whitespace) with //
        if stripped.startswith('// -- [AI') or stripped.startswith('//-- [AI') or stripped.startswith('//  -- [AI'):
            # Full-line comment with [AI] marker — delete entire line
            changed = True
            continue

        # Check for Lua / xmake style: -- [AI...]
        if stripped.startswith('-- [AI') and not stripped.startswith('-- ['):
            # Actually, Lua comments are just --, let me be more precise
            pass

        # Lua full-line comment: -- [AI]... (starts with -- then [AI])
        if re.match(r'^--\s*\[AI', stripped):
            # Check it's not mid-line by verifying nothing meaningful before --
            before_comment = line[:len(line) - len(stripped)]
            if before_comment.strip() == '':
                changed = True
                continue

        # Markdown/doc: line containing [AI] marker as part of convention description
        # e.g., "- AI-generated code: `// -- [AI]` ..."
        if '[AI:BEGIN]' in stripped or '[AI:END]' in stripped or '[AI]' in stripped:
            # Still has AI marker in the remaining part
            # Check if it's an inline comment
            new_line = line

            # Remove C++ inline comments: // -- [AI...] or //-- [AI...]
            # Pattern: // optional-space -- optional-space [AI...]
            new_line = re.sub(r'\s*//\s*--\s*\[AI[^\n]*', '', new_line)

            # Remove Lua inline comments: -- [AI...]
            # Be careful not to remove -- that aren't [AI] markers
            new_line = re.sub(r'\s*--\s*\[AI[^\n]*', '', new_line)

            if new_line != line:
                changed = True
                # If the line is now just whitespace or empty, skip it
                if new_line.strip() == '':
                    continue
                line = new_line

        new_lines.append(line)

    if changed:
        # Write back with same line endings
        content = ''.join(new_lines)
        with open(filepath, 'w', encoding='utf-8', errors='replace') as f:
            f.write(content)
        return True
    return False


def main():
    # Find all relevant files
    patterns = ['**/*.cpp', '**/*.h', '**/*.lua', '**/*.comp', 'CLAUDE.md', 'xmake.lua']
    files_to_process = set()

    for pattern in patterns:
        for f in glob.glob(os.path.join(ROOT, pattern), recursive=True):
            files_to_process.add(f)

    # Filter: only files that actually contain [AI] markers
    ai_pattern = re.compile(r'\[AI\]|\[AI:BEGIN\]|\[AI:END\]')
    relevant_files = []
    for f in sorted(files_to_process):
        try:
            with open(f, 'r', encoding='utf-8', errors='replace') as fh:
                content = fh.read()
            if ai_pattern.search(content):
                relevant_files.append(f)
        except Exception:
            pass

    print(f"Found {len(relevant_files)} files with [AI] markers")

    changed_count = 0
    for f in relevant_files:
        try:
            if process_file(f):
                changed_count += 1
                rel = os.path.relpath(f, ROOT)
                print(f"  Modified: {rel}")
        except Exception as e:
            rel = os.path.relpath(f, ROOT)
            print(f"  ERROR ({rel}): {e}")

    print(f"\nDone. Modified {changed_count} files.")


if __name__ == '__main__':
    main()
