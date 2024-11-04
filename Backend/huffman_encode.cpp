#include <iostream>
#include <fstream>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <vector>

using namespace std;

#define ascii_size 256 // max ascii character size

#define ll long long

//structure for Nodes in a tree

struct Node{
    unsigned char character;
    ll int frequency;
    Node* left;
    Node* right;

    Node(unsigned char c, ll int f, Node* l = NULL, Node* r = NULL){
        character = c;
        frequency = f;
        left = l;
        right = r;  
    }
};

//We need to maintain a min-heap for the operations

void heapify_min(vector<Node*>& a, int i, int len){
    int shift = i; //index of element we need to shift
    
    if(2*i+1<=len && a[2*i+1]->frequency<a[i]->frequency){ //checks if left node has frequency less than current node
        shift = 2*i+1;
        if (2*i+2<=len && a[2*i+2]->frequency<a[2*i+1]->frequency) //checks if right child has frequency less than left node
            shift = 2*i+2;
    }
    else if (2*i+2<=len && a[2*i+2]->frequency<a[i]->frequency) //If the left child does not exist or does not have a smaller frequency than the current node, checks if right child has frequency less than current node
        shift = 2 * i + 2;

    if (shift != i) {
        swap(a[i], a[shift]); //swap elements on intial index and index 'shift'
        heapify_min(a, shift, len); //recursively call heapify_min on new position where it was just moved
    }
}

Node* extract_min(vector<Node*>& a){
    if(a.size()<1) return NULL; //if vector is empty i.e. tree is empty

    Node* min = a[0]; //first element of min-heap is minimum element
    a[0] = a.back(); // the last element is moved to the first
    a.pop_back(); // remove the last element

    heapify_min(a, 0, a.size()-1); // heapify to preserve the property of min-heap
    return min;
}

void insert(vector<Node*>& a, Node* element){
    a.push_back(element); // insert new node at the end
    int idx = a.size()-1;

    while(idx>0 && a[(idx-1)/2]->frequency > a[idx]->frequency){ // check if parent node has greater frequency
        swap(a[idx], a[(idx-1)/2]); // swap if true
        idx = (idx-1)/2; // move to index of parent
    }
}

void Build_Minheap(vector<Node*>& a, int len) {
    for(int i = (len - 1) / 2; i >= 0; i--) { //build a min heap from a list of nodes
        heapify_min(a, i, len);
    }
}

void encoding(Node* root, char code[], int i, vector<long long int>& codemap){
    // code[] stores huffmann code path, as we build it
    //Now we traverse tree
    if(root->left){
        code[i] = '0';
        encoding(root->left, code, i+1, codemap);
    }

    if(root->right){
        code[i] = '1';
        encoding(root->right, code, i+1, codemap);
    }

    //if we reach a leaf node
    if(!root->left && !root->right){
        for(int j = i; j>=0; j--){
            if(j!=i){ //if j!=i, it means that its part of the code
                codemap[root->character] *= 10; //shift to left by one to make room for further encoding
                codemap[root->character] += code[i] - '0'; //character from code[i] converted to integer
            }
            else{
                codemap[root->character] = 1; //initialise by 1 meaning the code has been assigned
            }
        }
    }
}

void tree(ofstream& input, Node* root){ //ofstream gives a reference to a file where tree will be stored
    if(!root->left && !root->right){ // checks for leaf node
        input<< '1' <<root->character; //writes 1 to signify leaf node and then writes root->character
    }
    // using '1' for leaf nodes helps program differentiate between leaf nodes and internal nodes
    
    else{
        input<<'0'; //writes '0' to signify internal node
        tree(input, root->left);
        tree(input, root->right);
        // recursively calls left an right subtreees
    }
}

Node* Huffman_main(long long int frequency[]){  //takes an array of character frequencies
    vector<Node*> minheap;

    for(int i = 0; i<ascii_size; i++){ // iterates through all possible ascii characters
        if(frequency[i]!=0){ //checks if characters is present in file
            minheap.push_back(new Node(i, frequency[i])); //creates a new node with i as character and frequency[i] as frequency
        }
    }

    Build_Minheap(minheap, minheap.size()-1);

    while(minheap.size()!=1){
        Node* temp = new Node(-1, 0, extract_min(minheap), extract_min(minheap)); //-1 acts as placeholder, and 0 as temporary frequency, and frequency is then set to sum of child frequencies
        temp->frequency = temp->left->frequency + temp->right->frequency;
        insert(minheap, temp);
    }

    return minheap[0]; //return root;
}

void write_compressed(ifstream& input, ofstream& output, vector<long long int>& codemap){ //ifstream gives reference for reading, and ofstream to a file where compressed data will be stored
    char c; //temp variable to store each character
    unsigned char bits; //8-bit buffer
    long long int count = 0; //keeps track of current bits

    while(input.get(c)) { //reads each character
        long long int temp = codemap[static_cast<unsigned char>(c)];

        while(temp!=1){ //iterates through each bit
            bits<<=1; //shifts left by 1 position
            if((temp%10)!=0){ // checks if last digit is 1
                bits |= 1; //sets least significant bit as 1
            }
            temp /= 10; //removes last digit from temp
            count++;
        }

        if(count == 8){ //full byte has been accumulated
            output<<bits; //writes full byte
            count = bits = 0; //resets counter and bits
        }
    }

    while(count != 8){ //pads bits with 0s on right
        bits<<=1;
        count++;
    }

    output<<bits;
    output.close();
}

int main(int argc, char* argv[]){ //takes in command line arguments
    vector<long long int> codemap; //vector to store huffman codes
    codemap.resize(ascii_size);
    long long int count[ascii_size] = {0}; //store frequency of each character

    string filename;
    cout<<"Enter Filename: ";
    cin>>filename;

    ifstream input_file(filename.c_str(), ios::binary); //opens the file in binary mode
    if(!input_file.good()){ //checks if file can be opened or not
        perror("Error: ");
        exit(-1);
    }

    cout<<"Compressing the file!";
    clock_t start_time = clock(); //recording time
    char c;

    while(input_file.get(c)){ //reads each character
        count[static_cast<unsigned char>(c)]++; //updates frequency
    }
    //resetting input file stream
    input_file.clear();
    input_file.seekg(0);

    Node* huffman_tree = Huffman_main(count);
    ofstream output_file((filename + ".huf").c_str(), ios::binary);//opens a file with same name but huf extension
    if(!output_file.good()){
        perror("Error: ");
        exit(-1);
    }

    output_file<<huffman_tree->frequency; //frequencies written to output file
    output_file<<',';
    tree(output_file, huffman_tree); //writes huffman tree structure to output file
    output_file<<' ';

    char code[16]; // store temporary binary code
    encoding(huffman_tree, code, 0, codemap); //traverses huffman tree and stores codes

    write_compressed(input_file, output_file, codemap);

    input_file.close();
    output_file.close();

    clock_t stop_time = clock();
    if(remove(filename.c_str())!=0){
        perror("Error deleting the compressed file: ");
    }

    cout<<"Time taken to compress: "<<double(stop_time-start_time) / CLOCKS_PER_SEC<<" seconds\n\n";
}