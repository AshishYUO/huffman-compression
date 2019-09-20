#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#define ll unsigned long long int

using namespace std;

string HuffmanValue[256] = {""};

class Node {
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
};

bool sortbysec(Node *a, Node *b) { 
	return (a->count > b->count); 
} 

map <char, ll> ParseFile(const char* filename, ll filesize) {
	FILE *ptr = fopen(filename, "rb");
	if (ptr == NULL) {
		perror("Error: File not found:");
		exit(-1);
	}
	char ch;
	ll size = 0;
	map <char, ll> store;
	while(size != filesize) {
		ch = fgetc(ptr);
		++store[ch];
		++size;
	}
	fclose(ptr);
	return store;
}

Node *Combine(Node *a, Node *b) {
	Node *parent = new Node(a->count+b->count);
	parent->left = b;
	parent->right = a;
	return parent;
}

vector <Node *> SortByCharacterCount(map <char, ll > value) {
	vector < Node* > store;
	map <char, ll> :: iterator it;

	for(it = value.begin(); it != value.end(); ++it) 
		store.push_back(new Node(it->first, it->second));
	sort(store.begin(), store.end(), sortbysec);

	return store;
}

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

ll Compress(const char *filename, ll filesize) {
	FILE *iptr = fopen(filename, "rb");
	FILE *optr = fopen((string( filename)+".abiz").c_str(), "wb");
	
	if (iptr == NULL) {
		perror("Error: File not found: ");
		exit(-1);
	}
	
	char ch, fch, counter = 7;
	ll size = 0;
	short i = 0;
	
	while(size != filesize) {
		ch = fgetc(iptr);
		string huffcode = HuffmanValue[(unsigned char)ch];
		i = 0;
		while(huffcode[i] != '\0') {
			fch = fch | ((huffcode[i] - '0')<<counter);
			--counter;
			if(counter == -1) {
				fputc(fch, optr);
				counter = 7;
				fch = 0;
			}
			++i;
		}
		++size;
		if(((size*100/filesize)) > ((size-1)*100/filesize))
			printf("\r%d%% completed  ", (size*100/filesize));
	}
	if(fch) 
		fputc(fch, optr);

	fclose(iptr);
	fclose(optr);
	printf("\n");
	return counter+1;
}

int GetFileSize(const char *filename) {
	FILE *p_file = fopen(filename, "rb");
	fseek(p_file, 0, SEEK_END);
	// If the file size is bigger than 2^31 bytes(or 2 GB), consider using ftello64 instead of ftell
	int size = ftell(p_file);
	fclose(p_file);
	return size;
}

void Decompress(const char*filename, ll filesize, ll leftover, Node *root) {
	FILE *iptr = fopen((string(filename)+".abiz").c_str(), "rb");
	FILE *optr = fopen(("output"+string(filename).substr(string(filename).find_last_of("."))).c_str(), "wb");
	
	if (iptr == NULL) {
		perror("Error: File not found: ");
		exit(-1);
	}
	
	char ch, counter = 7, x;
	ll size = 0;
	Node *traverse = root;
	unsigned char temp = 0;
	ch = fgetc(iptr);
	printf("Total filesize: %d\n", filesize);
	while(size != filesize) {
		while(counter > -1) {
			traverse = ( ch & (1<<counter) ) ? traverse->right : traverse->left;
			ch ^= (1<<counter);
			--counter;
			if(traverse->left == NULL && traverse->right == NULL) {
				fputc(traverse->character, optr);
				if(size == filesize-1 && leftover == counter+1) 
					break;
				traverse = root;
			}
		}
		++size;
		counter = 7;
		if(((size*100/filesize)) > ((size-1)*100/filesize)) 
			printf("\r%d%% completed, size:%d bytes    ", (size*100/filesize), size);
		ch = fgetc(iptr);
	}
	fclose(iptr);
	fclose(optr);
}

int main(int argc, char *argv[]) {
	const char *filename = argv[1];
	printf("%s\n", filename);
	ll filesize = GetFileSize(filename);
	Node *root = GenerateHuffmanTree(ParseFile(filename, filesize));
	ll predfilesize = StoreHuffmanValue(root, string(""));
	printf("Original File: %lld\n", filesize);
	printf("Compressed File Size: %lld\n", (predfilesize+7)/8);
	
	ll size = Compress(filename, filesize);
	Inorder (root, string(""));
	Decompress(filename, GetFileSize((string(filename)+".abiz").c_str()), size, root);
	
	return 0;
}
