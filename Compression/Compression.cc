#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <queue>
#include <vector>
using namespace std;

const int max_size = 257;

struct huffmanNode{
	char id;
	int freq;
	bool isEof;
	string code;
	huffmanNode* left;
	huffmanNode* right;
	huffmanNode* parent;
	huffmanNode(char ch = '\0', string s = "", int num = 0, huffmanNode* lef = NULL, huffmanNode* rig = NULL, huffmanNode* par = NULL, bool eof = false) :
		id(ch), code(s), freq(num), left(lef), right(rig), parent(par), isEof(eof){}
};
typedef huffmanNode* nodePtr;

class Huffman{
private:
	nodePtr nodeArray[max_size];
	ifstream inputFile;
	ofstream outputFile;
	ofstream decompressFile;
	nodePtr child;
	nodePtr parent;
	char id;
	int decodingnum;
	string inputFilename;
	string outputFilename;
	class compare{
	public:
		bool operator()(const nodePtr& c1, const nodePtr& c2)const{
			return (*c1).freq>(*c2).freq;
		}
	};
	priority_queue<nodePtr, vector<nodePtr>, compare> pq;
public:
	nodePtr root;
	Huffman(string s1, string s2);
	void createHuffmanTree();
	void calculateHuffmanCodes(nodePtr & root);
	void compressTofile();
	void decompressTofile();
};

Huffman::Huffman(string s1, string s2){
	inputFilename = s1;
	outputFilename = s2;
	root = NULL;
	parent = NULL;
	child = NULL;
	decodingnum = 0;
	for (int i = 0; i<max_size; i++)
		nodeArray[i] = new huffmanNode();
}

void Huffman::createHuffmanTree(){
	while (pq.size()>1){
		nodePtr h1, h2;
		h1 = pq.top();
		pq.pop();
		h2 = pq.top();
		pq.pop();
		nodePtr h = new huffmanNode();
		h->freq = h1->freq + h2->freq;
		h->left = h1;
		h->right = h2;
		h1->parent = h;
		h2->parent = h;
		pq.push(h);
	}
	root = pq.top();
	root->parent = NULL;
}

void Huffman::calculateHuffmanCodes(nodePtr & root){
	if (root != NULL){
		if (root->left != NULL){
			root->left->code = root->code + '0';
			calculateHuffmanCodes(root->left);
		}
		if (root->right != NULL){
			root->right->code = root->code + '1';
			calculateHuffmanCodes(root->right);
		}
	}
}

void Huffman::compressTofile(){
	int length;
	char ch;
	inputFile.open(inputFilename.c_str(), ios::binary);
	outputFile.open(outputFilename.c_str(), ios::binary);
	if (inputFile)
		inputFile.seekg(0, inputFile.end);
	length = inputFile.tellg();
	inputFile.seekg(0, inputFile.beg);

	int num;
	for (int i = 0; i<length; i++){
		inputFile.get(ch);
		unsigned char tmpch = ch;
		num = int(tmpch);
		if (nodeArray[num]->freq == 0)
			nodeArray[num]->id = ch;
		nodeArray[num]->freq++;
	}
	nodeArray[256]->freq = 1;
	nodeArray[256]->isEof = true;

	for (int i = 0; i<max_size; i++){
		if (nodeArray[i]->freq>0){
			decodingnum++;
			pq.push(nodeArray[i]);
		}
	}
	unsigned char tmpchar = decodingnum - 1;
	outputFile << tmpchar;

	createHuffmanTree();

	calculateHuffmanCodes(root);

	for (int i = 0; i<max_size - 1; i++){
		if (nodeArray[i]->freq > 0){
			outputFile << nodeArray[i]->id << nodeArray[i]->code << ' ';
		}
	}
	outputFile << nodeArray[256]->code << ' ';

	inputFile.seekg(0, inputFile.beg);
	nodePtr tmp = new huffmanNode();
	int bytenum = 0;
	char dout = 0;
	for (int i = 0; i<length; i++){
		inputFile.get(ch);
		unsigned char tmpch = ch;
		int num1 = tmpch;
		tmp = nodeArray[num1];
		for (int j = 0; j<tmp->code.length(); j++){
			dout = dout << 1;
			bytenum++;
			if (tmp->code[j] == '1')
				dout = dout | 1;
			if (bytenum == 8){
				bytenum = 0;
				outputFile.put(dout);
				dout = 0;
			}
		}
	}

	for (int j = 0; j<nodeArray[256]->code.length(); j++){
		dout = dout << 1;
		bytenum++;
		if (nodeArray[256]->code[j] == '1')
			dout = dout | 1;
		if (bytenum == 8){
			bytenum = 0;
			outputFile.put(dout);
			dout = 0;
		}
	}
	if (bytenum != 0){
		dout = dout << (8 - bytenum);
		outputFile.put(dout);
	}

	inputFile.close();
	outputFile.close();
}

void Huffman::decompressTofile(){
	int tmp, num, totalcount;
	char ch;
	string convert[300];
	for (int i = 0; i < 257; i++){
		convert[i].clear();
	}
	outputFile.open(outputFilename.c_str());
	inputFile.open(inputFilename.c_str(), ios::binary);

	inputFile.get(ch);
	unsigned char tch = ch;
	num = int(tch);
	//overflow 
	if (num == 0){
		num = 256;
	}
	inputFile.get(ch);

	int id;
	string index, eof_index;
	while (num > 0){
		tch = ch;
		id = tch;
		index.clear();
		inputFile.get(ch);
		while (ch != ' '){
			index += ch;
			inputFile.get(ch);
		}
		convert[id] = index;
		num--;
		inputFile.get(ch);
	}
	index.clear();
	while (ch != ' '){
		index += ch;
		inputFile.get(ch);
	}
	eof_index = index;

	int currentPos = inputFile.tellg();
	inputFile.seekg(0, inputFile.end);
	int len = inputFile.tellg();

	index.clear();
	inputFile.seekg(currentPos, inputFile.beg);
	for (int i = 0; i < len - currentPos; i++){
		inputFile.get(ch);
		for (int j = 0; j < 8; j++){
			if (ch & 0x80){
				index += '1';
			}
			else{
				index += '0';
			}
			ch = ch << 1;
			if (index == eof_index){
				outputFile.close();
				inputFile.close();
				return;
			}
			for (int k = 0; k < 300; k++){
				if (convert[k] == index){
					outputFile.put(char(k));
					index.clear();
					break;
				}
			}
		}
	}
	//while (!inputFile.eof()){
	//	for (int j = 7; j >= 0; j--){
	//		tmp = (ch >> j) & 1;
	//		if (root->left == NULL&&root->right == NULL){
	//			/*				if(!root->id){
	//			isEOF=true;
	//			break;
	//			}*/
	//			outputFile << root->id;
	//			root = pq.top();
	//			decodingnum--;
	//			if (!totalcount) return;
	//		}
	//		if (tmp == 0) root = root->left;
	//		else {
	//			root = root->right;
	//		}
	//	}
	//	//if(isEOF) break;
	//	inputFile.get(ch);
	//}
	outputFile.close();
	inputFile.close();
}

void useage(string prog) {
	cerr << "Useage: " << endl
		<< "    " << prog << "[-d] input_file output_file" << endl;
	exit(2);
}

void compress(string s1, string s2){
	Huffman h(s1, s2);
	h.compressTofile();
}

void decompress(string s1, string s2){
	Huffman h(s1, s2);
	h.decompressTofile();
}

int main(int argc, char* argv[]) {
	int i;
	string inputFilename, outputFilename;
	bool isDecompress = false;
	for (i = 1; i < argc; i++) {
		if (argv[i] == string("-d")) isDecompress = true;
		else if (inputFilename == "") inputFilename = argv[i];
		else if (outputFilename == "") outputFilename = argv[i];
		else useage(argv[0]);
	}
	if (outputFilename == "") useage(argv[0]);
	if (isDecompress) decompress(inputFilename, outputFilename);
	else compress(inputFilename, outputFilename);
	return 0;
}