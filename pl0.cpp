#include<ctime>
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
using namespace std;

#define LIT 0 
#define OPR 1
#define LOD 2
#define STO 3
#define CAL 4
#define INT 5
#define JMP 6
#define JPC 7
#define RED 8
#define WRT 9

#define constant 0
#define variable 1
#define procedure 2

fstream intxt;   //词法分析读入文本
fstream outtxt;  //词法分词 单词输出文本
fstream readLexi; 
fstream writeGram;

//单词单元
class Unit
{
	public:
		string value;	//值
		string key;		//类型
		int line;
		int column;
		void print();
};

string order[] = { "LIT","OPR","LOD","STO","CAL","INT","JMP","JPC","RED","WRT" };
const string key[] = { "program","const","var","procedure","begin","end","if","then","while","do","call","read","write","odd" };
int line;   //读入单词所在行
int column;   //读入单词所在列
Unit  unit;   //每个单词的单元

int tx = 0;      //记录当前符号表的位置，也即当前符号表里有多少个符号  符号表地址从1开始
int dx = 0;      //
int cx = 0;     //记录即将生成的目标机指令的地址，以便添加生成的指令（从0开始） 
int lev = 0;     //记录过程层数信息 
int mm;
int isOutSym;
int isOutPCode;
int T;      //栈顶指示器寄存器T top 
int B;     //栈基址寄存器  存放当前运行过程的数据区在STACK中的起始地址 sp 
int P;     //下条指令（全局变量），存放下一条要执行的指令地址 
int I;    //指令寄存器，存放当前要执行的代码 
int dataStack[1000]; //数据栈

//符号表
struct SymTable
{
	string name; //符号名 
	int kind;    //符号类型：0常量、1变量、2进程 
	int value;   //值 
	int level;
	int adr;
	int size;         //记录过程的参数个数 
	int num;    //变量在程序出现的次数，方便内存优化
}SymTable[1000];

//中间代码
struct Pcode {
	int f;
	int l;
	int a;
}Pcode[1000];

//词法分析部分
void LexicalAnalysis();                            //词法分析函数 
bool isBC(char cc);                   //词法分析判断是否需要跳读
bool isDigit(char cc);                //判断是否是数字
bool isLetter(char cc);               //判断是否是字母
string concat(char cc, string loken); //将读入字符连接到单词
int reserve(string loken);            //判断所读单词是否是标识符
void Retract();                       //读入字符后退一个指针

//语法分析部分
void GrammarAnalysis();
void ReadLine();
void ThrowError(string error, int ismissing);
void prog(); 
void block();
void LexConst();
void ConstIntoSymTable();
void LexVariable();
void LexProcedure();
void body();
void statement();
void lexp();
void exp();
void term();
void factor();
void AddVariableToSymTable(string name, int level, int adr);
void AddConstToSymTable(string name, int level, int val);
void AddprocedureToSymTable(string name, int level, int adr);
int FindSymPosition(string name);
void GeneratePCode(int f, int l, int a);
int QueryTheLastProcessIncludeBlock();
int StringToInt(string name);
bool IsExistInTheSameLevelTable(string name, int lev);
bool IsExistInThePreviousLevelTable(string name, int lev);
void OutputPCode();
void OutputSymTable();
void error(int n);
int FindSymPosition(string name);
int StringToInt(string name);
bool IsExistInTheSameLevelTable(string name, int lev);
bool IsExistInThePreviousLevelTable(string name, int lev);

//解释器部分
int getBase(int nowBp, int lev);
void interpreter();
int main()
{
	LexicalAnalysis();
	GrammarAnalysis();
	cout<<"结束运行"<<endl;
	return  0;
}

void Unit::print()
{
	cout << "Position: [" << line << "," << column << "]" << endl;
}

//词法分析部分
bool isBC(char cc)                       //判断读入是否需要跳跃ok
{
	if (cc == '\n' || cc == '\t' || cc == ' ' || cc == '\r')
	{
		switch (cc)
		{
		case ' ':
			column++;
			break;
		case '\n':
			line++;
			column = 0;
			break;
		case '\t':
			column += 4;
			break;
		case '\r':
			column += 1;
		default:
			break;
		}
		return 1;
	}
	return 0;
}

bool isDigit(char cc)                    //判断是否是字母ok
{
	if (cc >= '0' && cc <= '9')
		return 1;
	return 0;
}

bool isLetter(char cc)                   //判断是否是字母ok
{
	if ((cc >= 'a' && cc <= 'z') || (cc >= 'A' && cc <= 'Z'))
		return 1;
	return 0;
}

string concat(char cc, string loken)     //将读入字符连接到单词ok
{
	return loken + cc;
}

int reserve(string loken)                //判断是否是标识符ok
{
	int i;
	for (int i = 0; i < 14; i++)
	{
		if (loken == key[i])
			return 1;
	}
	return 0;
}

void Retract()                           //读入单词后退一个指针ok 
{
	if (!intxt.eof())
		intxt.seekg(-1, ios::cur);
}

// 函数名:LexicalAnalysis
// 功能:词法分析主函数
void LexicalAnalysis()    //词法分析 Lexical analysis
{
	char ch;
	string strToken;
	string nm;
	cout << "输入文件名:";
	cin >> nm;
	intxt.open((nm).c_str(), ios::in);
	outtxt.open("write.txt", ios::out | ios::trunc);
	while ((ch = intxt.get()) != -1)
	{
		if (!line)
			line++;
		if (ch == '/')	//先进行预处理，剔除无意义的编辑用字符
		{

			ch = intxt.get();
			if (ch == '*')
			{
				while (ch != '/')
				{
					ch = intxt.get();
					strToken = "";
				}

			}
			else
			{
				while (ch != ' ')
				{
					ch = intxt.get();
					strToken = "";
				}

			}
			if (isBC(ch))
				strToken = "";
			//Retract();
		}
		else
		{
			if (isBC(ch))
				strToken = "";
			else if (isLetter(ch))	//处理字母
			{
				while (isLetter(ch) || isDigit(ch))
				{
					strToken = concat(ch, strToken);
					column++;
					ch = intxt.get();
				}
				if (reserve(strToken))	//判断关键字
				{
					cout << strToken << " " << "keyword" << " " << line << " " << column << " " << endl;
					outtxt << strToken << " " << "keyword" << " " << line << " " << column << " " << endl;
				}
				else
				{
					cout << strToken << " " << "ID" << " " << line << " " << column << " " << endl;
					outtxt << strToken << " " << "ID" << " " << line << " " << column << " " << endl;
				}
				Retract();
				strToken = "";
			}
			else if (isDigit(ch))	//处理常数
			{
				while (isDigit(ch))
				{
					strToken = concat(ch, strToken);
					column++;
					ch = intxt.get();
				}

				if (isLetter(ch))  //  char behaind int is wrong
				{
					cout << "[Lexical error]" << " " << line << " " << column << " " << endl;
					outtxt << "[Lexical error]" << " " << line << " " << column << " " << endl;

					while (isLetter(ch) || isDigit(ch))  //继续读ID
					{
						strToken = concat(ch, strToken);
						column++;
						ch = intxt.get();
					}
					cout << strToken << " " << " Invalid ID" << " " << line << " " << column << " " << endl;
					outtxt << strToken << " " << " Invalid ID" << " " << line << " " << column << " " << endl;

					cout << "[Lexical error]" << " " << line << " " << column << " " << endl;
					outtxt << "[Lexical error]" << " " << line << " " << column << " " << endl;

				}
				else
				{
					cout << strToken << " " << " INT" << " " << line << " " << column << " " << endl;
					outtxt << strToken << " " << " INT" << " " << line << " " << column << " " << endl;
				}
				Retract();
				strToken = "";
			}
			else switch (ch)	//处理算符和界符
			{
			case '=':
				column++;
				cout << ch << " " << "lop" << " " << line << " " << column << " " << endl;
				outtxt << ch << " " << "lop" << " " << line << " " << column << " " << endl;
				break;
			case ':':
				column++;
				strToken = concat(ch, strToken);
				ch = intxt.get();

				strToken = concat(ch, strToken);
				if (ch == '=')
				{
					column++;
					cout << strToken << " " << "aop" << " " << line << " " << column << " " << endl;
					outtxt << strToken << " " << "aop" << " " << line << " " << column << " " << endl;

				}
				else {
					cout << "Lexical error" << " " << "：=AOP" << line << " " << column << " " << "/.missing =./" << endl;
					outtxt << "Lexical error" << " " << "：=AOP" << line << " " << column << " " << "/.missing =./" << endl;
					Retract();
				}
				strToken = "";
				break;
			case '>':
				column++;
				ch = intxt.get();
				if (ch == '=')
				{
					column++;
					cout << ">=" << " " << "lop" << " " << line << " " << column << " " << endl;
					outtxt << ">=" << " " << "lop" << " " << line << " " << column << " " << endl;
				}
				else {
					cout << ">" << " " << "lop" << " " << line << " " << column << " " << endl;

					outtxt << ">" << " " << "lop" << " " << line << " " << column << " " << endl;
					Retract();
				}
				break;
			case '<':
				column++;
				ch = intxt.get();
				if (ch == '>')
				{
					column++;
					cout << "<>" << " " << "lop" << " " << line << " " << column << " " << endl;
					outtxt << "<>" << " " << "lop" << " " << line << " " << column << " " << endl;

				}
				else if (ch == '=')
				{
					column++;
					cout << "<=" << " " << "lop" << " " << line << " " << column << " " << endl;
					outtxt << "<=" << " " << "lop" << " " << line << " " << column << " " << endl;
				}
				else {
					cout << "<" << " " << "cop" << " " << line << " " << column << " " << endl;
					outtxt << "<" << " " << "cop" << " " << line << " " << column << " " << endl;
					Retract();
				}
				break;
			case '+':
				column++;
				cout << ch << " " << "aop" << " " << line << " " << column << " " << endl;
				outtxt << ch << " " << "aop" << " " << line << " " << column << " " << endl;
				break;

			case '-':
				column++;
				cout << ch << " " << "aop" << " " << line << " " << column << " " << endl;
				outtxt << ch << " " << "aop" << " " << line << " " << column << " " << endl;
				break;

			case '*':
				column++;
				cout << ch << " " << "mop" << " " << line << " " << column << " " << endl;
				outtxt << ch << " " << "mop" << " " << line << " " << column << " " << endl;
				break;
			case '/':
				column++;
				cout << ch << " " << "mop" << " " << line << " " << column << " " << endl;
				outtxt << ch << " " << "mop" << " " << line << " " << column << " " << endl;
				break;
			case ',':
				column++;
				cout << ch << " " << "sop" << " " << line << " " << column << " " << endl;
				outtxt << ch << " " << "sop" << " " << line << " " << column << " " << endl;
				break;
			case ';':
				column++;
				outtxt << ch << " " << "eop" << " " << line << " " << column << " " << endl;
				cout << ch << " " << "eop" << " " << line << " " << column << " " << endl;
				break;
			case '(':
				column++;
				cout << ch << " " << "sop" << " " << line << " " << column << " " << endl;
				outtxt << ch << " " << "sop" << " " << line << " " << column << " " << endl;
				break;
			case ')':
				column++;
				cout << ch << " " << "sop" << " " << line << " " << column << " " << endl;
				outtxt << ch << " " << "sop" << " " << line << " " << column << " " << endl;
				break;

			default:
				column++;
				cout << ch << " " << "UNKNOW" << " " << line << " " << column << " " << endl;
				outtxt << ch << " " << "UNKNOW" << " " << line << " " << column << " " << endl;
				break;
			}
		}
	}
	intxt.close();
	outtxt.close();
	cout << "词法分析结束" << endl; 
}

//语法分析部分

// 函数名:GrammarAnalysis
// 功能:语法分析主函数
void GrammarAnalysis()
{
	readLexi.open("write.txt", ios::in);
	ReadLine();
	prog();
	int count = 0; //记录未被使用的变量个数
	int i = 1;
	while (SymTable[i].num && !SymTable[i].level) { //遍历符号表第0层的主函数符号
		if (SymTable[i].num==1 && SymTable[i].kind==variable) //优化仅声明却不使用的变量
		{
			int j = i + 1;                                      
			while (SymTable[j].num && !SymTable[j].level)    //变量的相对地址减一
			{
				SymTable[j].adr--;                    
				j++;
			}
			count++;    //未使用变量个数加一
		}
		i++;
	}
	Pcode[mm].a -= count;     //回填INT指令开辟的空间
	string line;
	if (!readLexi.eof())   //分析结束,但文件未完
	{
		ThrowError(unit.value, 0);
		exit(0);
	}
	while (!readLexi.eof())    //文件未完全读取
	{
		getline(readLexi, line);
		if (readLexi.eof())
		{
			cout << "是否输出Pcode:1 or 0" << endl;
			cin >> isOutPCode;
			if (isOutPCode)
				OutputPCode();
			cout << "是否输出符号表:1 or 0" << endl;
			cin >> isOutSym;
			if (isOutSym)
			{
				OutputSymTable();
				cout<<endl;
			}
			interpreter();

			exit(0);
		}
		istringstream iss(line);
		iss >> unit.value;
		iss >> unit.key;
		iss >> unit.line;
		iss >> unit.column;
		ThrowError(unit.value, 0);
	}
	readLexi.close();
	cout << "是否输出Pcode:1 or 0" << endl;
	cin >> isOutPCode;
	if (isOutPCode)
		OutputPCode();
	cout << "是否输出符号表:1 or 0" << endl;
	cin >> isOutSym;
	if (isOutSym)
	{
		OutputSymTable();
		cout << endl;
	}
	interpreter();
}


// 函数名:FindSymPosition
// 功能:从后往前遍历符号表，根据符号名输出该符号在符号表的位置
// 注解参数:name是符号名
int FindSymPosition(string name)
{
	//倒序遍历符号表，先找同层或前面定义过的变量 
	for(int i=tx;i>=1;i--)
	{
		if(SymTable[i].name==name&&SymTable[i].level<=lev)
		{
			return i;
		}
	}
	return -1;
}

// 函数名:error
// 功能:输出语义分析错误处理
// 注解参数:n是错误序号
void error(int n)
{
	switch (n)
	{
		case  -1:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << "Missing var/const" << endl; break;
		case  0:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << "Missing  ;" << endl; break;
		case  1:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << "Identifier illegal" << endl; break;
		case  2:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << "Missing  program" << endl; break;
		case  3:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << "Missing  :=" << endl; break;
		case  4:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << "Missing  (" << endl; break;
		case  6:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << "Missing  Begin" << endl; break;
		case  7:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << "Missing  End" << endl; break;
		case  8:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << "Missing  Then" << endl; break;
		case  9:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << "Missing do" << endl; break;
		case 10:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << " " << ":" << "Not exist " << unit.value << endl; break;
		case 11:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << " " << unit.value << " is not a PROCEDURE" << endl; break;
		case 12:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << " " << unit.value << " is not a VARIABLE" << endl; break;
		case 13:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << " " << unit.value << " is not a CONSTANT" << endl; break;
		case 14:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << " " << "Not exist " << unit.value << endl; break;
		case 15:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << " " << "Duplicate definition  " << unit.value << endl; break;
		case 16:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << " " << "The number of parameters does not match" << endl; break;
		case 17:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << " " << "ILLEGAL" << endl; break;
		case 18:cout << "[ERROR]" << "index " << unit.line << " " << unit.column << " " << "Missing Var" << endl; break;
	}
}

// 函数名:OutputSymTable
// 功能:输出符号表
void OutputSymTable()
{
	int i = 1;
	cout << "符号名 "<< "符号类型" << " 符号所在层次 " << " " << "符号相对地址"<< " " << "符号出现频率"<<endl;
	while (SymTable[i].num)
	{
		cout <<"符号名: "<<SymTable[i].name << "   符号类型:  "  << SymTable[i].kind << "  符号所在层次:"  << SymTable[i].level << "  符号相对地址:"  << SymTable[i].adr << " 符号相对地址:" << SymTable[i].num << endl;
		i++;
	}
}

// 函数名:OutputPcode
// 功能:输出目标代码
void OutputPCode()
{ 
	for (int i = 0; i < cx; i++)
	{
		cout << order[Pcode[i].f] << " ";
		cout << Pcode[i].l << " ";
		cout << Pcode[i].a << " ";
		cout << endl;
	}
}

// 函数名:GeneratePCode
// 功能:生成PCode目标代码
// 注解参数:f为对应指令的位置,l,a为基本块编号
void GeneratePCode(int f,int l,int a)
{   
	Pcode[cx].f=f;
	Pcode[cx].l=l;          
	Pcode[cx].a=a;
	cx++;          //这样使每次cx都是指向最新生成的指令的下一位，便于记录需要回填的位置 
}

// 函数名:IsExistInTheSameLevelTable
// 功能:遍历符号表，判断该符号是否在同一层的符号表中,若存在则返回,否则返回0
// 注解参数:name是常量名也是符号名,lev是层次
bool IsExistInTheSameLevelTable(string name, int lev)	{
	for (int i = 1;i <= tx; i++)
	{
		if (SymTable[i].name == name && SymTable[i].level == lev)
		{
			return 1;
		}
	}
	return 0;
}

// 函数名:IsExistInThePreviousLevelTable
// 功能:遍历符号表，判断是否在同层或者上一层已经定义了该变量,若存在则返回,否则返回0
// 注解参数:name是常量名也是符号名,lev是层次
bool IsExistInThePreviousLevelTable(string name, int lev)
{      
	//为了检查符号 未定义就使用的错误 
	for (int i = 1;i <= tx;i++)
	{
		if (SymTable[i].name == name && SymTable[i].level <= lev)
		{
			return 1;
		}
	}
	return 0;
}

// 函数名:QueryTheLastProcessIncludeBlock
// 功能:从后往前遍历符号表,查询最近包含block语句的过程
// 注解参数:
int QueryTheLastProcessIncludeBlock()	{
	for (int i = tx; i >= 1; i--)
		if (SymTable[i].kind==2)
			return i;
	return -1;
}

// 函数名:StringToInt
// 功能:将字符串转为一个整数
// 注解参数:name为字符串
int StringToInt(string name){
	int num = 0;
	for (int i = 0; i < name.size(); i++)
	{
		num = num * 10 + (name[i] - '0');
	}
	return num;
}

// 函数名:AddVariableToSymTable
// 功能:
// 注解参数:name是变量名也是符号名,level是层次,adr是相对本层基地址的偏移量
void AddVariableToSymTable(string name, int level, int adr)	{
	tx=tx+1;  							//符号表扩容
	SymTable[tx].kind=variable;			//符号类型
	SymTable[tx].name=name;				//变量名也是符号名
	SymTable[tx].level=level;			//层次
	SymTable[tx].adr=adr;       		//相对本层基地址的偏移量
	SymTable[tx].num=1;					//频率
}
// 函数名:AddConstToSymTable
// 功能:将常量添加进符号表
// 注解参数:name是常量名也是符号名,level是层次,val是本层相对基地址的偏移量。
void AddConstToSymTable(string name, int level, int val)	{
	tx=tx+1;  						//符号表扩容
	SymTable[tx].kind=constant;		//符号类型
	SymTable[tx].name=name;			//常量名也即符号名
	SymTable[tx].level=level;		//层次
	SymTable[tx].value=val;			//相对本层基地址的偏移量
	SymTable[tx].num=1;				//频率
}

// 函数名:AddprocedureToSymTable
// 功能:将进程名即程序名添加进符号表
// 注解参数:
void AddprocedureToSymTable(string name, int level, int adr){
	tx = tx + 1;
	SymTable[tx].kind = procedure;
	SymTable[tx].name = name;
	SymTable[tx].level = level;
	SymTable[tx].adr = adr;
	SymTable[tx].num = 1;
}

void ReadLine()                               //每次从词法分析读入一个单词ok 
{
	string line;
	if (!readLexi.eof())
	{
		getline(readLexi, line);
		if (readLexi.eof())
		{
			return;
		}
		istringstream iss(line);
		iss >> unit.value;
		iss >> unit.key;
		iss >> unit.line;
		iss >> unit.column;
	}
	else
		return;
}

void ThrowError(string error, int ismissing)  //语法分析报错ok 
{
	if (ismissing == 1)
	{
		cout << error + " is missing ";
		unit.print();
	}
	if (ismissing == 0)
	{
		cout << error + " is extra ";
		unit.print();
	}
}

// 函数名: prog
// 功能:读取中间代码前置，检查是否正确
void prog()
{
	if (unit.value!="program")
	{
		ThrowError(unit.value, 0);
		return;
		ReadLine();
	}
	ReadLine();
	if (unit.key != "ID")
		ThrowError("ID", 1);
	else
		ReadLine();
	if (unit.value != ";")
	{
		ThrowError(";", 1);
	}
	else
		ReadLine();
	block();
}

// 函数名:Block
// 功能:根据读取到的单词值进行符号表装填,并且为过程调用的局部参数开辟空间
void block()
{
	int dx_now = dx;		  //当前过程的dx偏移量
	int tx_now = tx + 1;     //当前过程符号表的tx
	int n = 0;            //用来保留父过程的符号表长度

	if (tx_now > 1)		  //判断当前过程是否是主过程
	{
		n = QueryTheLastProcessIncludeBlock();
		tx_now = tx_now - SymTable[n].size;     //tx_now减去形参的个数
		
	}	

	if (tx_now > 1)					 		//判断当前过程是否是主过程
			dx = 3 + SymTable[n].size;   		//不是主过程,定义的变量的偏移量要从3+形参个数开始计算
		else
			dx = 3;                      		//是主过程,定义的变量的偏移量要从3开始计算


	int cx_now = cx;   //cx_now记录需要回填的指令序号
	GeneratePCode(JMP, 0, 0);		//扫描遍历程序,因为主过程可能不在首位,因此需要先无条件跳转到主过程
						//之后的跳转则由需回填的指令地址决定
	while ((unit.value != "const") && (unit.value != "var") && (unit.value != "procedure") && (unit.value != "begin"))
	{
		ThrowError(unit.value, 0);
		ReadLine();
	}
	if (unit.value == "const")
		LexConst();			//常量处理
	if (unit.value == "var")
		LexVariable();    		//变量处理，生成符号表,不生成指令
	if (unit.value == "procedure")
	{
		LexProcedure();
		lev--;  			//层数减一
	}
	if (tx_now > 1)   		//判断当前过程是否是主过程
	{
		int i;

		n = QueryTheLastProcessIncludeBlock();   //找到最近的过程调用


		for (i = 0; i < SymTable[n].size; i++)     //把数据栈栈顶已有的形参值存入预留的空间,传递形参值
		{	
			GeneratePCode(STO, 0, SymTable[n].size + 3 - 1 - i);
		}
	}

	Pcode[cx_now].a = cx; // 将预留的指令地址回填

	mm = cx;            //记录预留的指令地址 
	GeneratePCode(INT, 0, dx);    //开辟dx个空间,方便过程调用的参数回填 
    //
	if (tx != 1)//判断当前过程是否是主过程,不是则计算入口地址
		SymTable[n].value = cx - 1 - SymTable[n].size;  //过程入口地址,
	body();
	GeneratePCode(OPR, 0, 0);    //退栈 

	tx = tx_now;   //将符号表标记位前移,避免父过程使用子过程局部变量
	dx = dx_now;   //调整偏移量
}
// 函数名:LexConst
// 功能:处理中间代码常量数据
// 注解参数:
void LexConst()//ok
{
	while (unit.value != "const")
	{
		ThrowError(unit.value, 0);
		ReadLine();
	}
	ReadLine();
	ConstIntoSymTable();
	while (unit.value == ",")
	{
		ReadLine();
		ConstIntoSymTable();
	}
	if (unit.value != ";")
	{
		ThrowError(";", 1);
	}
	else
		ReadLine();
}

// 函数名:ConstIntoSymTable
// 功能:识别常量数据，加入符号表
// 注解参数:
void ConstIntoSymTable()//ok
{
	while (unit.key != "ID")
	{
		ThrowError(unit.value, 0);
		ReadLine();
	}
	string name = unit.value;

	ReadLine();
	if (unit.value != ":=")
		ThrowError(":=", 1);
	else
		ReadLine();
	if (unit.key != "INT")
		ThrowError("INT", 1);
	else 
	{
		int value = StringToInt(unit.value);

		if (IsExistInTheSameLevelTable(name, lev))
			error(15);

		AddConstToSymTable(name,lev,value);

		ReadLine();

	}
}

// 函数名:LexVariable
// 功能:处理中间代码变量数据
// 注解参数:
void LexVariable()//ok
{
	while (unit.value != "var")
	{
		ThrowError(unit.value, 0);
		ReadLine();
	}
	ReadLine();
	if (unit.key != "ID")
		ThrowError("ID", 1);
	else 
	{
		string name = unit.value;
		if (IsExistInTheSameLevelTable(name, lev))
			error(15);

		AddVariableToSymTable(name, lev, dx);
		dx++;    //已经放进去一个变量了，偏移量要往后加 


		ReadLine();
	}
	while (unit.value == ",")
	{
		ReadLine();
		if (unit.key != "ID")
			ThrowError("ID", 1);
		else 
		{
			string name = unit.value;

			if (IsExistInTheSameLevelTable(name, lev))
				error(15);

			AddVariableToSymTable(name, lev, dx);
			dx++;
			ReadLine();
		}

	}
	if (unit.value != ";")
		ThrowError(";", 1);
	else 
		ReadLine();
}

// 函数名:LexProcedure
// 功能:处理中间代码跳转程序指令
// 注解参数:
void LexProcedure()//ok 
{
	int count = 0;  //+3可以表示参数在这一层的相对地址 
	int tx0;
	while (unit.value != "procedure")
	{
		ThrowError(unit.value, 0);
		ReadLine();
	}
	ReadLine();
	if (unit.key != "ID")
		ThrowError("ID", 1);
	else 
	{
		string name = unit.value;
		if (IsExistInTheSameLevelTable(name, lev))
			error(15);
		tx0 = tx + 1;   // 
		AddprocedureToSymTable(name, lev, dx);
		lev++;   //层次深入 

		ReadLine();
	}
	if (unit.value != "(")
		ThrowError("(", 1);
	else
		ReadLine();
	if (unit.key == "ID")
	{
		string name = unit.value;
		AddVariableToSymTable(name, lev, 3 + count);    //+3是为了表示在这一层的偏移量，前面的0，1，2是SL,DL那三个 
		count++;
		SymTable[tx0].size = count;         //记录下此过程需要多少个（形式/实际）参数 
                                            //如果下面while语句不执行，那此刻就确定了 
		ReadLine();
		while (unit.value == ",")
		{
			ReadLine();
			if (unit.key != "ID")
				ThrowError("ID", 1);
			else 
			{
				string name = unit.value;
				AddVariableToSymTable(name, lev, 3 + count);
				count++;
				SymTable[tx0].size = count;
				ReadLine();
			}
		}
	}
	if (unit.value != ")")
		ThrowError(")", 1);
	else 
		ReadLine();
	if (unit.value != ";")
		ThrowError(";", 1);
	else 
		ReadLine();
	block();
	while (unit.value == ";")
	{
		ReadLine();
		LexProcedure();
	}
}

//处理body
void body()
{
	while (unit.value != "begin")
	{
		ThrowError(unit.value, 0);
		ReadLine();
	}
	ReadLine();
	statement();
	while (unit.value == ";")
	{
		ReadLine();
		statement();
	}
	if (unit.value != "end")
		ThrowError("end", 1);
	else 
		ReadLine();
}
//处理statement
void statement()
{
	while (unit.key != "ID"&&unit.value != "if"&&unit.value != "while"&&unit.value != "call"&&unit.value != "begin"&&unit.value != "read"&&unit.value != "write")
	{
		ThrowError(unit.value, 0);
		ReadLine();
	}
	//处理表达式
	if (unit.key == "ID")
	{
		string name = unit.value;

		//检查是否存在
		if(!IsExistInThePreviousLevelTable(name, lev)){
			error(14);
		}
		
		ReadLine();

		if (unit.value != ":=")
			ThrowError(":=", 1);
		else 
			ReadLine();
		
		exp();

		if (IsExistInThePreviousLevelTable(name, lev)) {
			int i = FindSymPosition(name);
			SymTable[i].num++;
			//检查类型
			if (SymTable[i].kind == variable){
				GeneratePCode(STO, lev - SymTable[i].level, SymTable[i].adr);
			}
			else{
				error(12);
			}
		}
	}
	//处理if条件语句
	else if (unit.value == "if")
	{
		ReadLine();
		lexp();				//将会生成一系列中间代码，最后在栈顶留下表达式最终的值
		int temp_cx1;		//记录下需要回填的指令的位置
		if (unit.value != "then"){
			ThrowError("then", 1);
		}
		else{
			temp_cx1 = cx;
			GeneratePCode(JPC, 0, 0);	//要回填的指令，若lexp为假，则跳过之后的statement
			ReadLine();
		}
		statement();
		Pcode[temp_cx1].a = cx;

		if (unit.value == "else"){	//如果有else，需要在JPC代码前加入JMP以在lexp为真时跳过else相关语句
			int temp_cx2 = cx;		//记录下需要回填指令的位置
			GeneratePCode(JMP, 0, 0);	//插入JMP
			Pcode[temp_cx1].a = cx;		//更新JPC指令跳转位置
			ReadLine();
			statement();
			Pcode[temp_cx2].a = cx;//回填 
		}  
	}
	else if (unit.value == "while"){
		int temp_cx1 = cx;			//记录while开始的位置
		int temp_cx2;
		ReadLine();
		lexp();
		if (unit.value != "do"){
			ThrowError("do", 1);
		}
		else {
			temp_cx2 = cx;			//记录要回填的语句位置
			GeneratePCode(JPC, 0, 0);
			ReadLine();
			statement();
			GeneratePCode(JMP, 0, temp_cx1);
			Pcode[temp_cx2].a = cx;
		}
	}
	else if (unit.value == "call")
	{
		ReadLine();
		int count = 0, index;		//count记录调用时形式参数个数
		if (unit.key != "ID"){
			ThrowError("ID", 1);
		}
		else {
			string proc_id = unit.value;
			if (!IsExistInThePreviousLevelTable(proc_id, lev)){
				error(10);
			}
			else {
				index=FindSymPosition(proc_id);
				SymTable[index].num++;
				if (SymTable[index].kind != 2)   //如果不是过程，则判错 
					error(11);
			}
			ReadLine();
		}
		if (unit.value != "("){
			ThrowError("(", 1);
		}
		else{
			ReadLine();
		}
		if (unit.value == "+" || unit.value == "-" || unit.key == "ID" || unit.key == "INT" || unit.value == "("){	 //exp的first集 
			exp();
			count++;
			while (unit.value == ",")
			{
				ReadLine();
				exp();
				count++;
			}
			if (count != SymTable[index].size){  //判断调用过程参数个数是否匹配，不匹配就报错 
				error(16);
			} 
		}
		if (unit.value != ")") {
			ThrowError(")", 1);
		}
		else {
			GeneratePCode(CAL, lev - SymTable[index].level, SymTable[index].value);  //SymTable[i].value 是要跳转的过程的入口地址（第一条指令编号）
			ReadLine();
		}
	}
	else if (unit.value == "begin"){
		body();
	}
	else if (unit.value == "read")
	{
		ReadLine();
		if (unit.value != "(") {
			ThrowError("(", 1);
		}
		else {
			ReadLine();
		}
		if (unit.key != "ID") {
			ThrowError("ID", 1);
		}
		else {
			string name = unit.value;
			if (!IsExistInThePreviousLevelTable(name, lev)){
				error(10);
			}
			else {
				int index = FindSymPosition(name);
				SymTable[index].num++;

				if (SymTable[index].kind == 1)
					GeneratePCode(RED, lev - SymTable[index].level, SymTable[index].adr);
				else
					error(12);
			}
			ReadLine();
		}
		while (unit.value == ","){
			ReadLine();
			string name = unit.value;
			if (!IsExistInThePreviousLevelTable(name, lev)){
				error(10);
			}
			else {
				int i = FindSymPosition(name);
				SymTable[i].num++;
				if (SymTable[i].kind == 1) GeneratePCode(RED, lev - SymTable[i].level, SymTable[i].adr);
				else error(12);
			}
			ReadLine();
		}
		if (unit.value != ")") {
			ThrowError(")", 1);
		}
		else {
			ReadLine();
		}
	}
	else if (unit.value == "write")
	{
		ReadLine();
		if (unit.value != "("){
			ThrowError("(", 1);
		}
		else{ 
			ReadLine();
		}
		exp();
		GeneratePCode(WRT, 0, 0);    		//每个exp计算完成立刻输出 
		while (unit.value == ","){
			ReadLine();
			exp();
			GeneratePCode(WRT, 0, 0);
		}
		GeneratePCode(OPR, 0, 15);           //屏幕输出换行分隔 
		if (unit.value != ")"){
			ThrowError(")", 1);
		}
		else{
			ReadLine();
		}
	}
}
int lop(){
	if (unit.value=="="){
		ReadLine();
		return 0;
	}
	else if (unit.value=="<>"){
		ReadLine();
		return 1;
	}
	else if (unit.value=="<"){
		ReadLine();
		return 2;
	}
	else if (unit.value=="<="){
		ReadLine();
		return 3;
	}
	else if (unit.value==">"){
		ReadLine();
		return 4;
	}
	else if (unit.value==">="){
		ReadLine();
		return 5;
	}
	return -1;
}
void lexp() {
	if (unit.value=="odd"){
		ReadLine();
		exp();
		GeneratePCode(OPR, 0, 6);   //单个的情况，无非是0或1，真或假 
	}
	else {
		exp();
		int index=lop();
		exp();
		if (index==0)
			GeneratePCode(OPR, 0, 8);
		else if (index==1)
			GeneratePCode(OPR, 0, 9);
		else if (index==2)
			GeneratePCode(OPR, 0, 10);
		else if (index==3)
			GeneratePCode(OPR, 0, 13);
		else if (index==4)
			GeneratePCode(OPR, 0, 12);
		else if (index==5)
			GeneratePCode(OPR, 0, 11);
	}
}
//处理":="右边部分
void exp(){
	string temp;
	while (unit.value!="+"&&unit.value!="-"&&unit.key!="ID"&&unit.key!="INT"&&unit.value!="("){
		ThrowError(unit.value, 0);
		ReadLine();
	}
	if (unit.value=="+"||unit.value=="-"){
		temp=unit.value;
		ReadLine();
	}
	term();
	if (temp=="-")
		GeneratePCode(OPR, 0, 1);

	while (unit.key=="aop"){
		temp = unit.value;
		ReadLine();
		term();
		if (temp == "+")
			GeneratePCode(OPR, 0, 2);
		else
			GeneratePCode(OPR, 0, 3);
	}
}
//处理term
void term() {
	factor();
	while (unit.key=="mop"){
		string temp=unit.key;
		ReadLine();
		factor();
		if (temp=="*") GeneratePCode(OPR,0,4);
		else if (temp=="/") GeneratePCode(OPR,0,5);
	}
}
//处理factor
void factor(){
	int index;
	while (unit.key != "ID"&&unit.key != "INT"&&unit.value != "("){
		ThrowError(unit.value, 0);
		ReadLine();
	}
	if (unit.key == "ID"){
		string name = unit.value;

		if(!IsExistInThePreviousLevelTable(name, lev))
			error(10);
		else {
			index=FindSymPosition(name);      //肯定能找到，因为前面if判断过了 
			SymTable[index].num++;     //把使用次数+1.证明除了声明的时候说了一次，这个变量确实使用了 

			if(SymTable[index].kind==1)
				GeneratePCode(LOD,lev-SymTable[index].level,SymTable[index].adr);
			else if(SymTable[index].kind==0)
				GeneratePCode(LIT,0,SymTable[index].value);
			else
			{
				error(12);
				return;
			}
		}
		ReadLine();
	}
	else if (unit.key=="INT"){
		GeneratePCode(LIT, 0, StringToInt(unit.value));
		ReadLine();
	}
	else if (unit.value=="("){
		ReadLine();
		exp();
		if (unit.value!=")")
			ThrowError(")", 1);
		else
			ReadLine();
	}
}

//解释器部分

// 函数名:getBase
// 功能:根据层差得到基地址
// 注解参数:当前基地址nowBp，层差lev
int getBase(int nowBp, int lev) {                                                
	int oldBp = nowBp;
	//当存在层差时沿着静态链往前找,寻找非局部变量
	while (lev > 0) {
		//定位到其直接外层的活动记录首地址
		oldBp = dataStack[oldBp + 1];
		lev--;
	}
	return oldBp; 
}

// 函数名:interpreter
// 功能：解释器
void interpreter(){ 
	P = 0;
	B = 0;
	T = 0;
	int t;	//用于输入输出
	do {
		I = P;
		P++;
		//根据伪操作码执行
		switch(Pcode[I].f) {
			//LIT 0,a
			case 0:	
				//常量a放入数据栈栈顶
				dataStack[T] = Pcode[I].a;
				T++;
				break;
			//OPR 0,a
			case 1:
				//执行运算a
				switch (Pcode[I].a){
					//OPR 0,0 
					case 0:
						//调用过程结束后，返回调用点并退栈						
						T = B;
						P = dataStack[B + 2];	//返回地址
						B = dataStack[B];	//静态链
						break;
					//opr 0,1
					case 1:
						//取反
						dataStack[T - 1] = -dataStack[T - 1];
						break;
					//opr 0,2
					case 2:
						//相加，退栈，结果进栈
						dataStack[T - 2] = dataStack[T - 1] + dataStack[T - 2];
						T--;
						break;
					//OPR 0,3
					case 3:
						//相减，退栈，结果进栈
						dataStack[T - 2] = dataStack[T - 2] - dataStack[T - 1];
						T--;
						break;
					//OPR 0,4
					case 4:
						//相乘，退栈，结果进栈
						dataStack[T - 2] = dataStack[T - 1] * dataStack[T - 2];
						T--;
						break;
					//OPR 0,5
					case 5:
						//相除，退栈，结果进栈
						dataStack[T - 2] = dataStack[T - 2] / dataStack[T - 1];
						T--;
						break;
					//OPR 0,6
					case 6:                 //栈顶元素值奇偶判断，结果值进栈,奇数为1
						dataStack[T - 1] = dataStack[T - 1] % 2;
						break;
					//OPR 0,7
					case 7:
						break;
					//OPR 0,8
					case 8:
						//判断相等，退栈，结果进栈
						if (dataStack[T - 1] == dataStack[T - 2]) {
							dataStack[T - 2] = 1;
							T--;
							break;
						}
						dataStack[T - 2] = 0;
						T--;
						break;
					//OPR 0,9
					case 9:
						//判断不等，退栈，结果进栈
						if (dataStack[T - 1] != dataStack[T - 2]) {
							dataStack[T - 2] = 1;
							T--;
							break;
						}
						dataStack[T - 2] = 0;
						T--;
						break;
					//OPR 0,10
					case 10:
						//判断小于，退栈，结果进栈
						if (dataStack[T - 2] < dataStack[T - 1]) {
							dataStack[T - 2] = 1;
							T--;
							break;
						}
						dataStack[T - 2] = 0;
						T--;
						break;
					//OPR 0,11
					case 11:
						//判断大于等于，退栈，结果进栈
						if (dataStack[T - 2] >= dataStack[T - 1]) {
							dataStack[T - 2] = 1;
							T--;
							break;
						}
						dataStack[T - 2] = 0;
						T--;
						break;
					//OPR 0,12
					case 12:
						//判断大于，退栈，结果进栈
						if (dataStack[T - 2] > dataStack[T - 1]) {
							dataStack[T - 2] = 1;
							T--;
							break;
						}
						dataStack[T - 2] = 0;
						T--;
						break;
					//OPR 0,13
					case 13:
						//判断小于等于，退栈，结果进栈
						if (dataStack[T - 2] <= dataStack[T - 1])
						{
							dataStack[T - 2] = 1;
							T--;
							break;
						}
						dataStack[T - 2] = 0;
						T--;
						break;
					//OPR 0,15
					case 15:
						//屏幕输出换行
						cout << endl;
						break;
				}
				break;
			//LOD L,a
			case 2:
				//变量相对地址为a，层差为L，放入数据栈栈顶
				dataStack[T] = dataStack[Pcode[I].a + getBase(B, Pcode[I].l)];
				T++;
				break;
			//STO L,a
			case 3:
				//数据栈栈顶的内容存入变量，相对地址为a，层次差为L
				dataStack[Pcode[I].a + getBase(B, Pcode[I].l)] = dataStack[T - 1];
				T--;
				break;
			//CAL L,a 
			case 4:
				//调用过程，入口地址为a，层次差为L
				dataStack[T] = B;	//动态链，调用前运行过程
				dataStack[T + 1] = getBase(B, Pcode[I].l);	//静态链，直接外层过程
				dataStack[T + 2] = P;	//返回地址，顺序产生出来的下一条要执行的指令序号 
				B = T;
				P = Pcode[I].a;
				break;
			//INT 0,a
			case 5:
				//数据栈栈顶指针增加a
				T = B + Pcode[I].a;
				break;
			//JMP 0,a
			case 6:
				//无条件转移到地址为a的指令
				P = Pcode[I].a;
				break;
			//JPC 0,a
			case 7:
				//条件转移指令，转移到地址为a的指令
				if (dataStack[T - 1] == 0) {
					P = Pcode[I].a;
				}
				break;
			//RED L,a 
			case 8:
				//从命令行读入一个数据并存入变量
				cin >> t;
				dataStack[Pcode[I].a + getBase(B, Pcode[I].l)] = t;
				break;
			//WRT
			case 9:
				//栈顶值输出至屏幕
				cout << dataStack[T - 1];
				cout << "  ";
				break;
		}
	} while (P != 0);
}
