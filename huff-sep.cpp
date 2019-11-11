#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <chrono>
#define ll unsigned long long int

using namespace std;
using namespace std::chrono;

string HuffmanValue[256] = {""};

typedef struct Node {
public:
	char character;
	ll count;
	Node *left, *right;

	Node(ll count) {
		this->character = 0;
		this->count = count;
		this->left = this->right = NULL;
	}

	Node(char character, ll count) {
		this->character = character;
		this->count = count;
		this->left = this->right = NULL;
	}
} Node;

// Initial Pass for counting characters.
map <char, ll> ParseFile(const char* filename, ll Filesize) {
	register FILE *ptr = fopen(filename, "rb");

	if (ptr == NULL) {
		perror("Error: File not found:");
		exit(-1);
	}
	
	register unsigned char ch;
	register ll size = 0, filesize = Filesize;
	ll Store[256];
	for(int i = 0; i < 256; ++i)
		Store[i] = 0;
	
	while(size != filesize) {
		ch = fgetc(ptr);
		++Store[ch];
		++size;
	}
	map <char, ll> store;
	for(int i = 0; i < 256; ++i)
		if(Store[i])
			store[i] = Store[i];
	fclose(ptr);
	return store;
}
/*
Combine two nodes:
          parent
          /    \
        b       a
*/
Node *Combine(Node *a, Node *b) {
	Node *parent = new Node(a->count+b->count);
	parent->left = b;
	parent->right = a;
	return parent;
}

bool sortbysec(Node *a, Node *b) { 
	return (a->count > b->count); 
}

vector <Node *> SortByCharacterCount(map <char, ll > value) {
	vector < Node* > store;
	map <char, ll> :: iterator it;

	for(it = value.begin(); it != value.end(); ++it) 
		store.push_back(new Node(it->first, it->second));
	sort(store.begin(), store.end(), sortbysec);

	return store;
}
// Store Huffman values for each character in string.
ll StoreHuffmanValue(Node *root, string value) {
	ll temp = 0;  
	if(root != NULL) {
		temp = StoreHuffmanValue(root->left, value+"0");
		if (root->left == NULL && root->right == NULL) {
			HuffmanValue[(unsigned char)root->character] = value;
			temp += value.length()*root->count;
		}
		temp += StoreHuffmanValue(root->right, value+"1");
	}
	return temp;
}
// Test function to print huffman codes for each character.
void Inorder(Node *root, string value) {
	if(root != NULL) {
		Inorder(root->left, value+"0");
		if (root->left == NULL && root->right == NULL) {
			printf("Character: %c, Count: %lld, ", root->character, root->count);
			cout << "Huffman Value: " << value << endl;
		}
		Inorder(root->right, value+"1");
	}
}
// Generate a header for the file.
// Format: 
// 1. Total Unique Character (1 byte)
// 2. For each unique character:
//    a. Character (1 byte)
//    b. Length of code (1 byte)
//    c. Huffman code (min: 1 byte, max: 255bytes)
// Worst case header size: 1 + (1+1)*(1+2+3+4+5+...+255) ~ 65kb... (only happens when skewed Huffman tree is generated)
// Best case header size: 1 + 1 + 1 + 1 (Happens only when a single character exists in an entire file).
string GenerateHeader(char padding) {
	string header = "";
	register unsigned char UniqueCharacter = 255;
	register int var = 0;
	register unsigned char j = 0;
	
	for(register int i = 0; i < 256; ++i) {
		var = 0; j = 0;
		if(HuffmanValue[i] != "") {
			
			string temp = HuffmanValue[i];
			
			while(temp[j] != '\0') 
				++j;
			header.push_back(i);
			header.push_back(j);
			header += temp;
			++UniqueCharacter;
		}
	}
	char value = UniqueCharacter;
	
	return value+header+(char)padding;

}
// Create huffman tree during decompression...
void GenerateHuffmanTree(Node *root, string codes, unsigned char ch) {
	Node *traverse = root;
	int i = 0;
	while(codes[i] != '\0') {

		if(codes[i] == '0')
			if(traverse->left == NULL) {
				Node *newnode = new Node(0);
				traverse->left = newnode;
				traverse = newnode;
			} else 
				traverse = traverse->left;
		else
			if(traverse->right == NULL) {
				Node *newnode = new Node(0);
				traverse->right = newnode;
				traverse = newnode;
			} else 
				traverse = traverse->right;
		++i;

	}
	traverse->character = ch;
}
// Function to store and generate a tree...
pair<Node *, pair<unsigned char, int> >DecodeHeader(FILE *iptr) {
	
	Node *root;
	Node *rootnode = new Node(0);

	root = rootnode;
	register int charactercount, buffer, j, total_length = 0;
	register char ch, len;
	charactercount = fgetc(iptr);
	++total_length;
	string codes;

	++charactercount;

	while(charactercount) {
		ch = fgetc(iptr);
		++total_length;
		j = 0;
		codes = ""; buffer = 0;
		len = fgetc(iptr);
		++total_length;
		buffer = len;

		while(buffer > j) {
			codes += fgetc(iptr);
			++j;
		}
		total_length += codes.length();

		GenerateHuffmanTree(root, codes, ch);
		--charactercount;
	}
	unsigned char padding = fgetc(iptr);
	++total_length;
	return {root, {padding, total_length}};
}
// Create huffman tree during compression...
Node *GenerateHuffmanTree(map <char, ll > value) {
	vector < Node* > store = SortByCharacterCount(value);
	Node *one, *two, *parent;
	ll size = store.size();
	sort(store.begin(), store.end(), sortbysec);

	while (size > 2) {
		one = *(store.end()-1); two = *(store.end()-2);
		parent = Combine(one, two);
		store.pop_back(); store.pop_back();
		store.push_back(parent);

		vector <Node *> :: iterator it1 = store.end()-2;
		while((*it1)->count < parent->count && it1 != store.begin()) 
			--it1;
		
		--size;
		sort(it1, store.end(), sortbysec);
	}
	one = *(store.end()-1); two = *(store.end()-2);
	parent = Combine(one, two);
	return parent;
}

void Compress(const char *filename, ll Filesize, ll PredictedFileSize) {
	
	char padding = (8 - ((PredictedFileSize)&(7)))&(7);
	string header = GenerateHeader(padding);
	int header_i = 0, h_length = header.length();
	cout << "Padding size: " << (int)padding << endl;
	FILE *optr = fopen((string(filename)+".abiz").c_str(), "wb");
	
	while(header_i < h_length) {
		fputc(header[header_i], optr);
		++header_i;
	}

	FILE *iptr = fopen(filename, "rb");
	
	if (iptr == NULL) {
		perror("Error: File not found: ");
		exit(-1);
	}

	register char ch, fch = 0;
	register const char _7 = 7, _c0 = '0', _NULL = '\0';
	register char counter = _7;
	ll size = 0;
	ll filesize = Filesize;
	int i;
	string huffcode;
	while(size != filesize) {
		ch = fgetc(iptr);
		huffcode = HuffmanValue[(unsigned char)ch];
		i = 0;
		while(huffcode[i] != _NULL) {
			fch = fch | ((huffcode[i] - _c0)<<counter);
			--counter;
			if(counter == -1) {
				fputc(fch, optr);
				counter = _7;
				fch = 0;
			}
			++i;
		}
		++size;
		// if(((size*100/filesize)) > ((size-1)*100/filesize))
		// 	printf("\r%d%% completed  ", (size*100/filesize));
	}
	if(fch) 
		fputc(fch, optr);

	fclose(iptr);
	fclose(optr);
}

ll GetFileSize(const char *filename) {
	FILE *p_file = fopen(filename, "rb");
	fseek(p_file, 0, SEEK_END);
	ll size = ftello64(p_file);
	fclose(p_file);
	return size;
}

void Decompress(const char*filename, ll Filesize, ll leftover) {
	string fl = string(filename);
	FILE *iptr = fopen(fl.c_str(), "rb");
	FILE *optr = fopen(string("output"+fl.substr(0, fl.length()-5)).c_str(), "wb");
	
	if (iptr == NULL) {
		perror("Error: File not found");
		exit(-1);
	}

	pair<Node *, pair<unsigned char, int> >HeaderMetadata = DecodeHeader(iptr);
	Node *root = HeaderMetadata.first;
	unsigned char padding = HeaderMetadata.second.first;
	unsigned int headersize = HeaderMetadata.second.second;

	register const char _1 = 1;
	register char ch, counter = 7;
	register ll size = 0;
	register ll filesize = Filesize-headersize;
	register Node *traverse = root;
	ch = fgetc(iptr);
	while(size != filesize) {
		while(counter > -_1) {
			traverse = ch & (_1<<counter)  ? (*traverse).right : (*traverse).left;
			ch ^= (_1<<counter);
			--counter;
			if((*traverse).left == NULL && (*traverse).right == NULL) {

				fputc((*traverse).character, optr);
				if(size == filesize-_1) {
					if(padding == counter+_1)
						break;
				}
				traverse = root;
			}
		}
		++size;
		counter = 7;
		// if(((size*100/filesize)) > ((size-_1)*100/filesize)) 
		// 	printf("\r%d%% completed, size: %d bytes   ", (size*100/filesize), size);
		ch = fgetc(iptr);
	}
	fclose(iptr);
	fclose(optr);
}

int main(int argc, char *argv[]) {
	
	if(argc != 3) {
		printf("Usage:\n (a.exe|./a.out) -c|-dc [FileToBeCompressed]");
		exit(-1);
	}
	const char *option = argv[1], *filename = argv[2];
	printf("%s\n", filename);
	
	time_point <system_clock> start, end; 
	duration <double> time;
	ll filesize, predfilesize;
	if(string(option) == "-c") {
	
		filesize = GetFileSize(filename);
		Node *root = GenerateHuffmanTree(ParseFile(filename, filesize));
		predfilesize = StoreHuffmanValue(root, string(""));
		printf("Original File: %lld bytes\n", filesize);
		printf("Compressed File Size: %lld bytes\n", (predfilesize+7)/8);
	
		start = system_clock::now();
		Compress(filename, filesize, predfilesize);
		end = system_clock::now();

		time = (end-start);
		cout << "Compression Time: " << time.count() << "s" << endl;
	
	}
	else if(string(option) == "-dc"){

		filesize = GetFileSize(filename);
		start = system_clock::now();
		Decompress(filename, filesize, predfilesize);
		end = system_clock::now();

		time = (end-start);
		cout << "\nDecompression Time: " << time.count() << "s" << endl;

	}else {
		cout << "\nInvalid Option... Exiting\n";
	}
	return 0;
	
}
