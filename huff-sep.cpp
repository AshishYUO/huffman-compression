#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <chrono>
#define ll unsigned long long int

using namespace std;
using namespace std::chrono;
/*
	Compression Library
*/
namespace Huffman {
	
	string HuffmanValue[256] = {""};
	
	typedef struct Node {
	public:
		char character;
		ll count;
		Node *left, *right;

		Node(ll count) {
			this->character = 0;
			this->count = count;
			this->left = this->right = nullptr;
		}

		Node(char character, ll count) {
			this->character = character;
			this->count = count;
			this->left = this->right = nullptr;
		}
	} Node;
	/*
		Common function necessary for both compression and decompression.
	*/
	namespace Utility {

		ll GetFileSize(const char *filename) {
			FILE *p_file = fopen(filename, "rb");
			fseek(p_file, 0, SEEK_END);
			ll size = ftello64(p_file);
			fclose(p_file);
			return size;
		}
		// Test function to print huffman codes for each character.
		void Inorder(Node *root, string &value) {
			if(root) {
				value.push_back('0');
				Inorder(root->left, value);
				value.pop_back();
				if (!root->left && !root->right) {
					printf("Character: %c, Count: %lld, ", root->character, root->count);
					cout << "Huffman Value: " << value << endl;
				}
				value.push_back('1');
				Inorder(root->right, value);
				value.pop_back();
			}
		}
	};

	/*
		Functions necessary for compression.
	*/
	namespace CompressUtility {
		/*
		Combine two nodes
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
		// Initial Pass for counting characters.
		map <char, ll> ParseFile(const char* filename, ll Filesize) {
			register FILE *ptr = fopen(filename, "rb");

			if (ptr == NULL) {
				perror("Error: File not found:");
				exit(-1);
			}
			register unsigned char ch;
			register ll size = 0, filesize = Filesize;
			vector<ll>Store(256, 0);

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

		vector <Node *> SortByCharacterCount(const map <char, ll >&value) {
			vector < Node* > store;
			auto it = begin(value);
			for(; it != end(value); ++it) 
				store.push_back(new Node(it->first, it->second));
			sort(begin(store), end(store), sortbysec);

			return store;
		}
		// Generate a header for the file.
		// Format: 
		// 1. Total Unique Character (1 byte)
		// 2. For each unique character:
		// 2a. Character (1 byte)
		// 2b. Length of code (1 byte)
		// 2c. Huffman code (min: 1 byte, max: 255bytes)
		// 3. Padding
		// Worst case header size: 1 + (1+1)*(1+2+3+4+5+...+255) + 1 ~ 32kb... (only happens when skewed Huffman tree is generated)
		// Best case header size: 1 + 1 + 1 + 1 + 1 = 5bytes (Happens only when a single character exists in an entire file).
		string GenerateHeader(char padding) {
			string header = "";
			// UniqueCharacter start from -1 {0 means 1, 1 means 2, to conserve memory}
			unsigned char UniqueCharacter = 255;
			
			for(int i = 0; i < 256; ++i) {
				if(HuffmanValue[i].size()) {
					header.push_back(i);
					header.push_back(HuffmanValue[i].size());
					header += HuffmanValue[i];
					++UniqueCharacter;
				}
			}
			char value = UniqueCharacter;
			
			return value+header+(char)padding;
		}

		// Store Huffman values for each character in string.
		// returns the size of the resulting file (without the header)
		ll StoreHuffmanValue(Node *root, string &value) {
			ll temp = 0;  
			if(root) {
				value.push_back('0');
				temp = StoreHuffmanValue(root->left, value);
				value.pop_back();
				if (!root->left && !root->right) {
					HuffmanValue[(unsigned char)root->character] = value;
					temp += value.size()*root->count;
				}
				value.push_back('1');
				temp += StoreHuffmanValue(root->right, value);
				value.pop_back();
			}
			return temp;
		}
		// Create huffman tree during compression...
		Node *GenerateHuffmanTree(const map <char, ll>&value) {
			vector < Node* > store = SortByCharacterCount(value);
			Node *one, *two, *parent;
			sort(begin(store), end(store), sortbysec);

			while (store.size() > 2) {
				one = *(end(store)-1); two = *(end(store)-2);
				parent = Combine(one, two);
				store.pop_back(); store.pop_back();
				store.push_back(parent);

				vector <Node *> :: iterator it1 = end(store)-2;
				while((*it1)->count < parent->count && it1 != begin(store)) 
					--it1;
				sort(it1, end(store), sortbysec);
			}
			one = *(end(store)-1); two = *(end(store)-2);
			parent = Combine(one, two);
			return parent;
		}
		// Actual compression of a file.
		void Compress(const char *filename, const ll Filesize, const ll PredictedFileSize) {
			const char padding = (8 - ((PredictedFileSize)&(7)))&(7);
			string header = GenerateHeader(padding);
			int header_i = 0;
			const int h_length = header.size();
			cout << "Padding size: " << (int)padding << endl;
			FILE *iptr = fopen(filename, "rb"), *optr = fopen((string(filename)+".abiz").c_str(), "wb");
			
			while(header_i < h_length) {
				fputc(header[header_i], optr);
				++header_i;
			}

			if (!iptr) {
				perror("Error: File not found: ");
				exit(-1);
			}

			unsigned char ch, fch = 0;
			char counter = 7;
			ll size = 0, i;
			while(size != Filesize) {
				ch = fgetc(iptr);
				i = 0;
				while(HuffmanValue[ch][i] != '\0') {
					fch = fch | ((HuffmanValue[ch][i] - '0')<<counter);
					--counter;
					if(counter == -1) {
						fputc(fch, optr);
						counter = 7;
						fch = 0;
					}
					++i;
				}
				++size;
				if(((size*100/Filesize)) > ((size-1)*100/Filesize))
					printf("\r%d%% completed  ", (size*100/Filesize));
			}
			if(fch) 
				fputc(fch, optr);
			printf("\n");
			fclose(iptr);
			fclose(optr);
		}

	};
	/*
		Functions necessary for decompression.
	*/
	namespace DecompressUtility {
		// Create huffman tree during decompression
		void GenerateHuffmanTree(Node * const root, const string &codes, const unsigned char ch) {
			Node *traverse = root;
			int i = 0;
			while(codes[i] != '\0') {
				if(codes[i] == '0') {
					if(!traverse->left)
						traverse->left = new Node(0);
					traverse = traverse->left;
				} else {
					if(!traverse->right) 
						traverse->right = new Node(0);
					traverse = traverse->right;
				}
				++i;
			}
			traverse->character = ch;
		}
		// Function to store and generate a tree
		pair<Node*, pair<unsigned char, int> >DecodeHeader(FILE *iptr) {
			Node *root = new Node(0);
			int charactercount, buffer, total_length = 1;
			register char ch, len;
			charactercount = fgetc(iptr);
			string codes;
			++charactercount;
			while(charactercount) {
				ch = fgetc(iptr);
				codes = ""; 
				buffer = 0;
				len = fgetc(iptr);
				buffer = len;

				while(buffer > codes.size()) 
					codes += fgetc(iptr);
				// character (1byte) + length(1byte) + huffmancode(n bytes where n is length of huffmancode)
				total_length += codes.size()+2;

				GenerateHuffmanTree(root, codes, ch);
				--charactercount;
			}
			unsigned char padding = fgetc(iptr);
			++total_length;
			return {root, {padding, total_length}};
		}
		// Actual decompression function
		void Decompress(const char*filename, const ll Filesize, const ll leftover) {
			string fl = string(filename);
			FILE *iptr = fopen(fl.c_str(), "rb");
			FILE *optr = fopen(string("output"+fl.substr(0, fl.length()-5)).c_str(), "wb");
			
			if (iptr == NULL) {
				perror("Error: File not found");
				exit(-1);
			}

			pair<Node*, pair<unsigned char, int> >HeaderMetadata = DecodeHeader(iptr);
			Node *const root = HeaderMetadata.first;
			const auto padding = HeaderMetadata.second.first;
			const auto headersize = HeaderMetadata.second.second;

			char ch, counter = 7;
			ll size = 0;
			const ll filesize = Filesize-headersize;
			Node *traverse = root;
			ch = fgetc(iptr);
			while(size != filesize) {
				while(counter >= 0) {
					traverse = ch & (1<<counter) ? traverse->right : traverse->left;
					ch ^= (1<<counter);
					--counter;
					if(!traverse->left && !traverse->right) {
						fputc(traverse->character, optr);
						if(size == filesize-1 && padding == counter+1) 
							break;
						traverse = root;
					}
				}
				++size;
				counter = 7;
				if(((size*100/filesize)) > ((size-1)*100/filesize)) 
					printf("\r%lld%% completed, size: %lld bytes   ", (size*100/filesize), size);
				ch = fgetc(iptr);
			}
			fclose(iptr);
			fclose(optr);
		}
	};
};

using namespace Huffman;

int main(int argc, char *argv[]) {
	
	if(argc != 3) {
		printf("Usage:\n (a.exe|./a.out) (-c FileToBeCompressed | -dc FileToBeDecompressed)");
		exit(-1);
	}
	const char *option = argv[1], *filename = argv[2];
	printf("%s\n", filename);
	
	time_point <system_clock> start, end; 
	duration <double> time;
	ll filesize, predfilesize;
	if(string(option) == "-c") {
	
		filesize = Utility::GetFileSize(filename);
		auto mapper = CompressUtility::ParseFile(filename, filesize);
		Node *const root = CompressUtility::GenerateHuffmanTree(mapper);
		string buf = "";
		predfilesize = CompressUtility::StoreHuffmanValue(root, buf);
		printf("Original File: %lld bytes\n", filesize);
		printf("Compressed File Size (without header): %lld bytes\n", (predfilesize+7)>>3);
	
		start = system_clock::now();
		CompressUtility::Compress(filename, filesize, predfilesize);
		end = system_clock::now();

		time = (end-start);
		cout << "Compression Time: " << time.count() << "s" << endl;
	
	}
	else if(string(option) == "-dc"){
		filesize = Utility::GetFileSize(filename);
		start = system_clock::now();
		DecompressUtility::Decompress(filename, filesize, predfilesize);
		end = system_clock::now();

		time = (end-start);
		cout << "\nDecompression Time: " << time.count() << "s" << endl;
	} else 
		cout << "\nInvalid Option... Exiting\n";
	return 0;
	
}
