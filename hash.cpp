// hash.cpp
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstring>
#include <getopt.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>

using namespace std;

// ANSI colors
const string RESET = "\033[0m";
const string GREEN = "\033[92m";
const string RED = "\033[91m";
const string YELLOW = "\033[93m";
const string BLUE = "\033[94m";

string colorize(const string& text, const string& color) {
    return color + text + RESET;
}

string sha256Hex(const vector<unsigned char>& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data.data(), data.size());
    SHA256_Final(hash, &sha256);
    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

string sha512Hex(const vector<unsigned char>& data) {
    unsigned char hash[SHA512_DIGEST_LENGTH];
    SHA512_CTX sha512;
    SHA512_Init(&sha512);
    SHA512_Update(&sha512, data.data(), data.size());
    SHA512_Final(hash, &sha512);
    stringstream ss;
    for (int i = 0; i < SHA512_DIGEST_LENGTH; i++) {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

string md5Hex(const vector<unsigned char>& data) {
    unsigned char hash[MD5_DIGEST_LENGTH];
    MD5_CTX md5;
    MD5_Init(&md5);
    MD5_Update(&md5, data.data(), data.size());
    MD5_Final(hash, &md5);
    stringstream ss;
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

string sha1Hex(const vector<unsigned char>& data) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA_CTX sha1;
    SHA1_Init(&sha1);
    SHA1_Update(&sha1, data.data(), data.size());
    SHA1_Final(hash, &sha1);
    stringstream ss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

string base64Encode(const vector<unsigned char>& data) {
    // Simple base64 encoding (not used in this version for brevity, just hex)
    // We'll implement hex only for C++ to avoid extra dependencies.
    return "";
}

vector<unsigned char> readFile(const string& filename) {
    ifstream file(filename, ios::binary);
    if (!file) throw runtime_error("Cannot open file");
    return vector<unsigned char>((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
}

void writeFile(const string& filename, const string& content) {
    ofstream file(filename);
    if (!file) throw runtime_error("Cannot write file");
    file << content;
}

string computeHash(const vector<unsigned char>& data, const string& algo) {
    if (algo == "md5") return md5Hex(data);
    if (algo == "sha1") return sha1Hex(data);
    if (algo == "sha256") return sha256Hex(data);
    if (algo == "sha512") return sha512Hex(data);
    throw runtime_error("Unsupported algorithm");
}

int main(int argc, char* argv[]) {
    string algo = "sha256";
    string text, inputFile, outputFile, salt, compare, format = "hex";
    bool help = false;

    static struct option long_options[] = {
        {"algo", required_argument, 0, 'a'},
        {"text", required_argument, 0, 't'},
        {"input", required_argument, 0, 'i'},
        {"output", required_argument, 0, 'o'},
        {"salt", required_argument, 0, 's'},
        {"compare", required_argument, 0, 'c'},
        {"format", required_argument, 0, 'f'},
        {"help", no_argument, 0, 'h'},
        {0,0,0,0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "a:t:i:o:s:c:f:h", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'a': algo = optarg; break;
            case 't': text = optarg; break;
            case 'i': inputFile = optarg; break;
            case 'o': outputFile = optarg; break;
            case 's': salt = optarg; break;
            case 'c': compare = optarg; break;
            case 'f': format = optarg; break;
            case 'h': help = true; break;
            default: help = true; break;
        }
    }

    if (help) {
        cout << "Usage: hash [options]\n"
             << "Options:\n"
             << "  -a <algo>     Algorithm: md5, sha1, sha256, sha512 (default sha256)\n"
             << "  -t <text>     Text to hash\n"
             << "  -i <file>     Input file\n"
             << "  -o <file>     Output file\n"
             << "  -s <salt>     Salt string\n"
             << "  -c <hash>     Hash to compare against\n"
             << "  -f <format>   Output format: hex (only supported)\n"
             << "  -h            Show this help\n";
        return 0;
    }

    vector<unsigned char> data;
    try {
        if (!text.empty()) {
            data.assign(text.begin(), text.end());
        } else if (!inputFile.empty()) {
            data = readFile(inputFile);
        } else {
            // read from stdin
            string line;
            while (getline(cin, line)) {
                data.insert(data.end(), line.begin(), line.end());
                data.push_back('\n');
            }
            if (data.empty()) {
                cerr << colorize("No input data", RED) << endl;
                return 1;
            }
            // remove trailing newline if desired? Keep as is.
        }
    } catch (const exception& e) {
        cerr << colorize("Error: " + string(e.what()), RED) << endl;
        return 1;
    }

    // Add salt
    if (!salt.empty()) {
        vector<unsigned char> salted;
        salted.insert(salted.end(), salt.begin(), salt.end());
        salted.insert(salted.end(), data.begin(), data.end());
        data = move(salted);
    }

    string result;
    try {
        result = computeHash(data, algo);
    } catch (const exception& e) {
        cerr << colorize("Error: " + string(e.what()), RED) << endl;
        return 1;
    }

    if (!compare.empty()) {
        if (result == compare) {
            cout << colorize("✅ Hash matches!", GREEN) << endl;
        } else {
            cout << colorize("❌ Hash does NOT match.", RED) << endl;
            cout << "Expected: " << compare << endl;
            cout << "Computed: " << result << endl;
        }
        return 0;
    }

    if (!outputFile.empty()) {
        try {
            writeFile(outputFile, result);
            cout << colorize("Hash written to " + outputFile, GREEN) << endl;
        } catch (const exception& e) {
            cerr << colorize("Error writing file: " + string(e.what()), RED) << endl;
            return 1;
        }
    } else {
        cout << result << endl;
    }

    return 0;
}
