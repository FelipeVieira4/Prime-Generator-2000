#include <iostream>
#include <fstream>
#include <vector>

typedef int64_t Number;

using namespace std;

// Function to read primes from the binary file
void ReadPrimesFromBinaryFile(const string& filename) {
    ifstream inFile(filename, ios::binary);  // Open file in binary mode
    if (!inFile) {
        cerr << "Error opening file for reading!" << endl;
        return;
    }

    // Read all primes into a vector
    vector<Number> primes;
    Number prime;

    // Read primes one by one until we reach the end of the file
    while (inFile.read(reinterpret_cast<char*>(&prime), sizeof(prime))) {
        primes.push_back(prime);  // Store the prime
    }

    inFile.close();

    // Print the first few primes to check
    for (size_t i = 0; i < primes.size(); ++i) {
        cout << primes[i] << " ";
    }
    cout << endl;
}

int main() {
    // Example of reading from the binary file
    string filename = "";  // The file where primes were saved
    std::cout << "Insert the name of the .bin file you want to open: ";
    std::cin >> filename;
    ReadPrimesFromBinaryFile(filename);

    return 0;
}
