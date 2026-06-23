#!/usr/bin/env ruby
# hash.rb
# encoding: UTF-8

require 'digest'
require 'optparse'
require 'base64'

# ANSI colors
COLORS = {
  green: "\e[92m",
  red: "\e[91m",
  yellow: "\e[93m",
  blue: "\e[94m",
  reset: "\e[0m"
}

def colorize(text, color)
  "#{COLORS[color]}#{text}#{COLORS[:reset]}"
end

def compute_hash(data, algo)
  case algo
  when 'md5' then Digest::MD5.digest(data)
  when 'sha1' then Digest::SHA1.digest(data)
  when 'sha256' then Digest::SHA256.digest(data)
  when 'sha512' then Digest::SHA512.digest(data)
  else raise "Unsupported algorithm: #{algo}"
  end
end

def format_hash(digest, format)
  if format == 'hex'
    digest.unpack1('H*')
  elsif format == 'base64'
    Base64.strict_encode64(digest)
  else
    digest.unpack1('H*')
  end
end

def read_file(filename)
  File.binread(filename)
rescue => e
  raise "Cannot read file: #{e.message}"
end

def write_file(filename, content)
  File.write(filename, content, encoding: 'UTF-8')
rescue => e
  raise "Cannot write file: #{e.message}"
end

options = {
  algo: 'sha256',
  text: nil,
  input: nil,
  output: nil,
  salt: nil,
  compare: nil,
  format: 'hex',
  help: false
}

OptionParser.new do |opts|
  opts.banner = "Usage: hash.rb [options]"
  opts.on('-a', '--algo ALGO', 'Algorithm: md5, sha1, sha256, sha512 (default sha256)') { |a| options[:algo] = a }
  opts.on('-t', '--text TEXT', 'Text to hash') { |t| options[:text] = t }
  opts.on('-i', '--input FILE', 'Input file') { |f| options[:input] = f }
  opts.on('-o', '--output FILE', 'Output file') { |f| options[:output] = f }
  opts.on('-s', '--salt SALT', 'Salt string') { |s| options[:salt] = s }
  opts.on('-c', '--compare HASH', 'Hash to compare against') { |c| options[:compare] = c }
  opts.on('-f', '--format FORMAT', 'Output format: hex, base64 (default hex)') { |f| options[:format] = f }
  opts.on('-h', '--help', 'Show this help') { options[:help] = true }
end.parse!

if options[:help]
  puts <<~HELP
    Usage: hash.rb [options]
    Options:
      -a, --algo ALGO     Algorithm: md5, sha1, sha256, sha512 (default sha256)
      -t, --text TEXT     Text to hash
      -i, --input FILE    Input file
      -o, --output FILE   Output file
      -s, --salt SALT     Salt string
      -c, --compare HASH  Hash to compare against
      -f, --format FORMAT Output format: hex, base64 (default hex)
      -h, --help          Show this help
  HELP
  exit
end

# Read input
data = nil
if options[:text]
  data = options[:text].dup.force_encoding('UTF-8')
elsif options[:input]
  begin
    data = read_file(options[:input])
  rescue => e
    puts colorize("Error reading file: #{e}", :red)
    exit 1
  end
else
  # read from stdin
  if $stdin.tty?
    print colorize("Enter data (Ctrl+D to finish): ", :yellow)
  end
  data = $stdin.binread
  if data.nil? || data.empty?
    puts colorize("No input data", :red)
    exit 1
  end
end

# Add salt
if options[:salt]
  salt = options[:salt].dup.force_encoding('ASCII-8BIT')
  data = salt + data
end

# Compute hash
begin
  digest = compute_hash(data, options[:algo])
rescue => e
  puts colorize("Error: #{e.message}", :red)
  exit 1
end

result = format_hash(digest, options[:format])

# Compare
if options[:compare]
  if result == options[:compare]
    puts colorize("✅ Hash matches!", :green)
  else
    puts colorize("❌ Hash does NOT match.", :red)
    puts "Expected: #{options[:compare]}"
    puts "Computed: #{result}"
  end
  exit 0
end

# Output
if options[:output]
  begin
    write_file(options[:output], result)
    puts colorize("Hash written to #{options[:output]}", :green)
  rescue => e
    puts colorize("Error writing file: #{e}", :red)
    exit 1
  end
else
  puts result
end
