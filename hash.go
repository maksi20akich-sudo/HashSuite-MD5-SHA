// hash.go
package main

import (
	"crypto/md5"
	"crypto/sha1"
	"crypto/sha256"
	"crypto/sha512"
	"encoding/base64"
	"encoding/hex"
	"flag"
	"fmt"
	"io"
	"os"
	"strings"
)

// ANSI colors
const (
	reset  = "\033[0m"
	green  = "\033[92m"
	red    = "\033[91m"
	yellow = "\033[93m"
	blue   = "\033[94m"
)

func colorize(text, color string) string {
	return color + text + reset
}

type hashFunc func(data []byte) []byte

func getHasher(algo string) (hashFunc, error) {
	switch algo {
	case "md5":
		return func(data []byte) []byte {
			h := md5.Sum(data)
			return h[:]
		}, nil
	case "sha1":
		return func(data []byte) []byte {
			h := sha1.Sum(data)
			return h[:]
		}, nil
	case "sha256":
		return func(data []byte) []byte {
			h := sha256.Sum256(data)
			return h[:]
		}, nil
	case "sha512":
		return func(data []byte) []byte {
			h := sha512.Sum512(data)
			return h[:]
		}, nil
	default:
		return nil, fmt.Errorf("unsupported algorithm: %s", algo)
	}
}

func formatHash(digest []byte, format string) string {
	if format == "hex" {
		return hex.EncodeToString(digest)
	} else if format == "base64" {
		return base64.StdEncoding.EncodeToString(digest)
	}
	return hex.EncodeToString(digest) // fallback
}

func readFile(filename string) ([]byte, error) {
	file, err := os.Open(filename)
	if err != nil {
		return nil, err
	}
	defer file.Close()
	return io.ReadAll(file)
}

func main() {
	algo := flag.String("a", "sha256", "Algorithm: md5, sha1, sha256, sha512")
	text := flag.String("t", "", "Text to hash")
	input := flag.String("i", "", "Input file")
	output := flag.String("o", "", "Output file")
	salt := flag.String("s", "", "Salt string")
	compare := flag.String("c", "", "Hash to compare against")
	format := flag.String("f", "hex", "Output format: hex, base64")
	help := flag.Bool("h", false, "Show help")
	flag.Parse()

	if *help {
		fmt.Println(`Usage: hash [options]
Options:
  -a <algo>     Algorithm: md5, sha1, sha256, sha512 (default sha256)
  -t <text>     Text to hash
  -i <file>     Input file
  -o <file>     Output file
  -s <salt>     Salt string
  -c <hash>     Hash to compare against
  -f <format>   Output format: hex, base64 (default hex)
  -h            Show this help`)
		return
	}

	// Read input
	var data []byte
	if *text != "" {
		data = []byte(*text)
	} else if *input != "" {
		d, err := readFile(*input)
		if err != nil {
			fmt.Println(colorize("Error reading file: "+err.Error(), red))
			os.Exit(1)
		}
		data = d
	} else {
		// read from stdin
		stat, _ := os.Stdin.Stat()
		if (stat.Mode() & os.ModeCharDevice) == 0 {
			// data from pipe
			d, err := io.ReadAll(os.Stdin)
			if err != nil {
				fmt.Println(colorize("Error reading stdin: "+err.Error(), red))
				os.Exit(1)
			}
			data = d
		} else {
			fmt.Print(colorize("Enter data (Ctrl+D to finish): ", yellow))
			d, err := io.ReadAll(os.Stdin)
			if err != nil {
				fmt.Println(colorize("Error reading stdin: "+err.Error(), red))
				os.Exit(1)
			}
			data = d
		}
	}
	if len(data) == 0 {
		fmt.Println(colorize("No input data", red))
		os.Exit(1)
	}

	// Get hasher
	hasher, err := getHasher(*algo)
	if err != nil {
		fmt.Println(colorize(err.Error(), red))
		os.Exit(1)
	}

	// Prepend salt if given
	finalData := data
	if *salt != "" {
		finalData = append([]byte(*salt), data...)
	}

	// Compute hash
	digest := hasher(finalData)
	result := formatHash(digest, *format)

	// Compare if requested
	if *compare != "" {
		if result == *compare {
			fmt.Println(colorize("✅ Hash matches!", green))
		} else {
			fmt.Println(colorize("❌ Hash does NOT match.", red))
			fmt.Printf("Expected: %s\n", *compare)
			fmt.Printf("Computed: %s\n", result)
		}
		os.Exit(0)
	}

	// Output
	if *output != "" {
		err := os.WriteFile(*output, []byte(result), 0644)
		if err != nil {
			fmt.Println(colorize("Error writing file: "+err.Error(), red))
			os.Exit(1)
		}
		fmt.Println(colorize("Hash written to "+*output, green))
	} else {
		fmt.Println(result)
	}
}
