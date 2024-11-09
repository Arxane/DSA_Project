# Huffman Coding compression and decompressio system

This project implements a data compression and decompression system using Huffman Coding, a popular lossless data compression algorithm. The system reads a file, compresses it into a smaller format using Huffman coding, and supports decompression to recover the original content.
# Features

 -> Efficient Compression: Utilizes Huffman coding to assign variable-length codes to characters based on their frequencies.
 -> Data Decompression: Supports decompression to restore the original file content from the compressed data.
 -> Performance Measurement: Tracks compression ratio, original and compressed file sizes, and processing time.
 -> Graph Visualization: Optionally includes visual tools to display compression ratios over time.
 
 Huffman coding is a greedy algorithm that uses the frequency of each character in the file to generate an optimal binary tree where the most frequent characters have the shortest codes. This results in an efficient representation of the data, reducing file size without losing any information.
# Steps to Huffman Compression
-> Frequency Calculation : Count the frequency of each character in the input file.
-> Huffman Tree construction : Build a binary tree where each character becomes a leaf node with a path determined by its frequency.
-> Code Assignment : Assign binary codes to each character based on their position in the tree.
-> Compression : Encode the file using the generated Huffman codes, replacing each character with its binary code.
-> Decompression : Use the Huffman tree to decode the compressed binary data back into the original characters.

# Requirements
-> C++ compiler (GCC)
-> FLTK (Fast Light Toolkit) for GUI elements

# Usage
-> Launch the application
-> Use the file chooser dialogs to choose input file (*.txt) and output file (*.huf) for compression.
-> Use the file chooser dialogs to choose input file (*.huf) and output file (*.txt) for decompression.
-> View the status output to see the compression ratio and processing time.

# Performance
-> The program displays compression ratio and execution time, allowing users to analyze compression effeciency.

# Future Enhancements
-> Additional Compression Algorithms like LZW, Arithmetic coding.
-> Multi-threading : Enable concurrent encoding and decoding for large files.
