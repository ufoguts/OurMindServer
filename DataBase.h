
// 数据库
//

#pragma once

#define USER_NUM 1000


//文件存储结构：
/*
文件目录：
{
UserData.dat
MindData.dat
(求和MindNum个)<UserName>_<MindName>.mind
}

文件内部结构存储：
UserData.dat：
{
(int)UserNum
(UserNum个)(tUserData)UserData
}

MindData.dat：
{
(int)UserNum
(UserNum个)(ListTemplet<class BlockNode>)MindList
}


<UserName>_<MindName>.mind


运行后，读取 UserData.dat，MindData.dat 文件
<UserName>_<MindName>.mind 文件用到时再读取


/****************************************************************/
/*************************服务器同步开始*************************/
/****************************************************************/
/*
附文件、数据结构
CString：
{
(int)Length
(Length个)(wchar_t)ch
}
其余的结构体或类见其定义
*/

//链表操作
#define LIST_GO(list, p) ((list) = (list)->p)
#define LIST_FORCE(list, p, type) ((list) = (type)((list)->p))

#define MAX_SHARE 100//思维导图最大共享人数
#define MAX_JOIN 100//最多参与思维导图数

#define MAX_BLOCK_DEEP 200//思维导图最大深度

#define DEFAULT_LENGTH 550//默认导图长
#define DEFAULT_WIDTH 450//默认导图宽

#define DEFAULT_BLOCK_LENGTH 70//默认分块长
#define DEFAULT_BLOCK_WIDTH 40//默认分块宽

#define MIN_LENGTH 110//最小导图长
#define MIN_WIDTH 90//最小导图宽

#define MIN_BLOCK_LENGTH 14//最小分块长
#define MIN_BLOCK_WIDTH 8//最小分块宽


//链表模板
//文件、数据结构
/*
(int)num
(num个)(TYPE)Node
*/
template <typename TYPE>
class ListTemplet
{
	//变量
public:

	TYPE * const qnode;
	TYPE *pnode;
	TYPE *ptail;
	int num;

	//方法
public:
	ListTemplet(): qnode(new TYPE)
	{
		pnode = ptail = qnode;
		num = 0;
	};
	~ListTemplet()
	{
		pnode = qnode;
		while(pnode->pt)
		{
			//LIST_FORCE(pnode, pt, TYPE*);
			LIST_GO(pnode, pt);
			delete pnode->qt;
		}
		delete pnode;
	};
	void Add();//在pnode后面插入节点，pnode指向新节点
	void Del();//删除pnode后面的节点，pnode不变
	TYPE *Cut();//剪切pnode后面的节点，pnode不变
	void Paste(TYPE *p);//在pnode后面粘贴节点，pnode指向新节点
	void Clear();//清空链表项
	void ReadFile(FILE *fp);//从文件读取
	void WriteFile(FILE *fp);//写入文件
	void Send(SOCKET sock);//从网络发送
	void Recv(SOCKET sock);//从网络接收
};
//链表节点基类
//文件、数据结构
/*
依据TYPE覆盖虚函数决定
*/
template <typename TYPE>
class NodeBase
{
	//变量
public:
	TYPE *pt, *qt;

	//方法
public:
	NodeBase()
	{
		pt = qt = 0;
	};
	virtual void ReadFile(FILE *fp){};//从文件读取
	virtual void WriteFile(FILE *fp){};//写入文件
	virtual void Send(SOCKET sock){};//从网络发送
	virtual void Recv(SOCKET sock){};//从网络接收
};


//树模板
//文件、数据结构
//按节点递归的定义，初始为根节点的第一个子节点
/*
(当前节点及其兄弟节点个数个)
{
(bool)true
(递归存储)(第一个子节点)
(TYPE)Node
}
(bool)false
*/
template <typename TYPE>
class TreeTemplet
{
	//变量
public:
	TYPE * const qnode;
	TYPE *pnode;

	//方法
public:
	TreeTemplet(): qnode(new TYPE)
	{
		pnode = qnode;
	};
	~TreeTemplet()
	{
		while(qnode->pch)
		{
			pnode = qnode->pch;
			Del();
		}
		delete qnode;
	};
	void Add();//在pnode下面插入新节点作为第一个子节点，pnode指向新节点
	int Del();//删除pnode节点，其子节点归为其父节点的第一个子节点，依次向后排，pnode指向其父节点，返回子节点个数
	TYPE *Cut();//剪切pnode节点，pnode指向其父节点
	void Paste(TYPE *p);//将节点连接到pnode下面
	int GetLocal(int string[]);//位置转化为数组
	bool SetLocal(int string[], int num);//从数组设置位置，成功返回true
	void ReadFile(FILE *fp);//从文件读取
	void PartReadFile(FILE *fp, TYPE *p);//递归从文件读取部分
	void WriteFile(FILE *fp);//写入文件
	void PartWriteFile(FILE *fp, TYPE *p);//递归写入文件部分
	void Send(SOCKET sock);//从网络发送
	void PartSend(SOCKET sock, TYPE *p);//递归从网络发送部分
	void Recv(SOCKET sock);//从网络接收
	void PartRecv(SOCKET sock, TYPE *p);//递归从网络接收部分
};
//树节点基类
//文件、数据结构
/*
依据TYPE覆盖虚函数决定
*/
template <typename TYPE>
class TreeNodeBase
{
	//变量
public:
	TYPE *pt, *qt;
	TYPE *pch, *ppr;

	//方法
public:
	TreeNodeBase()
	{
		pt = qt = pch = ppr = 0;
	};
	virtual void ReadFile(FILE *fp){};//从文件读取
	virtual void WriteFile(FILE *fp){};//写入文件
	virtual void Send(SOCKET sock){};//从网络发送
	virtual void Recv(SOCKET sock){};//从网络接收
};


//用户数据结构体
//文件、数据结构
/*
(CString)UserId
(CString)Password
*/
struct tUserData
{
	//变量
public:

	//外存变量
	CString UserId;//用户名
	CString Password;//密码
	int JoinNum;//参与思维导图数
	CString JoinUser[MAX_JOIN];//思维导图拥有者名称
	CString JoinMind[MAX_JOIN];//思维导图名称

	//内存变量
	bool Online;//在线状态
	SYSTEMTIME LastTime;//上次keep时间
	class MindNode *pEditMind;//编辑的思维导图
	class BlockNode *pLockBlock;//锁定的分块
	ListTemplet<class OperateNode> OperateList;//操作链表

	//额外保存变量
	ListTemplet<class MindNode> MindList;//拥有思维导图

	//方法
public:
	tUserData();
	void ReadFile(FILE *fp);//从文件获取信息
	void WriteFile(FILE *fp);//将信息写入文件
};


//思维导图节点类
//文件、数据结构
/*
(CString)UserId
(CString)MindName
(int)Length
(int)Width
(bool)ReadOnly
(int)ShareNum
(ShareNum个)(CString)ShareUser
(CString)EditTime
*/
class MindNode: public NodeBase<MindNode>
{
	//变量
public:

	//外存变量
	CString UserId;//用户名
	CString MindName;//思维导图名称
	int Length; //长
	int Width;//宽
	int Pos_X;//最大分块位置x
	int Pos_Y;//最大分块位置x
	bool ReadOnly; //是否只读
	int ShareNum;//共享用户个数
	CString ShareUser[MAX_SHARE];//共享用户组
	CString EditTime;//编辑时间

	//内存变量
	int EditNum;//当前编辑人数
	bool EditStatus;//拥有者编辑状态
	bool UserStatus[MAX_SHARE];//用户编辑状态

	//额外保存变量
	TreeTemplet<class BlockNode> BlockTree;//思维导图分块树

	//方法
public:
	MindNode();
	void Copy(const MindNode &Node);//拷贝函数
	void ReadFile(FILE *fp);//从文件读取
	void WriteFile(FILE *fp);//写入文件
	void Send(SOCKET sock);//从网络发送
	void Recv(SOCKET sock);//从网络接收
};


//形状枚举
enum enumShape 
{
	rect, //长方形
	rore, //圆角矩形
	round, //圆形
	eche, //梯形
	diamond, //菱形
	para, //平行四边形
	octa, //八边形
};


//方向枚举
enum enumDir 
{
	dirup,
	dirdown,
	dirleft,
	dirright,
};


//思维导图分块节点类
class BlockNode: public TreeNodeBase<BlockNode>
{
	//变量
public:

	//外存变量
	int PosX;//位置x
	int PosY;//位置y
	enumShape Shape;//形状
	enumDir Dir;//连线方向
	int Color;//颜色
	int Length;//长
	int Width;//宽
	CString Text;//文字
	bool Lock;//是否锁定

	//内存变量
	CString LockUser;//锁定者，为空说明是禁止分析用户修改

	//方法
public:
	BlockNode();
	void Copy(const BlockNode &Node);//拷贝函数
	void ReadFile(FILE *fp);//从文件读取
	void WriteFile(FILE *fp);//写入文件
	void Send(SOCKET sock);//从网络发送
	void Recv(SOCKET sock);//从网络接收
};


//操作串类型枚举
enum enumOperate
{
	updatamind,//更新导图信息
	updatablock,//更新分块信息
	updatarelation,//更新分块关系
};
//分块关系类型枚举
enum enumRelation
{
	addroot,//分块添加根分块
	delwith,//分块删除连带子分块
	delcombine,//分块删除合并子分块
	addchild,//分块插入子分块
	addparent,//分块插入父分块
	lineto,//分块连接到新分块
	separateroot,//分块分离为根分块
	delseparate,//分块删除分离子分块
};


//分块信息操作数据
struct BlockOperateData
{
	//变量
public:
	int Deep;
	int Local[MAX_BLOCK_DEEP];
	BlockNode Block;
	bool SetWith;

	//方法
public:
	void Copy(const BlockOperateData &Data);
	void Send(SOCKET sock);
	void Recv(SOCKET sock);
};


//分块关系操作数据
struct RelationOperateData
{
	//变量
public:
	enumRelation Relation;
	int Deep1;
	int Deep2;
	int Local1[MAX_BLOCK_DEEP];
	int Local2[MAX_BLOCK_DEEP];
	int Message;

	//方法
public:
	void Copy(const RelationOperateData &Data);
	void Send(SOCKET sock);
	void Recv(SOCKET sock);
};


//操作包链表节点类
class OperateNode: public NodeBase<OperateNode>
{
	//变量
public:
	enumOperate Operate;//操作类型
	MindNode MindData;//导图信息操作数据
	struct BlockOperateData BlockData;//分块信息操作数据
	struct RelationOperateData RelationData;//分块关系操作数据

	//方法
public:
	void Copy(const OperateNode &Node);//拷贝函数
	void Send(SOCKET sock);//从网络发送
	void Recv(SOCKET sock);//从网络接收
};


//读文件导入CString
void ReadFileCString(FILE* fp, CString &string);


//CString数据写入文件
void WriteFileCString(FILE* fp, CString &string);


//二分查找，找不到返回前一个数
//CString
bool BinarySearch(const CString string[], const CString &target, int start, int end, int &result);
//CString、CString
bool BinarySearch(const CString string[], const CString string2[], const CString &target, 
				  const CString &target2, int start, int end, int &result);
//tUserData
bool BinarySearch(tUserData * const string[], const CString &target, int start, int end, int &result);


//对思维导图进行操作
bool OperateData(MindNode &TheMind, ListTemplet<class OperateNode> &OperateList);


//对思维导图进行关系操作
int OperateRelation(TreeTemplet<class BlockNode> &BlockTree, enumRelation Relation, int Local[], int Deep);


//递归设置分块锁定解锁
void LockBlockWith(BlockNode *pBlock, bool IsLock);

/****************************************************************/
/*************************服务器同步结束*************************/
/****************************************************************/


//对时间进行减法
int MinusTime(const SYSTEMTIME &time1, const SYSTEMTIME &time2);


//时间转换字符串
void GetTime(const SYSTEMTIME &time, CString &string);


//时间转换详细字符串
void GetAllTime(const SYSTEMTIME &time, CString &string);


//用户内存数据全局变量
extern tUserData *g_pUserData[USER_NUM];
extern int g_UserNum;


//数据库初始化
void DataInit();


//数据库保存
void DataSave();


//添加用户
bool AddUser(CString &UserId, CString &Password);


//核对用户
bool TestUser(CString &UserId, CString &Password);


//修改用户信息
bool ChangeUser(CString &UserId, CString &Password, CString &Password2);


//找到用户拥有思维导图链表
ListTemplet<MindNode> *GetOwnMindList(CString &UserId);


//构造用户参与思维导图链表
ListTemplet<MindNode> *BuildJoinMindList(CString &UserId);


//新建思维导图
MindNode *NewMind(CString &UserId);


//选择思维导图
MindNode *SelMind(CString &UserId, CString &MindUserId, CString &MindName);


//删除思维导图
bool DelMind(CString &UserId, CString &MindName);


//结束编辑
enum PacketType EditEnd(CString &UserId);


//更改思维导图名称
enum PacketType ChangeName(CString &UserId, CString &MindName);


//添加共享用户
enum PacketType AddShare(CString &UserId, CString &ShareUserId);


//删除共享用户
enum PacketType DelShare(CString &UserId, CString &ShareUserId);


//设置分块锁定解锁
enum PacketType LockBlock(CString &UserId, int Deep, int Local[], ListTemplet<OperateNode> * &pReOperateList, 
						  bool IsLock, bool SetWith);


//上传思维导图
enum PacketType UpMind(CString &UserId, MindNode &TheMind, ListTemplet<OperateNode> * &pReOperateList);


//保持思维导图
enum PacketType KeepMind(CString &UserId, ListTemplet<OperateNode> * &pReOperateList);


//下载思维导图
enum PacketType DownMind(CString &UserId, ListTemplet<OperateNode> * &pReOperateList);


//申请分块
enum PacketType ApplyBlock(CString &UserId, int Local[], int Deep, ListTemplet<OperateNode> * &pReOperateList);


//释放分块
enum PacketType FinishBlock(CString &UserId, BlockNode &NewBlock, ListTemplet<OperateNode> * &pReOperateList);


//添加根分块
enum PacketType BlockAddRoot(CString &UserId, int x, int y, ListTemplet<OperateNode> * &pReOperateList);


//删除连带子分块
enum PacketType BlockDelWith(CString &UserId, BlockNode &NewBlock, ListTemplet<OperateNode> * &pReOperateList);


//删除连带子分块
enum PacketType BlockDelCombine(CString &UserId, BlockNode &NewBlock, ListTemplet<OperateNode> * &pReOperateList);


//添加子分块
enum PacketType BlockAddChild(CString &UserId, BlockNode &NewBlock, ListTemplet<OperateNode> * &pReOperateList, 
							  int x, int y);


//插入父分块
enum PacketType BlockAddParent(CString &UserId, BlockNode &NewBlock, ListTemplet<OperateNode> * &pReOperateList, 
							   int x, int y);


//连接到已有分块
enum PacketType BlockLineInto(CString &UserId, BlockNode &NewBlock, ListTemplet<OperateNode> * &pReOperateList, 
							  int Deep, int Local[]);


//分离为根分块
enum PacketType BlockSeparateRoot(CString &UserId, BlockNode &NewBlock, ListTemplet<OperateNode> * &pReOperateList);


//删除分离子分块
enum PacketType BlockDelSeparate(CString &UserId, BlockNode &NewBlock, ListTemplet<OperateNode> * &pReOperateList);


//更新操作链表，修改导图编辑时间
//mind更新
void RefreshOperateList(MindNode &TheMind, SYSTEMTIME *ptime, bool OwnUser);
//Bolck更新
void RefreshOperateList(MindNode &TheMind, SYSTEMTIME *ptime, int Deep, int Local[], bool SetWith);
//Relation更新
void RefreshOperateList(MindNode &TheMind, SYSTEMTIME *ptime, enumRelation Relation,  
						int Deep1, int Local1[], int Deep2, int Local2[], int Message);


//刷新定位信息
void RefreshBlockLocal(int Local[], int &Deep, ListTemplet<OperateNode> &OperateList);


//递归刷新分块位置最大值
void RefreshMaxPos(BlockNode *pBlock, int &Pos_X, int &Pos_Y);


//递归判断是否被锁定
bool JudgeBlockLock(BlockNode *pBlock, bool Own);

