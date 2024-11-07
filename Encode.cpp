#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <cstdlib>
#include "Encode.h"

using namespace std;

#define Char_size 256 // ASCII character set only

// Structure of Node of Huffman tree
struct Node {
    unsigned char character;
    long long int Freq;
    Node *left;
    Node *right;

    Node(unsigned char c, long long int f, Node *l = NULL, Node *r = NULL) 
        : character(c), Freq(f), left(l), right(r) {}
};

// Heapify function
void Mindownheap(vector<Node *> &A, int i, int length) {
    int least = i;
    if (2 * i + 1 <= length && A[2 * i + 1]->Freq < A[i]->Freq) {
        least = 2 * i + 1;
        if (2 * i + 2 <= length && A[2 * i + 2]->Freq < A[2 * i + 1]->Freq)
            least = 2 * i + 2;
    } else if (2 * i + 2 <= length && A[2 * i + 2]->Freq < A[i]->Freq)
        least = 2 * i + 2;

    if (least != i) {
        swap(A[i], A[least]);
        Mindownheap(A, least, length);
    }
}

// Extract minimum character from min-heap
Node *Extract_min(vector<Node *> &A) {
    if (A.empty()) return NULL;
    Node *minimum = A[0];
    A[0] = A.back();
    A.pop_back();
    Mindownheap(A, 0, A.size() - 1);
    return minimum;
}

// Insert Character in Min-heap
void Insert_MinHeap(vector<Node *> &A, Node *element) {
    A.push_back(element);
    int i = A.size() - 1;
    while (i > 0 && A[(i - 1) / 2]->Freq > A[i]->Freq) {
        swap(A[i], A[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

// Build min-heap from nodes
void Build_Minheap(vector<Node *> &A, int length) {
    for (int i = (length - 1) / 2; i >= 0; i--) {
        Mindownheap(A, i, length);
    }
}

// Store code for each character in a vector
void store_codes(Node *Root, char single_code[], int index, vector<long long int> &Huffman_codemap) {
    if (Root->left) {
        single_code[index] = '0';
        store_codes(Root->left, single_code, index + 1, Huffman_codemap);
    }
    if (Root->right) {
        single_code[index] = '1';
        store_codes(Root->right, single_code, index + 1, Huffman_codemap);
    }
    if (!Root->left && !Root->right) {
        for (int i = index; i >= 0; i--) {
            if (i != index) {
                Huffman_codemap[Root->character] *= 10;
                Huffman_codemap[Root->character] += single_code[i] - '0';
            } else
                Huffman_codemap[Root->character] = 1;
        }
    }
}

// Write tree to file
void store_tree(ofstream &output, Node *Root) {
    if (!Root->left && !Root->right) {
        output << '1';
        output << Root->character;
    } else {
        output << '0';
        store_tree(output, Root->left);
        store_tree(output, Root->right);
    }
}

// Main Huffman Algorithm
Node *Huffman(long long int Count[]) {
    vector<Node *> minheap;
    for (int i = 0; i < Char_size; i++)
        if (Count[i] != 0)
            minheap.push_back(new Node(i, Count[i]));
    Build_Minheap(minheap, minheap.size() - 1);
    while (minheap.size() != 1) {
        Node *Z = new Node(-1, 0, Extract_min(minheap), Extract_min(minheap));
        Z->Freq = Z->left->Freq + Z->right->Freq;
        Insert_MinHeap(minheap, Z);
    }
    return minheap[0];
}

// Write compressed data to file
void Write_compressed(ifstream &input, ofstream &output, vector<long long int> &Huffman_codemap) {
    char ch;
    unsigned char bits_8 = 0;
    long long int counter = 0;
    while (input.get(ch)) {
        long long int temp = Huffman_codemap[static_cast<unsigned char>(ch)];
        while (temp != 1) {
            bits_8 <<= 1;
            if ((temp % 10) != 0)
                bits_8 |= 1;
            temp /= 10;
            counter++;
            if (counter == 8) {
                output << bits_8;
                counter = bits_8 = 0;
            }
        }
    }
    while (counter != 8) {
        bits_8 <<= 1;
        counter++;
    }
    output << bits_8;
}

// Compress a file and save as output.huf
// Modify compressFile to take an output path
// Compress the file and save as output.huf
bool compressFile(const std::string &input_filename, const std::string &output_filename) {
    std::vector<long long int> Huffman_codemap(Char_size, 0);
    long long int Count[Char_size] = {0};

    // Open the input file in binary mode
    std::ifstream input_file(input_filename, std::ios::binary);
    if (!input_file.good()) {
        std::cerr << "Error: Could not open input file.\n";
        return false;
    }

    // Count the frequency of each character in the input file
    char ch;
    while (input_file.get(ch)) {
        Count[static_cast<unsigned char>(ch)]++;
    }
    input_file.clear();  // Reset file pointer to beginning
    input_file.seekg(0);

    // Create the Huffman tree based on the character frequencies
    Node *tree = Huffman(Count);

    // Open the output file in binary mode
    std::ofstream output_file(output_filename, std::ios::binary);
    if (!output_file.good()) {
        std::cerr << "Error: Could not create output file at " << output_filename << "\n";
        return false;
    }

    // Store the frequency and the Huffman tree structure
    output_file << tree->Freq << ',';
    store_tree(output_file, tree);
    output_file << ' ';

    // Store the Huffman codes for each character
    char single_code[16];
    store_codes(tree, single_code, 0, Huffman_codemap);

    // Write the compressed data to the output file
    Write_compressed(input_file, output_file, Huffman_codemap);

    // Close both files
    input_file.close();
    output_file.close();

    return true;
}
