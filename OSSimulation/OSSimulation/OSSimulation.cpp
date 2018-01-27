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
#pragma pack ()

class FileSystem {
	using T_OperationFunc = void(FileSystem::*)(initializer_list<string>);
	static const int BLOCK_NUMBER = 100;
	static const int BLOCK_SIZE = 512;
	map <string, T_OperationFunc> map_func;
	map <string, int> map_dataLen;							//file name map to data file length. 
	list<T_BLOCK> m_blocks;
	T_BLOCK m_rootBlock;

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
	}

	~FileSystem() {
	}

	void initialBlocks() {
		m_blocks.resize(BLOCK_NUMBER + 1);
		int i = 0;
		for (list<T_BLOCK>::iterator it = m_blocks.begin(); it != m_blocks.end(); ++it) {
			(*it).num = i;
			++i;
		}
	}

	T_BLOCK& allocateBlock() {
		T_BLOCK block = m_blocks.front();
		if (block.num == 100) {
			cout << "The Memory is exhausted, wait for Memory Compaction... " << endl;
			MemoryCompaction();
			block = m_blocks.front();
			if (block.num == 100) {
				cout << "The Memory is exhausted!!!" << endl;
				return m_blocks.front();
			}
			else
			{
				m_blocks.pop_front();
				m_blocks.push_back(block);
				return m_blocks.back();
			}
		}
		else {
			m_blocks.pop_front();
			m_blocks.push_back(block);
			return m_blocks.back();
		}
	}

	void freeBlock(int num) {

	}

	void MemoryCompaction() {
		cout << "Memory Compaction begin.. " << endl;
		list<T_BLOCK> temp;
		temp.clear();
		cout << "Memory Compaction have done.. " << endl;
	}

	void SetRootDirectory() {
		initialBlocks();
		T_Directory rootBlock;
		T_BLOCK& block = allocateBlock();
		memcpy(&(block.block), &rootBlock, sizeof(T_Directory));
		m_rootBlock = block;
	}

	T_Directory& GetRootBlock() {
		return *((struct Directory*)&m_rootBlock);
	}

	T_Directory& GetDirBlockByNum(int num) {
		for (list<T_BLOCK>::iterator it = m_blocks.begin(); it != m_blocks.end(); ++it) {
			if (it->num == num) {
				return *((struct Directory*)&(*it));
			}		 
		}
		assert(0);
	}

	T_Data& GetDataBlockByNum(int num) {
		for (list<T_BLOCK>::iterator it = m_blocks.begin(); it != m_blocks.end(); ++it) {
			if (it->num == num) {
				return *((struct data*)&(*it));
			}
		}
		assert(0);
	}

	void DeleteDir(int num, const char* name, string wholeName) {
		T_Directory& dir = GetDirBlockByNum(num);
		for (int i = 0; i < 31; ++i) {
			if (dir.subdir[i].type == 'D') {
				wholeName += dir.subdir[i].name;
				DeleteDir(dir.subdir[i].link, dir.subdir[i].name, wholeName);

				dir.subdir[i].type = 'F';
				memset(dir.subdir[i].name, 0, sizeof(dir.subdir[i].name));
				dir.subdir[i].link = -1;
				dir.subdir[i].size = -1;
			}
			else if (dir.subdir[i].type == 'U') {
				T_Data& data = GetDataBlockByNum(dir.subdir[i].link);
				wholeName += dir.subdir[i].name;
				int dataLen = map_dataLen[wholeName];
				int start = dir.subdir[i].size - dataLen;
				memset(data.user_data + start, 0, dataLen);
			}
			else
				return;
		}
	}

	void DeleteDataFile() {

	}

	void SetSubDir(T_Directory& dir, string name, string wholeName) {
		for (int i = 0; i < 31; ++i) {
			if (dir.subdir[i].type == 'D' && strcmp(dir.subdir[i].name, name.c_str()) == 0) {
				DeleteDir(dir.subdir[i].link, name.c_str(), wholeName);
			}
		}

	}

	void SetDirToBlock(char* name) {
		T_BLOCK& block = allocateBlock();


	}

	void SetDataToBlock() {

	}

	void Operation(istringstream& ss) {
		string str1, str2, str3;
		ss >> str1 >> str2 >> str3;
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
		int slash_num = std::count(name.begin(), name.end(), '/');
		int depth = slash_num + 1;
		//if exist, delete it then recreate
		//Delete();
		T_Directory& dir = GetRootBlock();
		while (depth--) {
			string subName = name.substr(0, name.find('/') - 1);
			if (type == "D") {
				SetSubDir(dir, subName, name);
				//SetDirToBlock();
			}
			else if (type == "U") {

			}
			else {
				assert(0);
			}
			name = name.substr(name.find('/') + 1);
			//dir = GetDirBlock();
		}
	
	}

	void Open(initializer_list<string> str) {

	}

	void Close(initializer_list<string> str) {
		for (auto it = str.begin(); it != str.end(); ++it)
			cout << *it << endl;
	}

	void Delete(initializer_list<string> str) {

	}

	void Read(initializer_list<string> str) {

	}

	void Write(initializer_list<string> str) {
		
	}

	void Seek(initializer_list<string> str) {

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


