// hash.java
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.*;
import java.util.Base64;

public class hash {
    // ANSI colors (if supported)
    private static final String RESET = "\u001B[0m";
    private static final String GREEN = "\u001B[92m";
    private static final String RED = "\u001B[91m";
    private static final String YELLOW = "\u001B[93m";
    private static final String BLUE = "\u001B[94m";

    private static String colorize(String text, String color) {
        return color + text + RESET;
    }

    private static byte[] computeHash(byte[] data, String algo) throws NoSuchAlgorithmException {
        MessageDigest md = MessageDigest.getInstance(algo);
        return md.digest(data);
    }

    private static String formatHash(byte[] hash, String format) {
        if (format.equals("hex")) {
            StringBuilder sb = new StringBuilder();
            for (byte b : hash) sb.append(String.format("%02x", b));
            return sb.toString();
        } else if (format.equals("base64")) {
            return Base64.getEncoder().encodeToString(hash);
        } else {
            return new String(hash, StandardCharsets.ISO_8859_1); // fallback
        }
    }

    private static byte[] readFile(String filename) throws IOException {
        return Files.readAllBytes(Paths.get(filename));
    }

    private static void writeFile(String filename, String content) throws IOException {
        Files.write(Paths.get(filename), content.getBytes(StandardCharsets.UTF_8));
    }

    private static void printHelp() {
        System.out.println("Usage: java hash [options]");
        System.out.println("Options:");
        System.out.println("  -a <algo>     Algorithm: md5, sha1, sha256, sha512 (default sha256)");
        System.out.println("  -t <text>     Text to hash");
        System.out.println("  -i <file>     Input file");
        System.out.println("  -o <file>     Output file");
        System.out.println("  -s <salt>     Salt string");
        System.out.println("  -c <hash>     Hash to compare against");
        System.out.println("  -f <format>   Output format: hex, base64 (default hex)");
        System.out.println("  -h            Show this help");
    }

    public static void main(String[] args) {
        String algo = "SHA-256";
        String text = "", inputFile = "", outputFile = "", salt = "", compare = "", format = "hex";
        boolean help = false;

        for (int i = 0; i < args.length; i++) {
            switch (args[i]) {
                case "-a":
                case "--algo":
                    algo = args[++i].toUpperCase();
                    if (!algo.startsWith("SHA") && !algo.equals("MD5")) {
                        System.err.println(colorize("Unsupported algorithm: " + algo, RED));
                        System.exit(1);
                    }
                    break;
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
                    System.err.println(colorize("Unknown option: " + args[i], RED));
                    System.exit(1);
            }
        }

        if (help) { printHelp(); return; }

        byte[] data;
        try {
            if (!text.isEmpty()) {
                data = text.getBytes(StandardCharsets.UTF_8);
            } else if (!inputFile.isEmpty()) {
                data = readFile(inputFile);
            } else {
                // read from stdin
                ByteArrayOutputStream baos = new ByteArrayOutputStream();
                byte[] buffer = new byte[4096];
                int bytesRead;
                while ((bytesRead = System.in.read(buffer)) != -1) {
                    baos.write(buffer, 0, bytesRead);
                }
                data = baos.toByteArray();
                if (data.length == 0) {
                    System.err.println(colorize("No input data", RED));
                    System.exit(1);
                }
            }
        } catch (Exception e) {
            System.err.println(colorize("Error reading input: " + e.getMessage(), RED));
            System.exit(1);
            return;
        }

        // Add salt
        if (!salt.isEmpty()) {
            byte[] saltBytes = salt.getBytes(StandardCharsets.UTF_8);
            byte[] combined = new byte[saltBytes.length + data.length];
            System.arraycopy(saltBytes, 0, combined, 0, saltBytes.length);
            System.arraycopy(data, 0, combined, saltBytes.length, data.length);
            data = combined;
        }

        byte[] hash;
        try {
            hash = computeHash(data, algo);
        } catch (NoSuchAlgorithmException e) {
            System.err.println(colorize("Unsupported algorithm: " + algo, RED));
            System.exit(1);
            return;
        }

        String result = formatHash(hash, format);

        if (!compare.isEmpty()) {
            if (result.equals(compare)) {
                System.out.println(colorize("✅ Hash matches!", GREEN));
            } else {
                System.out.println(colorize("❌ Hash does NOT match.", RED));
                System.out.println("Expected: " + compare);
                System.out.println("Computed: " + result);
            }
            return;
        }

        if (!outputFile.isEmpty()) {
            try {
                writeFile(outputFile, result);
                System.out.println(colorize("Hash written to " + outputFile, GREEN));
            } catch (IOException e) {
                System.err.println(colorize("Error writing file: " + e.getMessage(), RED));
                System.exit(1);
            }
        } else {
            System.out.println(result);
        }
    }
}
