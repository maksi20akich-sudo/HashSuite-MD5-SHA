# hash.py
#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import hashlib
import sys
import argparse
import base64
import os

# ANSI colors
COLORS = {
    'green': '\033[92m',
    'red': '\033[91m',
    'yellow': '\033[93m',
    'blue': '\033[94m',
    'reset': '\033[0m'
}

def colorize(text, color):
    return f"{COLORS.get(color, '')}{text}{COLORS['reset']}"

def get_hasher(algo):
    """Возвращает функцию хеширования по имени алгоритма."""
    algos = {
        'md5': hashlib.md5,
        'sha1': hashlib.sha1,
        'sha256': hashlib.sha256,
        'sha512': hashlib.sha512
    }
    if algo not in algos:
        raise ValueError(f"Unsupported algorithm: {algo}")
    return algos[algo]

def compute_hash(data, algo, salt=''):
    """Вычисляет хеш данных с солью."""
    if salt:
        data = salt + data
    if isinstance(data, str):
        data = data.encode('utf-8')
    hasher = get_hasher(algo)()
    hasher.update(data)
    return hasher.digest()

def format_hash(digest, fmt):
    if fmt == 'hex':
        return digest.hex()
    elif fmt == 'base64':
        return base64.b64encode(digest).decode('ascii')
    else:
        raise ValueError(f"Unsupported format: {fmt}")

def read_file(filename):
    with open(filename, 'rb') as f:
        return f.read()

def main():
    parser = argparse.ArgumentParser(description="HashSuite – MD5/SHA hashing tool")
    parser.add_argument('-a', '--algo', default='sha256',
                        choices=['md5', 'sha1', 'sha256', 'sha512'],
                        help='Hash algorithm (default: sha256)')
    parser.add_argument('-t', '--text', help='Text to hash')
    parser.add_argument('-i', '--input', help='Input file')
    parser.add_argument('-o', '--output', help='Output file')
    parser.add_argument('-s', '--salt', default='', help='Salt string (prepended)')
    parser.add_argument('-c', '--compare', help='Hash to compare against')
    parser.add_argument('-f', '--format', default='hex', choices=['hex', 'base64'],
                        help='Output format (default: hex)')
    parser.add_argument('-v', '--version', action='version', version='HashSuite 1.0')

    args = parser.parse_args()

    # Read input
    if args.text:
        data = args.text.encode('utf-8')
    elif args.input:
        try:
            data = read_file(args.input)
        except Exception as e:
            sys.exit(colorize(f"Error reading file: {e}", 'red'))
    else:
        # read from stdin
        if sys.stdin.isatty():
            sys.stderr.write(colorize("Enter data (Ctrl+D to finish): ", 'yellow'))
        data = sys.stdin.buffer.read()
        if not data:
            sys.exit(colorize("No input data", 'red'))

    try:
        # Compute hash
        hasher = get_hasher(args.algo)()
        if args.salt:
            hasher.update(args.salt.encode('utf-8'))
        hasher.update(data)
        digest = hasher.digest()
        result = format_hash(digest, args.format)
    except Exception as e:
        sys.exit(colorize(f"Error during hashing: {e}", 'red'))

    # Compare if requested
    if args.compare:
        if result == args.compare:
            print(colorize("✅ Hash matches!", 'green'))
        else:
            print(colorize("❌ Hash does NOT match.", 'red'))
            print(f"Expected: {args.compare}")
            print(f"Computed: {result}")
        # still output the computed hash if output specified? We'll just exit.
        if args.output:
            # but we might want to write the result anyway? Usually comparison is final.
            pass
        sys.exit(0)

    # Output
    if args.output:
        try:
            with open(args.output, 'w', encoding='utf-8') as f:
                f.write(result)
            print(colorize(f"Hash written to {args.output}", 'green'))
        except Exception as e:
            sys.exit(colorize(f"Error writing file: {e}", 'red'))
    else:
        print(result)

if __name__ == '__main__':
    main()
