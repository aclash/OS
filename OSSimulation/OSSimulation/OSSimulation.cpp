#include <cassert>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
using namespace std;
#pragma pack(1)
typedef struct {
	int back;			//block number of previous directory block
	int forward;		//block number of next directory block
	int free;			//block number of first unused block
	char filler[4];		//unused
	struct T_dir {
		char type;		//'F' = free, 'D' = directory, 'U' = user data
		char name[9];   // file name, left-justified, blank filled
		int link;		//block number of first block of file
		short size;		// number of bytes used in the last block of the file
	};
	T_dir dir[31];
}T_Dirctory;

typedef struct {
	int back;
	int forward;
	char user_data[504];
}T_Data;
#pragma pack ()

class FileSystem {
	using T_OperationFunc = void(FileSystem::*)(initializer_list<string>);
	static const int BLOCK_NUMBER = 100;
	static const int BLOCK_SIZE = 512;
	void* data;
	map <string, T_OperationFunc> map_func;
	map<string, int> mm;

public:
	FileSystem() {
		data = (void*)new char[BLOCK_SIZE * BLOCK_NUMBER];
		map_func.insert(std::pair<string, T_OperationFunc>("CREATE", &FileSystem::Create));
		map_func.insert(std::pair<string, T_OperationFunc>("OPEN", &FileSystem::Open));
		map_func.insert(std::pair<string, T_OperationFunc>("CLOSE", &FileSystem::Close));
		map_func.insert(std::pair<string, T_OperationFunc>("DELETE", &FileSystem::Delete));
		map_func.insert(std::pair<string, T_OperationFunc>("READ", &FileSystem::Read));
		map_func.insert(std::pair<string, T_OperationFunc>("WRITE", &FileSystem::Write));
		map_func.insert(std::pair<string, T_OperationFunc>("SEEK", &FileSystem::Seek));
	}

	~FileSystem() {
		delete [] data;
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
		for (auto it = str.begin(); it != str.end(); ++it)
			cout << *it << endl;

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


