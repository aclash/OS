#include <algorithm>
#include <cassert>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#pragma warning(disable:4996)  
using namespace std;
#pragma pack(1)
typedef struct Directory{
	int back;			//block number of previous directory block
	int forward;		//block number of next directory block
	int free;			//block number of first unused block
	char filler[4];		//unused
	struct T_dir {
		char type;		//'F' = free, 'D' = directory, 'U' = user data
		char name[9];   // file name, left-justified, blank filled
		int link;		//block number of first block of file
		short size;		// number of bytes used in the last block of the file
		T_dir() {
			type = 'F';
			memset(name, 0, 9);
			link = -1;
			size = -1;
		}
	};
	T_dir subdir[31];
	Directory() {
		back = -1;
		forward = -1;
		free = -1;
		memset(filler, 0, 4);
		
	}
}T_Directory;

typedef struct data{
	int back;
	int forward;
	char user_data[504];
	data() {
		back = -1;
		forward = -1;
		memset(user_data, 0, 504);
	}
}T_Data;

typedef struct BLOCK{
	int num;
	char block[512];
	BLOCK() {
		memset(block, 0, 512);
	}
}T_BLOCK;

typedef struct dataFile {
	int num;
	int end;
	int bytesNum;
	char type;
	int parent;
}T_DATA_FILE;
#pragma pack ()

class FileSystem {
	using T_OperationFunc = void(FileSystem::*)(initializer_list<string>);
	static const int BLOCK_NUMBER = 10;
	static const int BLOCK_SIZE = 512;
	static const int BLOCK_DATA_SIZE = 504;
	map <string, T_OperationFunc> map_func;
	map <string, T_DATA_FILE> map_dataFile;							//file name map to data file length. 
	list<T_BLOCK> m_blocks;
	int m_currentDataBlkNum;
	string m_currentFileName;
	int m_currentOffset;
	int m_freeBlkNum;
	enum STATE {
		NO_PERMITTED,
		INPUT,
		OUTPUT,
		UPDATE
	};
	STATE m_currentState;

public:
	FileSystem() {
		map_func.insert(std::pair<string, T_OperationFunc>("CREATE", &FileSystem::Create));
		map_func.insert(std::pair<string, T_OperationFunc>("OPEN", &FileSystem::Open));
		map_func.insert(std::pair<string, T_OperationFunc>("CLOSE", &FileSystem::Close));
		map_func.insert(std::pair<string, T_OperationFunc>("DELETE", &FileSystem::Delete));
		map_func.insert(std::pair<string, T_OperationFunc>("READ", &FileSystem::Read));
		map_func.insert(std::pair<string, T_OperationFunc>("WRITE", &FileSystem::Write));
		map_func.insert(std::pair<string, T_OperationFunc>("SEEK", &FileSystem::Seek));
		initialBlocks();
		SetRootDirectory();
		m_currentDataBlkNum = -1;
		m_freeBlkNum = BLOCK_NUMBER;
		m_currentState = NO_PERMITTED;
		m_currentOffset = -1;
	}

	~FileSystem() {
	}

	void SetState() {

	}

	void initialBlocks() {
		m_blocks.resize(BLOCK_NUMBER + 1);
		int i = 0;
		for (list<T_BLOCK>::iterator it = m_blocks.begin(); it != m_blocks.end(); ++it) {
			(*it).num = i;
			++i;
		}
	}

	bool allocateSuccess(const T_BLOCK& block) {
		if (block.num == BLOCK_NUMBER)
			return false;
		else
			return true;
	}

	T_BLOCK& allocateBlock() {
		T_BLOCK block = m_blocks.front();
		if (block.num == BLOCK_NUMBER) {
			cout << "The Memory is exhausted, wait for Memory Compaction... " << endl;
			//MemoryCompaction();
			block = m_blocks.front();
			if (block.num == BLOCK_NUMBER) {
				cout << "The Memory is exhausted!!!" << endl;
				return m_blocks.front();
			}
			else
			{
				m_blocks.pop_front();
				m_blocks.push_back(block);
				m_currentDataBlkNum = m_blocks.back().num;
				return m_blocks.back();
			}
		}
		else {
			m_blocks.pop_front();
			m_blocks.push_back(block);
			--m_freeBlkNum;
			m_currentDataBlkNum = m_blocks.back().num;
			return m_blocks.back();
		}
	}

	void freeBlock(int num) {
		for (list<T_BLOCK>::iterator it = m_blocks.begin(); it != m_blocks.end();) {
			if (it->num == num) {
				T_BLOCK block;
				block.num = it->num;
				it = m_blocks.erase(it);
				m_blocks.push_front(block);
				++m_freeBlkNum;
				return;
			}
			else
				++it;
		}
	}

	void MemoryCompaction() {
		cout << "Memory Compaction begin.. " << endl;
		list<T_BLOCK> temp;
		temp.clear();
		//TO DO
		cout << "Memory Compaction have done.. " << endl;
	}

	void SetRootDirectory() {
		initialBlocks();
		T_Directory rootBlock;
		T_BLOCK& block = allocateBlock();
		memcpy(&(block.block), &rootBlock, sizeof(T_Directory));
	}

	T_Directory& GetRootBlock() {
		for (list<T_BLOCK>::iterator it = m_blocks.begin(); it != m_blocks.end(); ++it) {
			if (it->num == 0) {
				return *((struct Directory*)&(*it).block);
			}
		}
		assert(0);
	}

	T_BLOCK& GetBlockByNum(int num) {
		for (list<T_BLOCK>::iterator it = m_blocks.begin(); it != m_blocks.end(); ++it) {
			if (it->num == num) {
				return *it;
			}
		}
		assert(0);
	}

	T_Directory& GetDirBlockByNum(int num) {
		for (list<T_BLOCK>::iterator it = m_blocks.begin(); it != m_blocks.end(); ++it) {
			if (it->num == num) {
				return *((struct Directory*)&(*it).block);
			}		 
		}
		assert(0);
	}

	T_Data& GetDataBlockByNum(int num) {
		for (list<T_BLOCK>::iterator it = m_blocks.begin(); it != m_blocks.end(); ++it) {
			if (it->num == num) {
				return *((struct data*)&(*it).block);
			}
		}
		assert(0);
	}

	void DeleteDir(int num, string wholeName) {
		T_Directory& dir = GetDirBlockByNum(num);
		for (int i = 0; i < 31; ++i) {
			if (dir.subdir[i].type == 'D') {
				wholeName += dir.subdir[i].name;
				DeleteDir(dir.subdir[i].link, wholeName);
				dir.subdir[i].type = 'F';
				memset(dir.subdir[i].name, 0, sizeof(dir.subdir[i].name));
				dir.subdir[i].link = -1;
				dir.subdir[i].size = -1;
			}
			else if (dir.subdir[i].type == 'U') {
				T_Data& data = GetDataBlockByNum(dir.subdir[i].link);
				wholeName += dir.subdir[i].name;
				int dataLen = map_dataFile[wholeName].bytesNum;
				int start = dir.subdir[i].size - dataLen;
				memset(data.user_data + start, 0, dataLen);
			}
			else {
				freeBlock(num);
				return;
			}	
		}
	}
	
	void DeleteData(int num) {
		int oldNum = num;
		do {
			T_Data& data = GetDataBlockByNum(oldNum);
			int blkNum = data.forward;
			freeBlock(oldNum);
			oldNum = blkNum;
		} while (oldNum != -1);
	}

	int SetDataByRecreate(T_Directory& dir, string name, string wholeName, int dirNum) {
		for (int i = 0; i < 31; ++i) {
			if (dir.subdir[i].type == 'U' && strcmp(dir.subdir[i].name, name.c_str()) == 0) {
				DeleteData(dir.subdir[i].link);
				dir.subdir[i].type = 'F';
				memset(dir.subdir[i].name, 0, sizeof(dir.subdir[i].name));
				dir.subdir[i].link = -1;
				dir.subdir[i].size = -1;
				int num = SetDataByRecreate(dir, name, wholeName, dirNum);
				return num;
			}
			else if (dir.subdir[i].type == 'F') {
				dir.subdir[i].type = 'U';
				memcpy(dir.subdir[i].name, name.c_str(), sizeof(dir.subdir[i].name));
				T_BLOCK& block = allocateBlock();
				if (!allocateSuccess(block))
					return -1;
				int num = block.num;
				dir.subdir[i].link = num;
				T_BLOCK& blockDir = GetBlockByNum(dirNum);
				memcpy(&(blockDir.block), &dir, sizeof(T_Directory));
				T_Data tempData;
				memcpy(&(block.block), &tempData, sizeof(T_Data));
				//m_currentDataBlkNum = num;
				m_currentFileName = wholeName;
				T_DATA_FILE df;
				df.num = num;
				df.end = 0;
				df.bytesNum = 0;
				df.type = 'U';
				df.parent = dirNum;
				map_dataFile[wholeName] = df;
				m_currentState = STATE::OUTPUT;
				m_currentOffset = map_dataFile[m_currentFileName].bytesNum;
				return num;
			}
		}
		assert(0);
	}

	int SetSubDirByRecreate(T_Directory& dir, string name, string wholeName, int dirNum) {
		for (int i = 0; i < 31; ++i) {
			if (dir.subdir[i].type == 'D' && strcmp(dir.subdir[i].name, name.c_str()) == 0) {
				DeleteDir(dir.subdir[i].link, wholeName);
				dir.subdir[i].type = 'F';
				memset(dir.subdir[i].name, 0, sizeof(dir.subdir[i].name));
				dir.subdir[i].link = -1;
				dir.subdir[i].size = -1;
				int num = SetSubDirByRecreate(dir, name, wholeName, dirNum);
				return num;
			}
			else if (dir.subdir[i].type == 'F') {
				dir.subdir[i].type = 'D';
				memcpy(dir.subdir[i].name, name.c_str(), sizeof(dir.subdir[i].name));
				T_BLOCK& block = allocateBlock();
				if (!allocateSuccess(block))
					return -1;
				int num = block.num;
				dir.subdir[i].link = num;
				T_BLOCK& blockDir = GetBlockByNum(dirNum);
				memcpy(&(blockDir.block), &dir, sizeof(T_Directory));
				T_Directory tempDir;
				memcpy(&(block.block), &tempDir, sizeof(T_Directory));
				T_DATA_FILE df;
				df.num = num;
				df.end = 0;
				df.bytesNum = 0;
				df.type = 'D';
				df.parent = dirNum;
				map_dataFile[wholeName] = df;
				return num;
			}
			else {
				assert(0);
			}
		}
		assert(0);
	}

	int SetSubDir(T_Directory& dir, string name, int dirNum) {
		for (int i = 0; i < 31; ++i) {
			if (dir.subdir[i].type == 'D' && strcmp(dir.subdir[i].name, name.c_str()) == 0) {
				return dir.subdir[i].link;
			}
			else if (dir.subdir[i].type == 'F') {
				dir.subdir[i].type = 'D';
				memcpy(dir.subdir[i].name, name.c_str(), sizeof(dir.subdir[i].name));
				T_BLOCK& block = allocateBlock();
				if (!allocateSuccess(block))
					return -1;
				int num = block.num;
				dir.subdir[i].link = num;
				T_BLOCK& blockDir = GetBlockByNum(dirNum);
				memcpy(&(blockDir.block), &dir, sizeof(T_Directory));
				T_Directory tempDir;
				memcpy(&(block.block), &tempDir, sizeof(T_Directory));
				return num;
			}
			else {
				assert(0);
			}
		}
		assert(0);
	}

	void Operation(istringstream& ss) {
		string str1, str2, str3;
		ss >> str1 >> str2;
		getline(ss, str3);
		str3.erase(0, 1);   //delete space
		transform(str1.begin(), str1.end(), str1.begin(), toupper);
		transform(str2.begin(), str2.end(), str2.begin(), toupper);
		T_OperationFunc pFun = map_func[str1];
		if (str2.empty())
			(this->*pFun)({});
		else if (str3.empty())
			(this->*pFun)({ str2 });
		else
			(this->*pFun)({ str2, str3 });
	}

	void Create(initializer_list<string> str) {
		assert(str.size() == 2);
		string type = *(str.begin());
		string name = *(str.begin() + 1);
		string subName = name;
		int slash_num = std::count(name.begin(), name.end(), '/');
		int depth = slash_num + 1;
		T_Directory dir = GetRootBlock();
		int num = 0;
		while (depth--) {
			string tempSubName = subName.substr(0, subName.find('/'));
			if (depth == 0) {
				if (type == "D") {
					SetSubDirByRecreate(dir, tempSubName, name, num);
				}
				else if (type == "U") {
					SetDataByRecreate(dir, tempSubName, name, num);
				}
				else {
					assert(0);
				}
			}
			else {
				num = SetSubDir(dir, tempSubName, num);
				subName = subName.substr(subName.find('/') + 1);
				dir = GetDirBlockByNum(num);
			}
		}
	}

	void Open(initializer_list<string> str) {
		assert(str.size() == 2);
		string mode = *(str.begin());
		string name = *(str.begin() + 1);
		m_currentFileName = name;
		if (map_dataFile.find(m_currentFileName) == map_dataFile.end())
		{
			cout << "**************WARNING************* :" << endl;
			cout << "This file's not existed!" << endl;
			cout << "********************************** :" << endl;
			return;
		}
		else {
			if (mode == "I"){
				m_currentState = STATE::INPUT;
				m_currentOffset = 0;
			}
			else if (mode == "O") {
				m_currentState = STATE::OUTPUT;
				m_currentOffset = map_dataFile[m_currentFileName].bytesNum;
			}
			else {
				m_currentState = STATE::UPDATE;
				m_currentOffset = 0;
			}
		}
	}

	void Close(initializer_list<string> str) {
		m_currentState = STATE::NO_PERMITTED;
	}

	void Delete(initializer_list<string> str) {
		assert(str.size() == 1);
		string name = *(str.begin());
		if (map_dataFile.find(name) != map_dataFile.end()) {
			if (map_dataFile[name].type == 'U') {
				DeleteData(map_dataFile[name].num);
				T_Directory& dir = GetDirBlockByNum(map_dataFile[name].parent);
				for (int i = 0; i < 31; ++i) {
					if (dir.subdir[i].type == 'U' && strcmp(dir.subdir[i].name, name.c_str()) == 0) {
						dir.subdir[i].type = 'F';
						memset(dir.subdir[i].name, 0, sizeof(dir.subdir[i].name));
						dir.subdir[i].link = -1;
						dir.subdir[i].size = -1;
					}
				}
			}
			else if (map_dataFile[name].type == 'D') {
				DeleteDir(map_dataFile[name].parent, name);
			}
		}
		else { 
			cout << "**************WARNING************* :" << endl;
			cout << "No such file!!" << endl;
			cout << "********************************** :" << endl;
		}
	}

	void Read(initializer_list<string> str) {
		if (m_currentState != STATE::INPUT || m_currentState != STATE::UPDATE) {
			cout << "**************WARNING************* :" << endl;
			cout << "Only INPUT or UPDATE mode can read!" << endl;
			cout << "********************************** :" << endl;
			return;
		}
		assert(str.size() == 1);
		string len = *(str.begin());
		int bytesNum = stoi(len);
		int currentDataNum = map_dataFile[m_currentFileName].num;
		int fileBytesNum = map_dataFile[m_currentFileName].bytesNum;
		T_Data& block = GetDataBlockByNum(currentDataNum);
		string outPut(&block.user_data[0], &block.user_data[0] + min(fileBytesNum, bytesNum));
		cout << outPut << endl;
		if (fileBytesNum < bytesNum)
			cout << "*******N is less than file length, the end of file is reached.**********" << endl;
	}

	void Write(initializer_list<string> str) {
		if (m_currentState != STATE::OUTPUT || m_currentState != STATE::UPDATE) {
			cout << "**************WARNING************* :" << endl;
			cout << "Only OUTPUT or UPDATE mode can write!" << endl;
			cout << "********************************** :" << endl;
			return;
		}
		assert(str.size() == 2);
		string len = *(str.begin());
		string text = *(str.begin() + 1);
		int bytesNum = stoi(len);
		int originBytesNum = bytesNum;
		int needBlkNum = -1;
		if (bytesNum % BLOCK_DATA_SIZE == 0)
			needBlkNum = bytesNum / BLOCK_DATA_SIZE;
		else 
			needBlkNum = bytesNum / BLOCK_DATA_SIZE + 1;
		if (needBlkNum > m_freeBlkNum) {
			//MemoryCompaction();
			if (needBlkNum > m_freeBlkNum) {
				cout << "**************WARNING************* :" << endl;
				cout << "No Sufficient Space!!!" << endl;
				cout << "********************************** :" << endl;
				return;
			}
		}
		else {
			needBlkNum = bytesNum / BLOCK_DATA_SIZE;
			int currentDataNum = map_dataFile[m_currentFileName].num;
			int fileBytesNum = map_dataFile[m_currentFileName].bytesNum;
			T_Data& block = GetDataBlockByNum(currentDataNum);
			int dataBlkNum = currentDataNum;
			text.erase(text.size() - 1, string::npos);
			text.erase(0, 1);

			if (fileBytesNum == 0) {
				while (needBlkNum--) {
					memcpy(block.user_data, text.c_str(), sizeof(block.user_data));
					text = text.substr(std::min((int)text.size(), BLOCK_DATA_SIZE));
					bytesNum -= BLOCK_DATA_SIZE;
					T_BLOCK& dataBlk = allocateBlock();
					if (!allocateSuccess(dataBlk))
						return;
					T_Data& tempBlk = GetDataBlockByNum(dataBlk.num);
					tempBlk.back = dataBlkNum;
					dataBlkNum = dataBlk.num;
					block.forward = dataBlk.num;
					block = tempBlk;
				}
				memcpy(block.user_data, text.c_str(), bytesNum);
				map_dataFile[m_currentFileName].bytesNum = originBytesNum;
			}
			else {
				if (m_currentOffset + originBytesNum > fileBytesNum) {
					//in data file part
					memcpy(block.user_data + m_currentOffset, text.c_str(), fileBytesNum - m_currentOffset);
					//out of data file part
					bytesNum = (bytesNum - fileBytesNum + m_currentOffset);

					while (needBlkNum--) {
						memcpy(block.user_data, text.c_str(), sizeof(block.user_data));
						text = text.substr(std::min((int)text.size(), BLOCK_DATA_SIZE));
						bytesNum -= BLOCK_DATA_SIZE;
						T_BLOCK& dataBlk = allocateBlock();
						if (!allocateSuccess(dataBlk))
							return;
						T_Data& tempBlk = GetDataBlockByNum(dataBlk.num);
						tempBlk.back = dataBlkNum;
						dataBlkNum = dataBlk.num;
						block.forward = dataBlk.num;
						block = tempBlk;
					}
					memcpy(block.user_data, text.c_str(), bytesNum);
					map_dataFile[m_currentFileName].bytesNum = m_currentOffset + originBytesNum;

				}
				else {
					memcpy(block.user_data + m_currentOffset, text.c_str(), originBytesNum);
					//map_dataFile[m_currentFileName].bytesNum = bytesNum;
				}
			}
		}
	}

	void Seek(initializer_list<string> str) {
		if (m_currentState != STATE::INPUT || m_currentState != STATE::UPDATE) {
			cout << "**************WARNING************* :" << endl;
			cout << "Only INPUT or UPDATE mode can seek!" << endl;
			cout << "********************************** :" << endl;
			return;
		}
		assert(str.size() == 2);
		string strbase = *(str.begin());
		string stroffset = *(str.begin() + 1);
		int base = stoi(strbase);
		int offset = stoi(stroffset);
		switch (base) {
			case -1:
				if (offset < 0){
					cout << "**************WARNING************* :" << endl;
					cout << "In base -1, Offset cannot be negative!" << endl;
					cout << "********************************** :" << endl;
				}
				m_currentOffset = max(offset, 0);
			case 0:
				if (offset < 0) {
					cout << "**************WARNING************* :" << endl;
					cout << "In base 0, Position cannot be negative!" << endl;
					cout << "********************************** :" << endl;
				}
				m_currentOffset = max(m_currentOffset + offset, 0);
			case +1:
				if (offset > 0) {
					cout << "**************WARNING************* :" << endl;
					cout << "In base 1, Position cannot be positive!" << endl;
					cout << "********************************** :" << endl;
				}
				m_currentOffset = map_dataFile[m_currentFileName].bytesNum + min(offset, 0);
			default: 
				return;
		}
	}

};

int main()
{
	FileSystem fs;
	string strline;
	while (getline(cin, strline)) {
		istringstream ss(strline);
		fs.Operation(ss);
	}
	system("pause");
    return 0;
}

/*
void func(initializer_list<string> op)
{
	assert(op.size() >= 1);
	string str("");
	for(auto beg = op.begin(); beg != op.end(); ++beg)
	str += *beg;
	return str;
}

template <typename First, typename... Rest>
void print(const First& first, const Rest&... rest) {
	cout << first << ", ";
	print(rest...); // recursive call using pack expansion syntax
}

template <typename T>
using alias_map = std::map < std::string, T > ;
alias_map<int>  map_t;
alias_map<std::string> map_str;

int main()
{
	cout << func({"CREATE","type","name"}) << endl;
	cout << func({"CLOSE"}) << endl;
	cout << func({"READ", "n"}) << endl;

	print(); 
	print(1); 
	print(10, 20);
	print(100, 200, 300);
	print("first", 2, "third", 3.14159);

	return 0;
}
*/


