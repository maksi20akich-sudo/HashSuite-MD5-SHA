// hash.js
#!/usr/bin/env node
'use strict';

const crypto = require('crypto');
const fs = require('fs');
const readline = require('readline');

// ANSI colors
const COLORS = {
    green: '\x1b[92m',
    red: '\x1b[91m',
    yellow: '\x1b[93m',
    blue: '\x1b[94m',
    reset: '\x1b[0m'
};

function colorize(text, color) {
    return COLORS[color] + text + COLORS.reset;
}

function getHasher(algo) {
    const algos = ['md5', 'sha1', 'sha256', 'sha512'];
    if (!algos.includes(algo)) {
        throw new Error(`Unsupported algorithm: ${algo}`);
    }
    return (data) => {
        const hash = crypto.createHash(algo);
        hash.update(data);
        return hash.digest();
    };
}

function formatHash(digest, format) {
    if (format === 'hex') return digest.toString('hex');
    if (format === 'base64') return digest.toString('base64');
    return digest.toString('hex');
}

function readFile(filename) {
    return fs.readFileSync(filename);
}

function writeFile(filename, content) {
    fs.writeFileSync(filename, content, 'utf8');
}

function parseArgs() {
    const args = process.argv.slice(2);
    const opts = {
        algo: 'sha256',
        text: '',
        input: '',
        output: '',
        salt: '',
        compare: '',
        format: 'hex',
        help: false
    };
    for (let i = 0; i < args.length; i++) {
        const arg = args[i];
        switch (arg) {
            case '-a':
            case '--algo': opts.algo = args[++i]; break;
            case '-t':
            case '--text': opts.text = args[++i]; break;
            case '-i':
            case '--input': opts.input = args[++i]; break;
            case '-o':
            case '--output': opts.output = args[++i]; break;
            case '-s':
            case '--salt': opts.salt = args[++i]; break;
            case '-c':
            case '--compare': opts.compare = args[++i]; break;
            case '-f':
            case '--format': opts.format = args[++i]; break;
            case '-h':
            case '--help': opts.help = true; break;
            default:
                console.error(colorize(`Unknown option: ${arg}`, 'red'));
                process.exit(1);
        }
    }
    return opts;
}

async function main() {
    const opts = parseArgs();
    if (opts.help) {
        console.log(`Usage: node hash.js [options]
Options:
  -a, --algo <algo>   Algorithm: md5, sha1, sha256, sha512 (default sha256)
  -t, --text <text>   Text to hash
  -i, --input <file>  Input file
  -o, --output <file> Output file
  -s, --salt <salt>   Salt string
  -c, --compare <hash> Hash to compare against
  -f, --format <fmt>  Output format: hex, base64 (default hex)
  -h, --help          Show this help`);
        return;
    }

    // Read input
    let data;
    if (opts.text) {
        data = Buffer.from(opts.text, 'utf8');
    } else if (opts.input) {
        try {
            data = readFile(opts.input);
        } catch (err) {
            console.error(colorize(`Error reading file: ${err.message}`, 'red'));
            process.exit(1);
        }
    } else {
        // read from stdin
        const rl = readline.createInterface({
            input: process.stdin,
            output: process.stdout,
            terminal: false
        });
        const chunks = [];
        for await (const chunk of rl) {
            chunks.push(Buffer.from(chunk + '\n', 'utf8'));
        }
        if (chunks.length === 0) {
            console.error(colorize('No input data', 'red'));
            process.exit(1);
        }
        data = Buffer.concat(chunks);
        // remove trailing newline if present? We'll keep as is.
    }

    // Prepare data with salt
    let finalData = data;
    if (opts.salt) {
        finalData = Buffer.concat([Buffer.from(opts.salt, 'utf8'), data]);
    }

    // Compute hash
    let hasher;
    try {
        hasher = getHasher(opts.algo);
    } catch (err) {
        console.error(colorize(err.message, 'red'));
        process.exit(1);
    }
    const digest = hasher(finalData);
    const result = formatHash(digest, opts.format);

    // Compare
    if (opts.compare) {
        if (result === opts.compare) {
            console.log(colorize('✅ Hash matches!', 'green'));
        } else {
            console.log(colorize('❌ Hash does NOT match.', 'red'));
            console.log(`Expected: ${opts.compare}`);
            console.log(`Computed: ${result}`);
        }
        process.exit(0);
    }

    // Output
    if (opts.output) {
        try {
            writeFile(opts.output, result);
            console.log(colorize(`Hash written to ${opts.output}`, 'green'));
        } catch (err) {
            console.error(colorize(`Error writing file: ${err.message}`, 'red'));
            process.exit(1);
        }
    } else {
        console.log(result);
    }
}

main().catch(err => {
    console.error(colorize(`Unhandled error: ${err.message}`, 'red'));
    process.exit(1);
});
