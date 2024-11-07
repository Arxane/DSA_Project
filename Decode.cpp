#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include "Decode.h"

using namespace std;

// Structure of a Node of a Huffman tree for decoding
struct Node {
    unsigned char character;
    Node *left;
    Node *right;
    
    Node(char c, Node *l = NULL, Node *r = NULL) : character(c), left(l), right(r) {}
};

// Rebuild Huffman tree from compressed file data
Node *Make_Huffman_tree(ifstream &input) {
    char ch;
    input.get(ch);
    if (ch == '1') {
        input.get(ch);
        return new Node(ch);
    } else {
        Node *left = Make_Huffman_tree(input);
        Node *right = Make_Huffman_tree(input);
        return new Node(-1, left, right);
    }
}

// Decode each binary symbol based on Huffman tree structure
void decode(ifstream &input, const string &output_filename, Node *Root, long long int Total_Freq) {
    ofstream output(output_filename.c_str(), ios::binary);
    if (!output.good()) {
        cerr << "Error: Could not create output file.\n";
        exit(-1);
    }

    bool eof_flag = false;
    char bits_8;
    Node *pointer = Root;
    while (input.get(bits_8)) {
        int counter = 7;
        while (counter >= 0) {
            if (!pointer->left && !pointer->right) {
                output << pointer->character;
                Total_Freq--;
                if (!Total_Freq) {
                    eof_flag = true;
                    break;
                }
                pointer = Root;
                continue;
            }
            if ((bits_8 & (1 << counter))) {
                pointer = pointer->right;
            } else {
                pointer = pointer->left;
            }
            counter--;
        }
        if (eof_flag) break;
    }
    output.close();
}

// Main function to decompress a file
bool decompressFile(const string &input_filename, const string &output_filename) {
    if (input_filename.find(".huf") == string::npos) {
        cerr << "Error: File does not have a .huf extension.\n";
        return false;
    }

    ifstream input_file(input_filename.c_str(), ios::binary);
    if (!input_file.good()) {
        cerr << "Error: Could not open input file.\n";
        return false;
    }

    cout << "\nDecompressing the file....";
    clock_t start_time = clock();

    long long int Total_freq = 0;
    char ch;
    while (input_file.get(ch)) {
        if (ch == ',') break;
        Total_freq *= 10;
        Total_freq += ch - '0';
    }

    Node *Huffman_tree = Make_Huffman_tree(input_file);
    input_file.get(ch); // Read extra space between compressed data and tree

    // Call the decode function, passing the correct parameters
    decode(input_file, output_filename, Huffman_tree, Total_freq);

    input_file.close();

    clock_t stop_time = clock();

    cout << "\n\nFile Decompressed Successfully!\n";
    cout << "Time taken to Decompress:\t" << double(stop_time - start_time) / CLOCKS_PER_SEC << " seconds\n";
    return true;
}

