// hash.cs
using System;
using System.IO;
using System.Security.Cryptography;
using System.Text;
using System.Linq;

class HashSuite
{
    static string Colorize(string text, string color)
    {
        string col = color switch
        {
            "green" => "\x1b[92m",
            "red" => "\x1b[91m",
            "yellow" => "\x1b[93m",
            "blue" => "\x1b[94m",
            _ => "\x1b[0m"
        };
        return col + text + "\x1b[0m";
    }

    static byte[] ComputeHash(byte[] data, string algo)
    {
        HashAlgorithm algorithm = algo.ToLower() switch
        {
            "md5" => MD5.Create(),
            "sha1" => SHA1.Create(),
            "sha256" => SHA256.Create(),
            "sha512" => SHA512.Create(),
            _ => throw new ArgumentException("Unsupported algorithm")
        };
        return algorithm.ComputeHash(data);
    }

    static string FormatHash(byte[] hash, string format)
    {
        if (format == "hex")
            return BitConverter.ToString(hash).Replace("-", "").ToLower();
        else if (format == "base64")
            return Convert.ToBase64String(hash);
        else
            return BitConverter.ToString(hash).Replace("-", "").ToLower();
    }

    static byte[] ReadFile(string filename)
    {
        return File.ReadAllBytes(filename);
    }

    static void WriteFile(string filename, string content)
    {
        File.WriteAllText(filename, content, Encoding.UTF8);
    }

    static void Main(string[] args)
    {
        string algo = "sha256";
        string text = "", inputFile = "", outputFile = "", salt = "", compare = "", format = "hex";
        bool help = false;

        for (int i = 0; i < args.Length; i++)
        {
            string arg = args[i];
            switch (arg)
            {
                case "-a":
                case "--algo": algo = args[++i]; break;
                case "-t":
                case "--text": text = args[++i]; break;
                case "-i":
                case "--input": inputFile = args[++i]; break;
                case "-o":
                case "--output": outputFile = args[++i]; break;
                case "-s":
                case "--salt": salt = args[++i]; break;
                case "-c":
                case "--compare": compare = args[++i]; break;
                case "-f":
                case "--format": format = args[++i]; break;
                case "-h":
                case "--help": help = true; break;
                default:
                    Console.WriteLine(Colorize($"Unknown option: {arg}", "red"));
                    Environment.Exit(1);
                    break;
            }
        }

        if (help)
        {
            Console.WriteLine(@"Usage: hash [options]
Options:
  -a <algo>     Algorithm: md5, sha1, sha256, sha512 (default sha256)
  -t <text>     Text to hash
  -i <file>     Input file
  -o <file>     Output file
  -s <salt>     Salt string
  -c <hash>     Hash to compare against
  -f <format>   Output format: hex, base64 (default hex)
  -h            Show this help");
            return;
        }

        byte[] data;
        try
        {
            if (!string.IsNullOrEmpty(text))
                data = Encoding.UTF8.GetBytes(text);
            else if (!string.IsNullOrEmpty(inputFile))
                data = ReadFile(inputFile);
            else
            {
                // read from stdin
                using var stdin = Console.OpenStandardInput();
                using var memoryStream = new MemoryStream();
                stdin.CopyTo(memoryStream);
                data = memoryStream.ToArray();
                if (data.Length == 0)
                {
                    Console.WriteLine(Colorize("No input data", "red"));
                    Environment.Exit(1);
                }
            }
        }
        catch (Exception e)
        {
            Console.WriteLine(Colorize($"Error reading input: {e.Message}", "red"));
            Environment.Exit(1);
            return;
        }

        // Add salt
        if (!string.IsNullOrEmpty(salt))
        {
            byte[] saltBytes = Encoding.UTF8.GetBytes(salt);
            byte[] combined = new byte[saltBytes.Length + data.Length];
            Buffer.BlockCopy(saltBytes, 0, combined, 0, saltBytes.Length);
            Buffer.BlockCopy(data, 0, combined, saltBytes.Length, data.Length);
            data = combined;
        }

        byte[] hash;
        try
        {
            hash = ComputeHash(data, algo);
        }
        catch (Exception e)
        {
            Console.WriteLine(Colorize($"Error computing hash: {e.Message}", "red"));
            Environment.Exit(1);
            return;
        }

        string result = FormatHash(hash, format);

        if (!string.IsNullOrEmpty(compare))
        {
            if (result == compare)
                Console.WriteLine(Colorize("✅ Hash matches!", "green"));
            else
            {
                Console.WriteLine(Colorize("❌ Hash does NOT match.", "red"));
                Console.WriteLine($"Expected: {compare}");
                Console.WriteLine($"Computed: {result}");
            }
            return;
        }

        if (!string.IsNullOrEmpty(outputFile))
        {
            try
            {
                WriteFile(outputFile, result);
                Console.WriteLine(Colorize($"Hash written to {outputFile}", "green"));
            }
            catch (Exception e)
            {
                Console.WriteLine(Colorize($"Error writing file: {e.Message}", "red"));
                Environment.Exit(1);
            }
        }
        else
        {
            Console.WriteLine(result);
        }
    }
}
