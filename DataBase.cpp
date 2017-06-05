
// 数据库
//

#include "stdafx.h"
#include "OurMindServer.h"
#include "OurMindServerDlg.h"



//用户内存数据全局变量，内部从小到大存储
//以指针数组开启堆空间保存，节省空间以及移动快速
tUserData *g_pUserData[USER_NUM];
int g_UserNum;



/****************************************************************/
/*************************服务器同步开始*************************/
/****************************************************************/

//整形全局缓存变量
int g_IntBuf[10000];


//ListTemplet方法
template <typename TYPE>
void ListTemplet<TYPE>::Add()
{
	TYPE *p = pnode->pt;
	pnode->pt = new TYPE;
	pnode->pt->qt = pnode;
	pnode->pt->pt = p;
	if(p)
		p->qt = pnode->pt;
	LIST_GO(pnode, pt);
	if(ptail->pt)
		LIST_GO(ptail, pt);
	num ++;
}
template <typename TYPE>
void ListTemplet<TYPE>::Del()
{
	TYPE *p = pnode->pt;
	if(ptail->qt==pnode)
		LIST_GO(ptail, qt);
	pnode->pt = pnode->pt->pt;
	if(pnode->pt)
		pnode->pt->qt = pnode;
	delete p;
	num --;
}
template <typename TYPE>
TYPE *ListTemplet<TYPE>::Cut()
{
	TYPE *p = pnode->pt;
	if(ptail->qt==pnode)
		LIST_GO(ptail, qt);
	pnode->pt = pnode->pt->pt;
	if(pnode->pt)
		pnode->pt->qt = pnode;
	num --;
	return p;
}
template <typename TYPE>
void ListTemplet<TYPE>::Paste(TYPE *p)
{
	p->pt = pnode->pt;
	if(p->pt)
		p->pt->qt = p;
	pnode->pt = p;
	p->qt = pnode;
	pnode = p;
	if(ptail->pt)
		LIST_GO(ptail, pt);
	num ++;
}
template <typename TYPE>
void ListTemplet<TYPE>::Clear()
{
	pnode = ptail;
	while(pnode->qt)
	{
		LIST_GO(pnode, qt);
		delete pnode->pt;
	}
	qnode->pt = 0;
	ptail = qnode;
	num = 0;
}
template <typename TYPE>
void ListTemplet<TYPE>::ReadFile(FILE *fp)
{
	int i;
	int temp;
	fread(&temp, sizeof(num), 1, fp);
	pnode = qnode;
	for(i=0; i<temp; i++)
	{
		Add();
		pnode->ReadFile(fp);
	}
}
template <typename TYPE>
void ListTemplet<TYPE>::WriteFile(FILE *fp)
{
	int i;
	int temp = num;
	fwrite(&temp, sizeof(num), 1, fp);
	pnode = qnode;
	for(i=0; i<temp; i++)
	{
		pnode->pt->WriteFile(fp);
		Del();
	}
}
template <typename TYPE>
void ListTemplet<TYPE>::Send(SOCKET sock)
{
	int i;
	send(sock, (char *)&num, sizeof(num), 0);
	pnode=qnode;
	for(i=0; i<num; i++)
	{
		LIST_GO(pnode, pt);
		pnode->Send(sock);
	}
}
template <typename TYPE>
void ListTemplet<TYPE>::Recv(SOCKET sock)
{
	pnode = qnode;
	while(qnode->pt)
		Del();
	int i;
	int temp;
	CertainRecv(sock, &temp, sizeof(int));
	for(i=0; i<temp; i++)
	{
		Add();
		pnode->Recv(sock);
	}
}


//TreeTemplet方法
template <typename TYPE>
void TreeTemplet<TYPE>::Add()
{
	TYPE *p = pnode->pch;
	pnode->pch = new TYPE;//新节点作为pnode的第一个子节点
	pnode->pch->ppr = pnode;
	LIST_GO(pnode, pch);//pnode转到新节点
	pnode->pt = p;
	pnode->qt = 0;
	if(p)
		p->qt = pnode;
}
template <typename TYPE>
int TreeTemplet<TYPE>::Del()
{
	int num = 0;
	TYPE *p1, *p2;
	p1 = pnode->pch;
	p2 = pnode->ppr;
	//断开pnode的连接
	if(p2->pch==pnode)
		p2->pch = pnode->pt;
	if(pnode->pt)
		pnode->pt->qt = pnode->qt;
	if(pnode->qt)
		pnode->qt->pt = pnode->pt;
	//删除pnode，pnode转到其父节点
	delete pnode;
	pnode = p2;
	//处理pnode原本子节点
	if(p1)
	{
		num ++;
		//将其插入到父节点最前方
		p2 = pnode->pch;
		pnode->pch = p1;
		p1->ppr = pnode;
		while(p1->pt)
		{
			LIST_GO(p1, pt);
			p1->ppr = pnode;
			num ++;
		}
		p1->pt = p2;
		if(p2)
			p2->qt = p1;
	}
	return num;
}
template <typename TYPE>
TYPE *TreeTemplet<TYPE>::Cut()
{
	TYPE *p = pnode;
	LIST_GO(pnode, ppr);
	//断开p的连接
	if(pnode->pch==p)
		pnode->pch = p->pt;
	if(p->pt)
		p->pt->qt = p->qt;
	if(p->qt)
		p->qt->pt = p->pt;
	//返回p
	return p;
}
template <typename TYPE>
void TreeTemplet<TYPE>::Paste(TYPE *p)
{
	//p作为pnode第一个子节点
	p->pt = pnode->pch;
	pnode->pch = p;
	p->ppr = pnode;
	p->qt = 0;
	if(p->pt)
		p->pt->qt = p;
	pnode = p;
}
template <typename TYPE>
int TreeTemplet<TYPE>::GetLocal(int string[])
{
	int num = 0;
	int i;
	for(num=0; pnode->ppr; num++)
	{
		for(i=0; pnode->qt; i++)
			LIST_GO(pnode, qt);
		string[num] = i;
		LIST_GO(pnode, ppr);
	}
	return num;
}
template <typename TYPE>
bool TreeTemplet<TYPE>::SetLocal(int string[], int num)
{
	pnode = qnode;
	int i;
	for(num--; num>=0; num--)
	{
		if(pnode->pch==0)
			return false;
		LIST_GO(pnode, pch);
		for(i=0; i<string[num]; i++)
		{
			if(pnode->pt==0)
				return false;
			LIST_GO(pnode, pt);
		}
	}
	return true;
}
template <typename TYPE>
void TreeTemplet<TYPE>::ReadFile(FILE *fp)
{
	PartReadFile(fp, qnode);
}
template <typename TYPE>
void TreeTemplet<TYPE>::PartReadFile(FILE *fp, TYPE *p)
{
	bool temp;
	fread(&temp, sizeof(temp), 1, fp);
	if(temp==false)
	{
		p->pch = 0;
		return;
	}
	p->pch = new TYPE;
	p->pch->ppr = p;
	LIST_GO(p, pch);
	p->qt = 0;
	PartReadFile(fp, p);
	p->ReadFile(fp);
	fread(&temp, sizeof(temp), 1, fp);
	while(temp)
	{
		p->pt = new TYPE;
		p->pt->qt = p;
		LIST_GO(p, pt);
		p->ppr = p->qt->ppr;
		PartReadFile(fp, p);
		p->ReadFile(fp);
		fread(&temp, sizeof(temp), 1, fp);
	}
	p->pt = 0;
}
template <typename TYPE>
void TreeTemplet<TYPE>::WriteFile(FILE *fp)
{
	PartWriteFile(fp, qnode->pch);
	qnode->pch = 0;
}
template <typename TYPE>
void TreeTemplet<TYPE>::PartWriteFile(FILE *fp, TYPE *p)
{
	bool temp;
	TYPE *p2;
	while(p)
	{
		temp = true;
		fwrite(&temp, sizeof(temp), 1, fp);
		PartWriteFile(fp, p->pch);
		p->WriteFile(fp);
		p2 = p;
		LIST_GO(p, pt);
		delete p2;
	}
	temp = false;
	fwrite(&temp, sizeof(temp), 1, fp);
}
template <typename TYPE>
void TreeTemplet<TYPE>::Send(SOCKET sock)
{
	PartSend(sock, qnode->pch);
}
template <typename TYPE>
void TreeTemplet<TYPE>::PartSend(SOCKET sock, TYPE *p)
{
	bool temp;
	TYPE *p2;
	while(p)
	{
		temp = true;
		send(sock, (char *)&temp, sizeof(temp), 0);
		PartSend(sock, p->pch);
		p->Send(sock);
		p2 = p;
		LIST_GO(p, pt);
	}
	temp = false;
	send(sock, (char *)&temp, sizeof(temp), 0);
}
template <typename TYPE>
void TreeTemplet<TYPE>::Recv(SOCKET sock)
{
	while(qnode->pch)
	{
		pnode = qnode->pch;
		Del();
	}
	PartRecv(sock, qnode);
}
template <typename TYPE>
void TreeTemplet<TYPE>::PartRecv(SOCKET sock, TYPE *p)
{
	bool temp;
	CertainRecv(sock, &temp, sizeof(temp));
	if(temp==false)
	{
		p->pch = 0;
		return;
	}
	p->pch = new TYPE;
	p->pch->ppr = p;
	LIST_GO(p, pch);
	p->qt = 0;
	PartRecv(sock, p);
	p->Recv(sock);
	CertainRecv(sock, &temp, sizeof(temp));
	while(temp)
	{
		p->pt = new TYPE;
		p->pt->qt = p;
		LIST_GO(p, pt);
		p->ppr = p->qt->ppr;
		PartRecv(sock, p);
		p->Recv(sock);
		CertainRecv(sock, &temp, sizeof(temp));
	}
	p->pt = 0;
}


//tUserData方法
tUserData::tUserData()
{
	JoinNum = 0;
	Online = false;
}
void tUserData::ReadFile(FILE *fp)
{
	int i;
	//外存变量
	ReadFileCString(fp, UserId);
	ReadFileCString(fp, Password);
	fread(&JoinNum, sizeof(JoinNum), 1, fp);
	for(i=0; i<JoinNum; i++)
	{
		ReadFileCString(fp, JoinUser[i]);
		ReadFileCString(fp, JoinMind[i]);
	}
}
void tUserData::WriteFile(FILE *fp)
{
	int i;
	//外存变量
	WriteFileCString(fp, UserId);
	WriteFileCString(fp, Password);
	fwrite(&JoinNum, sizeof(JoinNum), 1, fp);
	for(i=0; i<JoinNum; i++)
	{
		WriteFileCString(fp, JoinUser[i]);
		WriteFileCString(fp, JoinMind[i]);
	}
}


//TempletDebug
void MindListDebug()
{
	ListTemplet<MindNode> MindList;
	MindList.Add();
	MindList.Del();
	MindList.Cut();
	MindList.Paste(0);
	MindList.Clear();
	MindList.ReadFile(0);
	MindList.WriteFile(0);
	MindList.Send(0);
	MindList.Recv(0);
}
//MindNode方法
MindNode::MindNode()
{
	Length = DEFAULT_LENGTH;
	Width = DEFAULT_WIDTH;
	Pos_X = 0;
	Pos_X = 0;
	ReadOnly = true;
	ShareNum = 0;
	EditNum = 0;
	EditStatus = false;
}
void MindNode::Copy(const MindNode &Node)
{
	int i;
	//外存变量
	UserId = Node.UserId;
	MindName = Node.MindName;
	Length = Node.Length;
	Width = Node.Width;
	Pos_X = Node.Pos_X;
	Pos_Y = Node.Pos_Y;
	ReadOnly = Node.ReadOnly;
	ShareNum = Node.ShareNum;
	for(i=0; i<ShareNum; i++)
		ShareUser[i] = Node.ShareUser[i];
	EditTime = Node.EditTime;
	//内存变量
	EditNum = Node.EditNum;
	EditStatus = Node.EditStatus;
	memcpy(UserStatus, Node.UserStatus, sizeof(bool)*ShareNum);
}
void MindNode::ReadFile(FILE *fp)
{
	int i;
	//外存变量
	ReadFileCString(fp, UserId);
	ReadFileCString(fp, MindName);
	fread(&Length, sizeof(Length), 1, fp);
	fread(&Width, sizeof(Width), 1, fp);
	fread(&Pos_X, sizeof(Pos_X), 1, fp);
	fread(&Pos_Y, sizeof(Pos_Y), 1, fp);
	fread(&ReadOnly, sizeof(ReadOnly), 1, fp);
	fread(&ShareNum, sizeof(ShareNum), 1, fp);
	for(i=0; i<ShareNum; i++)
		ReadFileCString(fp, ShareUser[i]);
	ReadFileCString(fp, EditTime);
	//内存变量初始化
	EditNum = 0;
	EditStatus = false;
	memset(UserStatus, 0, sizeof(bool)*ShareNum);
}
void MindNode::WriteFile(FILE *fp)
{
	int i;
	//外存变量
	WriteFileCString(fp, UserId);
	WriteFileCString(fp, MindName);
	fwrite(&Length, sizeof(Length), 1, fp);
	fwrite(&Width, sizeof(Width), 1, fp);
	fwrite(&Pos_X, sizeof(Pos_X), 1, fp);
	fwrite(&Pos_Y, sizeof(Pos_Y), 1, fp);
	fwrite(&ReadOnly, sizeof(ReadOnly), 1, fp);
	fwrite(&ShareNum, sizeof(ShareNum), 1, fp);
	for(i=0; i<ShareNum; i++)
		WriteFileCString(fp, ShareUser[i]);
	WriteFileCString(fp, EditTime);
}
void MindNode::Send(SOCKET sock)
{
	int i;
	//外存变量
	SendCString(sock, UserId);
	SendCString(sock, MindName);
	send(sock, (char *)&Length, sizeof(Length), 0);
	send(sock, (char *)&Width, sizeof(Width), 0);
	send(sock, (char *)&Pos_X, sizeof(Pos_X), 0);
	send(sock, (char *)&Pos_Y, sizeof(Pos_Y), 0);
	send(sock, (char *)&ReadOnly, sizeof(ReadOnly), 0);
	send(sock, (char *)&ShareNum, sizeof(ShareNum), 0);
	for(i=0; i<ShareNum; i++)
		SendCString(sock, ShareUser[i]);
	SendCString(sock, EditTime);
	//内存变量初始化
	send(sock, (char *)&EditNum, sizeof(EditNum), 0);
	send(sock, (char *)&EditStatus, sizeof(EditStatus), 0);
	send(sock, (char *)UserStatus, sizeof(bool)*ShareNum, 0);
}
void MindNode::Recv(SOCKET sock)
{
	int i;
	//外存变量
	RecvCString(sock, UserId);
	RecvCString(sock, MindName);
	CertainRecv(sock, &Length, sizeof(Length));
	CertainRecv(sock, &Width, sizeof(Width));
	CertainRecv(sock, &Pos_X, sizeof(Pos_X));
	CertainRecv(sock, &Pos_Y, sizeof(Pos_Y));
	CertainRecv(sock, &ReadOnly, sizeof(ReadOnly));
	CertainRecv(sock, &ShareNum, sizeof(ShareNum));
	for(i=0; i<ShareNum; i++)
		RecvCString(sock, ShareUser[i]);
	RecvCString(sock, EditTime);
	//内存变量初始化
	CertainRecv(sock, &EditNum, sizeof(EditNum));
	CertainRecv(sock, &EditStatus, sizeof(EditStatus));
	CertainRecv(sock, UserStatus, sizeof(bool)*ShareNum);
}


//TempletDebug
void BlockNodeDebug()
{
	TreeTemplet<BlockNode> BlockList;
	BlockList.Add();
	BlockList.Del();
	BlockList.Cut();
	BlockList.Paste(0);
	BlockList.GetLocal(0);
	BlockList.SetLocal(0, 0);
	BlockList.ReadFile(0);
	BlockList.WriteFile(0);
	BlockList.Send(0);
	BlockList.Recv(0);
}
//BlockNode方法
BlockNode::BlockNode()
{
	Shape = rect;
	Dir = dirup;
	Color = RGB(255, 255, 255);
	Length = DEFAULT_BLOCK_LENGTH;
	Width = DEFAULT_BLOCK_WIDTH;
	Lock = false;
}
void BlockNode::Copy(const BlockNode &Node)
{
	PosX = Node.PosX;
	PosY = Node.PosY;
	Shape = Node.Shape;
	Dir = Node.Dir;
	Color = Node.Color;
	Length = Node.Length;
	Width = Node.Width;
	Text = Node.Text;
	Lock = Node.Lock;
	LockUser = Node.LockUser;
}
void BlockNode::ReadFile(FILE *fp)
{
	//外存变量
	fread(&PosX, sizeof(PosX), 1, fp);
	fread(&PosY, sizeof(PosY), 1, fp);
	fread(&Shape, sizeof(Shape), 1, fp);
	fread(&Dir, sizeof(Dir), 1, fp);
	fread(&Color, sizeof(Color), 1, fp);
	fread(&Length, sizeof(Length), 1, fp);
	fread(&Width, sizeof(Width), 1, fp);
	ReadFileCString(fp, Text);
	fread(&Lock, sizeof(Lock), 1, fp);
	//内存变量初始化
	LockUser = L"";
}
void BlockNode::WriteFile(FILE *fp)
{
	//外存变量
	fwrite(&PosX, sizeof(PosX), 1, fp);
	fwrite(&PosY, sizeof(PosY), 1, fp);
	fwrite(&Shape, sizeof(Shape), 1, fp);
	fwrite(&Dir, sizeof(Dir), 1, fp);
	fwrite(&Color, sizeof(Color), 1, fp);
	fwrite(&Length, sizeof(Length), 1, fp);
	fwrite(&Width, sizeof(Width), 1, fp);
	WriteFileCString(fp, Text);
	if(Lock==true && LockUser.GetLength()==0)
		fwrite(&Lock, sizeof(Lock), 1, fp);
	else
	{
		Lock = false;
		fwrite(&Lock, sizeof(Lock), 1, fp);
	}
}
void BlockNode::Send(SOCKET sock)
{
	//外存变量
	send(sock, (char *)&PosX, sizeof(PosX), 0);
	send(sock, (char *)&PosY, sizeof(PosY), 0);
	send(sock, (char *)&Shape, sizeof(Shape), 0);
	send(sock, (char *)&Dir, sizeof(Dir), 0);
	send(sock, (char *)&Color, sizeof(Color), 0);
	send(sock, (char *)&Length, sizeof(Length), 0);
	send(sock, (char *)&Width, sizeof(Width), 0);
	SendCString(sock, Text);
	send(sock, (char *)&Lock, sizeof(Lock), 0);
	//内存变量
	SendCString(sock, LockUser);
}
void BlockNode::Recv(SOCKET sock)
{
	//外存变量
	CertainRecv(sock, &PosX, sizeof(PosX));
	CertainRecv(sock, &PosY, sizeof(PosY));
	CertainRecv(sock, &Shape, sizeof(Shape));
	CertainRecv(sock, &Dir, sizeof(Dir));
	CertainRecv(sock, &Color, sizeof(Color));
	CertainRecv(sock, &Length, sizeof(Length));
	CertainRecv(sock, &Width, sizeof(Width));
	RecvCString(sock, Text);
	CertainRecv(sock, &Lock, sizeof(Lock));
	//内存变量
	RecvCString(sock, LockUser);
}


//TempletDebug
void OperateNodeDebug()
{
	ListTemplet<OperateNode> OperateList;
	OperateList.Add();
	OperateList.Del();
	OperateList.Cut();
	OperateList.Paste(0);
	OperateList.Clear();
	OperateList.Send(0);
	OperateList.Recv(0);
}
//OperateNode方法
void OperateNode::Copy(const OperateNode &Node)
{
	Operate = Node.Operate;
	switch(Operate)
	{
	case updatamind:
		MindData.Copy(Node.MindData);
		break;
	case updatablock:
		BlockData.Copy(Node.BlockData);
		break;
	case updatarelation:
		RelationData.Copy(Node.RelationData);
		break;
	}
}
void OperateNode::Send(SOCKET sock)
{
	send(sock, (char *)&Operate, sizeof(Operate), 0);
	switch(Operate)
	{
	case updatamind:
		MindData.Send(sock);
		break;
	case updatablock:
		BlockData.Send(sock);
		break;
	case updatarelation:
		RelationData.Send(sock);
		break;
	}
}
void OperateNode::Recv(SOCKET sock)
{
	CertainRecv(sock, &Operate, sizeof(Operate));
	switch(Operate)
	{
	case updatamind:
		MindData.Recv(sock);
		break;
	case updatablock:
		BlockData.Recv(sock);
		break;
	case updatarelation:
		RelationData.Recv(sock);
		break;
	}
}


//BlockOperateData方法
void BlockOperateData::Copy(const BlockOperateData &Data)
{
	Deep = Data.Deep;
	memcpy(Local, Data.Local, Deep*sizeof(int));
	Block.Copy(Data.Block);
	SetWith = Data.SetWith;
}
void BlockOperateData::Send(SOCKET sock)
{
	send(sock, (char *)&Deep, sizeof(Deep), 0);
	send(sock, (char *)Local, sizeof(int)*Deep, 0);
	Block.Send(sock);
	send(sock, (char *)&SetWith, sizeof(SetWith), 0);
}
void BlockOperateData::Recv(SOCKET sock)
{
	CertainRecv(sock, &Deep, sizeof(Deep));
	CertainRecv(sock, Local, sizeof(int)*Deep);
	Block.Recv(sock);
	CertainRecv(sock, &SetWith, sizeof(SetWith));
}


//RelationOperateData方法
void RelationOperateData::Copy(const RelationOperateData &Data)
{
	Relation = Data.Relation;
	Deep1 = Data.Deep1;
	Deep2 = Data.Deep2;
	memcpy(Local1, Data.Local1, Deep1*sizeof(int));
	memcpy(Local2, Data.Local2, Deep2*sizeof(int));
	Message = Data.Message;
}
void RelationOperateData::Send(SOCKET sock)
{
	send(sock, (char *)&Relation, sizeof(Relation), 0);
	send(sock, (char *)&Deep1, sizeof(Deep1), 0);
	send(sock, (char *)&Deep2, sizeof(Deep2), 0);
	send(sock, (char *)Local1, sizeof(int)*Deep1, 0);
	send(sock, (char *)Local2, sizeof(int)*Deep2, 0);
	send(sock, (char *)&Message, sizeof(Message), 0);
}
void RelationOperateData::Recv(SOCKET sock)
{
	CertainRecv(sock, &Relation, sizeof(Relation));
	CertainRecv(sock, &Deep1, sizeof(Deep1));
	CertainRecv(sock, &Deep2, sizeof(Deep2));
	CertainRecv(sock, Local1, sizeof(int)*Deep1);
	CertainRecv(sock, Local2, sizeof(int)*Deep2);
	CertainRecv(sock, &Message, sizeof(Message));
}


//读文件导入CString
void ReadFileCString(FILE* fp, CString &string)
{
	int len;
	static wchar_t buf[100000] = {0};
	fread(&len, sizeof(int), 1, fp);
	fread(buf, sizeof(wchar_t), len, fp);
	string = buf;
}


//CString数据写入文件
void WriteFileCString(FILE* fp, CString &string)
{
	int len = string.GetLength()+1;
	fwrite(&len, sizeof(int), 1, fp);
	fwrite(string, sizeof(wchar_t), len, fp);
}


//二分查找
//CString
bool BinarySearch(const CString string[], const CString &target, int start, int end, int &result)
{
	int mid;
	while(start<=end)
	{
		mid = (start+end)/2;
		if(string[mid]==target)
		{
			result = mid;
			return true;
		}
		else if(string[mid]>target)
			end = mid-1;
		else
			start = mid+1;
	}
	result = end;
	return false;
}
//CString、CString
bool BinarySearch(const CString string[], const CString string2[], const CString &target, 
				  const CString &target2, int start, int end, int &result)
{
	int mid;
	while(start<=end)
	{
		mid = (start+end)/2;
		if(string[mid]==target && string2[mid]==target2)
		{
			result = mid;
			return true;
		}
		else if(string[mid]>target || string[mid]==target && string2[mid]>target2)
			end = mid-1;
		else
			start = mid+1;
	}
	result = end;
	return false;
}
//tUserData
bool BinarySearch(tUserData * const string[], const CString &target, int start, int end, int &result)
{
	int mid;
	while(start<=end)
	{
		mid = (start+end)/2;
		if(string[mid]->UserId==target)
		{
			result = mid;
			return true;
		}
		else if(string[mid]->UserId>target)
			end = mid-1;
		else
			start = mid+1;
	}
	result = end;
	return false;
}


//对思维导图进行操作
bool OperateData(MindNode &TheMind, ListTemplet<class OperateNode> &OperateList)
{
	bool MindChange = false;
	while(OperateList.qnode->pt)
	{
		OperateList.pnode = OperateList.qnode->pt;
		//更新导图信息
		if(OperateList.pnode->Operate==updatamind)
		{
			TheMind.Copy(OperateList.pnode->MindData);
			MindChange = true;
		}
		//更新分块信息
		else if(OperateList.pnode->Operate==updatablock)
		{
			if(!TheMind.BlockTree.SetLocal(OperateList.pnode->BlockData.Local, 
				OperateList.pnode->BlockData.Deep))
				assert(0);
			TheMind.BlockTree.pnode->Copy(OperateList.pnode->BlockData.Block);
			bool IsLock;
			if(TheMind.BlockTree.pnode->Lock && TheMind.BlockTree.pnode->LockUser.GetLength()==0)
				IsLock = true;
			else
				IsLock = false;
			if(OperateList.pnode->BlockData.SetWith)
				LockBlockWith(TheMind.BlockTree.pnode->pch, IsLock);
		}
		//更新分块关系
		else if(OperateList.pnode->Operate==updatarelation)
		{
			TheMind.BlockTree.SetLocal(OperateList.pnode->RelationData.Local1, OperateList.pnode->RelationData.Deep1);
			OperateRelation(TheMind.BlockTree, OperateList.pnode->RelationData.Relation, 
				OperateList.pnode->RelationData.Local2, OperateList.pnode->RelationData.Deep2);
		}
		else
			assert(0);
		OperateList.pnode = OperateList.qnode;
		OperateList.Del();
	}
	return MindChange;
}


//对思维导图进行关系操作
int OperateRelation(TreeTemplet<class BlockNode> &BlockTree, enumRelation Relation, int Local[], int Deep)
{
	int Message = 0;
	BlockNode *pTemp, *pTemp2;
	switch(Relation)
	{
	//添加根
	case addroot:
		BlockTree.pnode = BlockTree.qnode;
		BlockTree.Add();
		break;
	//删除连带子
	case delwith:
		while(BlockTree.pnode->pch)
		{
			LIST_GO(BlockTree.pnode, pch);
			BlockTree.Del();
		}
		BlockTree.Del();
		break;
	//删除合并子
	case delcombine:
		Message = BlockTree.Del();
		break;
	//分块插入子
	case addchild:
		BlockTree.Add();
		break;
	//分块插入父
	case addparent:
		pTemp = BlockTree.Cut();
		BlockTree.Add();
		BlockTree.Paste(pTemp);
		break;
	//链接已有
	case lineto:
		pTemp = BlockTree.pnode;
		BlockTree.SetLocal(Local, Deep);
		pTemp2 = BlockTree.pnode;
		BlockTree.pnode = pTemp;
		BlockTree.Cut();
		BlockTree.pnode = pTemp2;
		BlockTree.Paste(pTemp);
		break;
	//分离为根
	case separateroot:
		pTemp = BlockTree.Cut();
		BlockTree.pnode = BlockTree.qnode;
		BlockTree.Paste(pTemp);
		break;
	//删除分离子
	case delseparate:
		pTemp = BlockTree.Cut();
		BlockTree.pnode = BlockTree.qnode;
		BlockTree.Paste(pTemp);
		Message = BlockTree.Del();
		break;
	default:
		assert(0);
		break;
	}
	return Message;
}


//递归设置分块锁定解锁
void LockBlockWith(BlockNode *pBlock, bool IsLock)
{
	while(pBlock)
	{
		//设置子分块
		LockBlockWith(pBlock->pch, IsLock);

		//设置自身
		if(IsLock && (pBlock->Lock && pBlock->LockUser.GetLength()!=0 || pBlock->Lock==false))
		{
			pBlock->Lock = true;
			pBlock->LockUser = L"";
		}
		else if(IsLock==false && pBlock->Lock && pBlock->LockUser.GetLength()==0)
		{
			pBlock->Lock = false;
		}

		//走向兄弟分块
		LIST_GO(pBlock, pt);
	}
}
/****************************************************************/
/*************************服务器同步结束*************************/
/****************************************************************/



//对时间进行减法
int MinusTime(const SYSTEMTIME &time1, const SYSTEMTIME &time2)
{
	int temp = 0;
	temp += time1.wMilliseconds-time2.wMilliseconds;
	temp += (time1.wSecond-time2.wSecond)*1000;
	temp += (time1.wMinute-time2.wMinute)*1000*60;
	temp += (time1.wHour-time2.wHour)*1000*60*60;
	temp += (time1.wDay-time2.wDay)*1000*60*60*24;
	while(temp<0)
		temp += 1000*60*60*24;
	return temp;
}


//时间转换字符串
void GetTime(const SYSTEMTIME &time, CString &string)
{
	string.Format(L"%02d:%02d:%02d.%03d", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
}


//时间转换详细字符串
void GetAllTime(const SYSTEMTIME &time, CString &string)
{
	string.Format(L"%4d年%02d月%02d日-%02d:%02d:%02d.%03d", time.wYear, time.wMonth, time.wDay,
		time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
}


//数据库初始化
void DataInit()
{
	FILE *fp;
	int i;
	//获取用户数据表
	if(fopen_s(&fp, "UserData.dat", "rb")==0)
	{
		fread(&g_UserNum, sizeof(g_UserNum), 1, fp);
		for(i=0; i<g_UserNum; i++)
		{
			g_pUserData[i] = new tUserData;
			g_pUserData[i]->ReadFile(fp);
			g_pUserData[i]->Online = false;
		}
		fclose(fp);
	}
	else
		g_UserNum = 0;

	//获取所有思维导图
	if(fopen_s(&fp, "MindData.dat", "rb")==0)
	{
		fread(&g_UserNum, sizeof(g_UserNum), 1, fp);
		for(i=0; i<g_UserNum; i++)
		{
			g_pUserData[i]->MindList.ReadFile(fp);
		}
		fclose(fp);
	}
}


//数据库保存
void DataSave()
{
	FILE *fp;
	int i;
	//保存思维导图
	fopen_s(&fp, "MindData.dat", "wb");
	fwrite(&g_UserNum, sizeof(int), 1, fp);
	for(i=0; i<g_UserNum; i++)
	{
		g_pUserData[i]->MindList.WriteFile(fp);
	}
	fclose(fp);

	//保存用户信息
	fopen_s(&fp, "UserData.dat", "wb");
	fwrite(&g_UserNum, sizeof(int), 1, fp);
	for(i=0; i<g_UserNum; i++)
	{
		g_pUserData[i]->WriteFile(fp);
		delete g_pUserData[i];
	}
	fclose(fp);
}


//添加用户
bool AddUser(CString &UserId, CString &Password)
{
	int result;
	int i;
	if(g_UserNum>=USER_NUM)
		return false;
	if(BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
		return false;
	for(i=g_UserNum-1; i>result; i--)
		g_pUserData[i+1] = g_pUserData[i];
	g_pUserData[result+1] = new tUserData;
	g_pUserData[result+1]->UserId = UserId;
	g_pUserData[result+1]->Password = Password;
	g_UserNum ++;
	return true;
}


//核对用户
bool TestUser(CString &UserId, CString &Password)
{
	int result;
	if(BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
	{
		if(g_pUserData[result]->Password==Password)
			return true;
		else
			return false;
	}
	else
		return false;
}


//修改用户信息
bool ChangeUser(CString &UserId, CString &Password, CString &Password2)
{
	int result;
	if(BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
	{
		if(g_pUserData[result]->Password==Password)
		{
			g_pUserData[result]->Password = Password2;
			return true;
		}
		else
			return false;
	}
	else
		return false;
}


//找到用户拥有思维导图链表
ListTemplet<MindNode> *GetOwnMindList(CString &UserId)
{
	int result;
	if(!BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
	{
		assert(0);
		return 0;
	}
	return &g_pUserData[result]->MindList;
}


//构造用户参与思维导图链表
ListTemplet<MindNode> *BuildJoinMindList(CString &UserId)
{
	int result;
	//找到我的用户信息
	if(!BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
	{
		assert(0);
		return 0;
	}
	int i;
	int start = 0;
	int result2;
	ListTemplet<MindNode> *pMindList = new ListTemplet<MindNode>;

	//循环我所参与的思维导图
	for(i=0; i<g_pUserData[result]->JoinNum; i++)
	{
		//找到思维导图的拥有者
		if(!BinarySearch(g_pUserData, g_pUserData[result]->JoinUser[i], start, g_UserNum-1, result2))
		{
			assert(0);
			continue;
		}
		//如果不和上一个是同一个拥有者，重新找
		if(!(i>0 && start==result2))
			g_pUserData[result2]->MindList.pnode = g_pUserData[result2]->MindList.qnode;

		//循环他的思维导图
		for(LIST_GO(g_pUserData[result2]->MindList.pnode, pt);
			g_pUserData[result2]->MindList.pnode; 
			LIST_GO(g_pUserData[result2]->MindList.pnode, pt))
		{
			//找到思维导图
			if(g_pUserData[result2]->MindList.pnode->MindName==g_pUserData[result]->JoinMind[i])
			{
				pMindList->Add();
				pMindList->pnode->Copy(*g_pUserData[result2]->MindList.pnode);
				break;
			}
		}
		if(g_pUserData[result2]->MindList.pnode==0)
			assert(0);
		start = result2;
	}
	return pMindList;
}


//新建思维导图
MindNode *NewMind(CString &UserId)
{
	int result;
	int i;
	//找到用户信息
	if(!BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
	{
		assert(0);
		return 0;
	}
	ListTemplet<MindNode> *pMindList = &g_pUserData[result]->MindList;
	CString MindName = L"新建思维导图";
	//找到思维导图
	for(pMindList->pnode=pMindList->qnode; pMindList->pnode->pt; LIST_GO(pMindList->pnode, pt))
	{
		if(pMindList->pnode->pt->MindName>=MindName)
			break;
	}

	//如果重名
	if(pMindList->pnode->pt && pMindList->pnode->pt->MindName==MindName)
	{
		MindName = L"新建思维导图2";
		int len = MindName.GetLength();
		//尝试加入编号
		for(i=2; ; i++)
		{
			MindName.Format(L"新建思维导图%d", i);
			if(MindName.GetLength()>len)
			{
				pMindList->pnode=pMindList->qnode;
				len = MindName.GetLength();
			}
			//重新查重
			for(; pMindList->pnode->pt; LIST_GO(pMindList->pnode, pt))
			{
				if(pMindList->pnode->pt->MindName>=MindName)
					break;
			}
			//不重名退出
			if(pMindList->pnode->pt==0 || pMindList->pnode->pt->MindName!=MindName)
				break;
		}
	}

	SYSTEMTIME time;
	GetLocalTime(&time);
	//新建赋值
	pMindList->Add();
	//外存变量
	pMindList->pnode->UserId = UserId;
	pMindList->pnode->MindName = MindName;
	GetAllTime(time, pMindList->pnode->EditTime);
	//内存变量
	pMindList->pnode->EditNum = 1;
	pMindList->pnode->EditStatus = true;

	//记录用户编辑信息
	g_pUserData[result]->Online = true;
	g_pUserData[result]->LastTime = time;
	g_pUserData[result]->pEditMind = pMindList->pnode;
	g_pUserData[result]->pLockBlock = 0;

	return pMindList->pnode;
}


//选择思维导图
MindNode *SelMind(CString &UserId, CString &MindUserId, CString &MindName)
{
	int result;
	//找到用户信息
	if(!BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
	{
		assert(0);
		return 0;
	}
	ListTemplet<MindNode> *pMindList;
	int result2, result3;
	//如果是相同用户
	if(UserId==MindUserId)
	{
		result2 = result;
	}
	//如果是不同用户
	else
	{
		//找不到参与思维导图则返回失败
		if(!BinarySearch(g_pUserData[result]->JoinUser, g_pUserData[result]->JoinMind, 
			MindUserId, MindName, 0, g_pUserData[result]->JoinNum-1, result3))
			return 0;
		//找到思维导图拥有者
		if(!BinarySearch(g_pUserData, MindUserId, 0, g_UserNum-1, result2))
		{
			assert(0);
			return 0;
		}
	}
	//找到思维导图
	pMindList = &g_pUserData[result2]->MindList;
	for(pMindList->pnode=pMindList->qnode->pt; pMindList->pnode; LIST_GO(pMindList->pnode, pt))
	{
		if(pMindList->pnode->MindName>=MindName)
			break;
	}
	if(pMindList->pnode==0 || pMindList->pnode->MindName!=MindName)
	{
		assert(0);
		return 0;
	}
	
	SYSTEMTIME time;
	GetLocalTime(&time);
	//记录用户编辑信息
	g_pUserData[result]->Online = true;
	g_pUserData[result]->LastTime = time;
	g_pUserData[result]->pEditMind = pMindList->pnode;
	g_pUserData[result]->pLockBlock = 0;

	//修改思维导图状态
	if(UserId==MindUserId)
	{
		//若为拥有者
		if(pMindList->pnode->EditStatus==false)
		{
			pMindList->pnode->EditStatus = true;
			pMindList->pnode->EditNum ++;
			//若更新则刷新操作链表
			RefreshOperateList(*pMindList->pnode, 0, true);
		}
	}
	else
	{
		//若为共享用户
		if(!BinarySearch(pMindList->pnode->ShareUser, UserId, 0, pMindList->pnode->ShareNum-1, result2))
			assert(0);
		if(pMindList->pnode->UserStatus[result2]==false)
		{
			pMindList->pnode->UserStatus[result2] = true;
			pMindList->pnode->EditNum ++;
			//若更新则刷新操作链表
			RefreshOperateList(*pMindList->pnode, 0, true);
		}
	}

	//若为第一编辑者则打开文件
	if(pMindList->pnode->EditNum==1)
	{
		CString FileName;
		FileName = MindUserId+L"_"+MindName+L".mind";
		FILE *fp;
		if(fopen_s(&fp, CT2A(FileName), "rb")==0)
		{
			pMindList->pnode->BlockTree.ReadFile(fp);
			fclose(fp);
		}
	}

	return pMindList->pnode;
}


//删除思维导图
bool DelMind(CString &UserId, CString &MindName)
{
	int result;
	int i, j;
	//找到用户信息
	if(!BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
		assert(0);
	ListTemplet<MindNode> *pMindList = &g_pUserData[result]->MindList;
	//找到思维导图
	for(pMindList->pnode=pMindList->qnode; pMindList->pnode->pt; LIST_GO(pMindList->pnode, pt))
	{
		if(pMindList->pnode->pt->MindName>=MindName)
			break;
	}

	//如果存在
	if(pMindList->pnode->pt && pMindList->pnode->pt->MindName==MindName)
	{
		//关闭所有共享用户
		MindNode *pTheMind = pMindList->pnode->pt;
		int result2, result3;
		for(i=0; i<pTheMind->ShareNum; i++)
		{
			//找到共享用户
			if(!BinarySearch(g_pUserData, pTheMind->ShareUser[i], 0, g_UserNum-1, result2))
			{
				assert(0);
				continue;
			}
			//找到参与思维导图
			if(!BinarySearch(g_pUserData[result2]->JoinUser, g_pUserData[result2]->JoinMind, 
				UserId, MindName, 0, g_pUserData[result2]->JoinNum-1, result3))
			{
				assert(0);
				continue;
			}
			//删除参与思维导图
			for(j=result3; j<g_pUserData[result2]->JoinNum-1; j++)
			{
				g_pUserData[result2]->JoinUser[j] = g_pUserData[result2]->JoinUser[j];
				g_pUserData[result2]->JoinMind[j] = g_pUserData[result2]->JoinMind[j];
			}
			g_pUserData[result2]->JoinNum --;
			//这里不再改写编辑失效，这样再次检测是可分辨是断开连接还是失去权限
		}
		//删除内存
		pMindList->Del();
		//删除文件
		CString FileName;
		FileName = UserId+L"_"+MindName+L".mind";
		remove(CT2A(FileName));
		return true;
	}
	else
	{
		assert(0);
		return false;
	}
}


//结束编辑
PacketType EditEnd(CString &UserId)
{
	int result;
	//找到用户信息
	if(!BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
		assert(0);
	//无论如何，清空该用户操作链表
	g_pUserData[result]->OperateList.Clear();
	//若编辑已失效(此图被删除，中途断开连接)，则返回
	if(g_pUserData[result]->Online==false)
		return fail;

	//结束编辑
	//更改用户编辑信息
	g_pUserData[result]->Online = false;
	MindNode *pMindNode = g_pUserData[result]->pEditMind;
	//更改思维导图编辑信息
	if(UserId==pMindNode->UserId)
	{
		pMindNode->EditNum --;
		pMindNode->EditStatus = false;
	}
	else
	{
		//若找得到该用户共享，即未失去权限
		int result2;
		if(BinarySearch(pMindNode->ShareUser, UserId, 0, pMindNode->ShareNum-1, result2))
		{
			pMindNode->EditNum --;
			pMindNode->UserStatus[result2] = false;
		}
	}
	//添加操作链表
	RefreshOperateList(*pMindNode, 0, true);
	//若用户处于编辑状态
	if(g_pUserData[result]->pLockBlock)
	{
		if(g_pUserData[result]->pLockBlock->Lock==false || g_pUserData[result]->pLockBlock->LockUser!=UserId)
			assert(0);
		//撤销分块锁定编辑
		g_pUserData[result]->pLockBlock->Lock = false;
		pMindNode->BlockTree.pnode = g_pUserData[result]->pLockBlock;
		int deep = pMindNode->BlockTree.GetLocal(g_IntBuf);
		//添加操作链表
		SYSTEMTIME time;
		GetLocalTime(&time);
		RefreshOperateList(*pMindNode, &time, deep, g_IntBuf, false);
	}

	//若此导图结束则保存
	if(pMindNode->EditNum==0)
	{
		//保存文件
		CString FileName;
		FileName = pMindNode->UserId+L"_"+pMindNode->MindName+L".mind";
		FILE *fp;
		fopen_s(&fp, CT2A(FileName), "wb");
		pMindNode->BlockTree.WriteFile(fp);
		fclose(fp);
		return success;
	}
	else
		return defaultvalue;
}


//更改思维导图名称
enum PacketType ChangeName(CString &UserId, CString &MindName)
{
	int result;
	//找到用户信息
	if(!BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
		assert(0);
	//检测编辑状态
	if(g_pUserData[result]->Online==false)
		return error;//返回断开连接
	if(g_pUserData[result]->pEditMind->UserId!=UserId)
	{
		assert(0);
		return error;
	}
	//刷新用户维持时间
	SYSTEMTIME time;
	GetLocalTime(&time);
	g_pUserData[result]->LastTime = time;

	//剪切节点，更改名字
	ListTemplet<MindNode> *pMindList = &g_pUserData[result]->MindList;
	pMindList->pnode = g_pUserData[result]->pEditMind->qt;
	pMindList->Cut();
	//查找重名
	for(pMindList->pnode=pMindList->qnode; pMindList->pnode->pt; LIST_GO(pMindList->pnode, pt))
	{
		if(pMindList->pnode->pt->MindName>=MindName)
			break;
	}
	if(pMindList->pnode->pt && pMindList->pnode->pt->MindName==MindName)
	{
		//粘贴返回失败
		pMindList->Paste(g_pUserData[result]->pEditMind);
		return fail;
	}
	else
	{
		//粘贴
		pMindList->Paste(g_pUserData[result]->pEditMind);
		//删除文件
		CString FileName;
		FileName = UserId+L"_"+g_pUserData[result]->pEditMind->MindName+L".mind";
		remove(CT2A(FileName));
		//更改共享用户的共享导图名字
		int i;
		int result2, result3;
		MindNode *pMind = g_pUserData[result]->pEditMind;
		for(i=0; i<pMind->ShareNum; i++)
		{
			if(!BinarySearch(g_pUserData, pMind->ShareUser[i], 0, g_UserNum-1, result2))
				assert(0);
			if(!BinarySearch(g_pUserData[result2]->JoinUser, g_pUserData[result2]->JoinMind, 
				pMind->UserId, pMind->MindName, 0, g_pUserData[result2]->JoinNum-1, result3))
				assert(0);
			g_pUserData[result2]->JoinMind[result3] = MindName;
		}
		//改名
		g_pUserData[result]->pEditMind->MindName = MindName;
		g_pUserData[result]->pEditMind = pMindList->pnode;
		//将此操作加入其他人的操作链表
		//除上传事件外思维导图更新无须更新拥有者操作链表
		//更改时间
		RefreshOperateList(*g_pUserData[result]->pEditMind, &time, false);
		return success;
	}
}


//添加共享用户
enum PacketType AddShare(CString &UserId, CString &ShareUserId)
{
	int result;
	//找到用户信息
	if(!BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
		assert(0);
	//检测编辑状态
	if(g_pUserData[result]->Online==false)
		return error;//返回断开连接
	if(g_pUserData[result]->pEditMind->UserId!=UserId)
	{
		assert(0);
		return error;
	}
	//刷新用户维持时间
	SYSTEMTIME time;
	GetLocalTime(&time);
	g_pUserData[result]->LastTime = time;

	int result2;
	//找到共享用户信息
	if(!BinarySearch(g_pUserData, ShareUserId, 0, g_UserNum-1, result2))
		return fail;//返回失败

	//更新思维导图共享用户列表
	int result3;
	int i;
	MindNode *pMind = g_pUserData[result]->pEditMind;
	if(BinarySearch(pMind->ShareUser, ShareUserId, 0, pMind->ShareNum-1, result3))
	{
		assert(0);
		return success;
	}
	for(i=pMind->ShareNum-1; i>result3; i--)
	{
		pMind->ShareUser[i+1] = pMind->ShareUser[i];
		pMind->UserStatus[i+1] = pMind->UserStatus[i];
	}
	pMind->ShareUser[result3+1] = ShareUserId;
	pMind->UserStatus[result3+1] = false;
	pMind->ShareNum ++;

	//更新共享用户思维导图列表
	if(BinarySearch(g_pUserData[result2]->JoinUser, g_pUserData[result2]->JoinMind, 
		UserId, pMind->MindName, 0, g_pUserData[result2]->JoinNum-1, result3))
	{
		assert(0);
		return success;
	}
	for(i=g_pUserData[result2]->JoinNum-1; i>result3; i--)
	{
		g_pUserData[result2]->JoinUser[i+1] = g_pUserData[result2]->JoinUser[i];
		g_pUserData[result2]->JoinMind[i+1] = g_pUserData[result2]->JoinMind[i];
	}
	g_pUserData[result2]->JoinUser[result3+1] = UserId;
	g_pUserData[result2]->JoinMind[result3+1] = pMind->MindName;
	g_pUserData[result2]->JoinNum ++;
	//若该用户正在编辑此导图，则Online置位false
	if(g_pUserData[result2]->Online && g_pUserData[result2]->pEditMind==pMind)
		g_pUserData[result2]->Online = false;

	//将此操作加入其他人的操作链表
	//除上传事件外思维导图更新无须更新拥有者操作链表
	//更改时间
	RefreshOperateList(*pMind, &time, false);
	return success;
}


//删除共享用户
enum PacketType DelShare(CString &UserId, CString &ShareUserId)
{
	int result;
	//找到用户信息
	if(!BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
		assert(0);
	//检测编辑状态
	if(g_pUserData[result]->Online==false)
		return error;//返回断开连接
	if(g_pUserData[result]->pEditMind->UserId!=UserId)
	{
		assert(0);
		return error;
	}
	//刷新用户维持时间
	SYSTEMTIME time;
	GetLocalTime(&time);
	g_pUserData[result]->LastTime = time;

	//更新思维导图共享用户列表
	int result3;
	int i;
	MindNode *pMind = g_pUserData[result]->pEditMind;
	if(!BinarySearch(pMind->ShareUser, ShareUserId, 0, pMind->ShareNum-1, result3))
	{
		assert(0);
		return success;
	}
	for(i=result3; i<pMind->ShareNum-1; i++)
	{
		pMind->ShareUser[i] = pMind->ShareUser[i+1];
		pMind->UserStatus[i] = pMind->UserStatus[i+1];
	}
	pMind->ShareNum --;
	
	//找到共享用户信息
	int result2;
	if(!BinarySearch(g_pUserData, ShareUserId, 0, g_UserNum-1, result2))
		assert(0);
	//更新共享用户思维导图列表
	if(!BinarySearch(g_pUserData[result2]->JoinUser, g_pUserData[result2]->JoinMind, 
		UserId, pMind->MindName, 0, g_pUserData[result2]->JoinNum-1, result3))
	{
		assert(0);
		return success;
	}
	for(i=result3; i<g_pUserData[result2]->JoinNum-1; i++)
	{
		g_pUserData[result2]->JoinUser[i] = g_pUserData[result2]->JoinUser[i+1];
		g_pUserData[result2]->JoinMind[i] = g_pUserData[result2]->JoinMind[i+1];
	}
	g_pUserData[result2]->JoinNum --;

	//若该用户正在编辑思维导图，不清理Online，清理操作链表，思维导图编辑人数减1
	if(g_pUserData[result2]->Online && g_pUserData[result2]->pEditMind==pMind)
	{
		g_pUserData[result2]->OperateList.Clear();
		pMind->EditNum --;
	}

	//将此操作加入其他人的操作链表
	//除上传事件外思维导图更新无须更新拥有者操作链表
	//更改时间
	RefreshOperateList(*pMind, &time, false);
	return success;
}


//设置分块锁定解锁
enum PacketType LockBlock(CString &UserId, int Deep, int Local[], ListTemplet<OperateNode> * &pReOperateList, 
						  bool IsLock, bool SetWith)
{
	int result;
	//找到用户信息
	if(!BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
		assert(0);
	//检测编辑状态
	if(g_pUserData[result]->Online==false)
		return error;//返回断开连接
	if(g_pUserData[result]->pEditMind->UserId!=UserId)
		assert(0);

	//刷新用户维持时间
	SYSTEMTIME time;
	GetLocalTime(&time);
	g_pUserData[result]->LastTime = time;

	MindNode *pMind = g_pUserData[result]->pEditMind;
	//找到节点
	RefreshBlockLocal(Local, Deep, g_pUserData[result]->OperateList);
	if(Deep==0)
		return fail;
	if(pMind->BlockTree.SetLocal(Local, Deep)==false)
		assert(0);
	BlockNode *pBlock = pMind->BlockTree.pnode;
	//进行操作
	if(IsLock && (pBlock->Lock && pBlock->LockUser.GetLength()!=0 || pBlock->Lock==false))
	{
		pBlock->Lock = true;
		pBlock->LockUser = L"";
	}
	else if(IsLock==false && pBlock->Lock && pBlock->LockUser.GetLength()==0)
	{
		pBlock->Lock = false;
	}
	if(SetWith)
		LockBlockWith(pBlock->pch, IsLock);

	//将此操作加入拥有者的操作链表
	//将此操作加入其他人的操作链表
	//更改时间
	RefreshOperateList(*g_pUserData[result]->pEditMind, &time, Deep, Local, SetWith);

	//返回成功和操作链表
	pReOperateList = &g_pUserData[result]->OperateList;
	return success;
}


//上传思维导图
enum PacketType UpMind(CString &UserId, MindNode &NewMind, ListTemplet<OperateNode> * &pReOperateList)
{
	int result;
	//找到用户信息
	if(!BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
		assert(0);
	//检测编辑状态
	if(g_pUserData[result]->Online==false)
		return error;//返回断开连接
	if(g_pUserData[result]->pEditMind->UserId!=UserId)
	{
		assert(0);
		return error;
	}
	//刷新用户维持时间
	SYSTEMTIME time;
	GetLocalTime(&time);
	g_pUserData[result]->LastTime = time;

	MindNode *pMind = g_pUserData[result]->pEditMind;
	//有选择的更改思维导图
	pMind->Length = NewMind.Length;//长
	if(pMind->Length<pMind->Pos_X)
		pMind->Length = pMind->Pos_X;
	pMind->Width = NewMind.Width;//宽
	if(pMind->Width<pMind->Pos_Y)
		pMind->Width = pMind->Pos_Y;
	if(pMind->Length<MIN_LENGTH)
		pMind->Length = MIN_LENGTH;
	if(pMind->Width<MIN_WIDTH)
		pMind->Width = MIN_WIDTH;
	pMind->ReadOnly = NewMind.ReadOnly;//只读

	//将此操作加入拥有者的操作链表
	//将此操作加入其他人的操作链表
	//更改时间
	RefreshOperateList(*g_pUserData[result]->pEditMind, &time, true);

	//返回成功和操作链表
	pReOperateList = &g_pUserData[result]->OperateList;
	return success;
}


//保持思维导图
enum PacketType KeepMind(CString &UserId, ListTemplet<OperateNode> * &pReOperateList)
{
	int result;
	//找到用户信息
	if(!BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
		assert(0);
	//检测编辑状态
	if(g_pUserData[result]->Online==false)
		return error;//返回断开连接
	if(g_pUserData[result]->pEditMind->UserId!=UserId)
	{
		assert(0);
		return error;
	}
	//刷新用户维持时间
	SYSTEMTIME time;
	GetLocalTime(&time);
	g_pUserData[result]->LastTime = time;
	
	//返回成功和操作链表
	pReOperateList = &g_pUserData[result]->OperateList;
	return success;
}


//下载思维导图
enum PacketType DownMind(CString &UserId, ListTemplet<OperateNode> * &pReOperateList)
{
	int result;
	//找到用户信息
	if(!BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
		assert(0);
	//检测编辑状态
	if(g_pUserData[result]->Online==false)
		return error;//返回断开连接
	MindNode *pMind = g_pUserData[result]->pEditMind;
	int result2;
	if(!BinarySearch(pMind->ShareUser, UserId, 0, pMind->ShareNum-1, result2))
		return nopermission;//返回没有权限

	//刷新用户维持时间
	SYSTEMTIME time;
	GetLocalTime(&time);
	g_pUserData[result]->LastTime = time;

	//刷新编辑状态
	if(g_pUserData[result]->pLockBlock)
	{
		BlockNode *pBlock = g_pUserData[result]->pLockBlock;
		if(pBlock->Lock==false)
		{
			assert(0);
			return error;
		}
		if(pBlock->LockUser.GetLength()==0)//分块锁定则返回失败
		{
			//使其退出状态返回
			int deep;
			pMind->BlockTree.pnode = pBlock;
			deep = pMind->BlockTree.GetLocal(g_IntBuf);
			RefreshOperateList(*pMind, 0, deep, g_IntBuf, false);
			g_pUserData[result]->pLockBlock = 0;
			pReOperateList = &g_pUserData[result]->OperateList;
			return fail;
		}
		if(pMind->ReadOnly)//思维导图只读则返回失败
		{
			//使其退出状态返回
			pBlock->Lock = false;
			pBlock->LockUser = L"";
			int deep;
			pMind->BlockTree.pnode = pBlock;
			deep = pMind->BlockTree.GetLocal(g_IntBuf);
			RefreshOperateList(*pMind, 0, deep, g_IntBuf, false);
			g_pUserData[result]->pLockBlock = 0;
			pReOperateList = &g_pUserData[result]->OperateList;
			return fail;
		}
	}

	//返回成功和操作链表
	pReOperateList = &g_pUserData[result]->OperateList;
	return success;
}


//申请分块
enum PacketType ApplyBlock(CString &UserId, int Local[], int Deep, ListTemplet<OperateNode> * &pReOperateList)
{
	int result;
	//找到用户信息
	if(!BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
		assert(0);
	//检测编辑状态
	if(g_pUserData[result]->Online==false)
		return error;//返回断开连接
	MindNode *pMind = g_pUserData[result]->pEditMind;
	//若非拥有者
	if(UserId!=pMind->UserId)
	{
		int result2;
		if(!BinarySearch(pMind->ShareUser, UserId, 0, pMind->ShareNum-1, result2))
			return nopermission;//返回没有权限
	}

	//刷新用户维持时间
	SYSTEMTIME time;
	GetLocalTime(&time);
	g_pUserData[result]->LastTime = time;

	//根据操作转换定位信息
	RefreshBlockLocal(Local, Deep, g_pUserData[result]->OperateList);
	if(Deep==0)
		return fail;//若定位丢失则返回失败

	//查看权限
	if(pMind->BlockTree.SetLocal(Local, Deep)==false)
		assert(0);
	BlockNode *pBlock = pMind->BlockTree.pnode;
	if(UserId==pMind->UserId)
	{
		//拥有者情况
		if(pBlock->Lock && pBlock->LockUser.GetLength()>0)//分块被他人锁定则返回失败
			return fail;
	}
	else
	{
		//共享用户情况
		if(pMind->ReadOnly)//思维导图只读则返回失败
			return fail;
		if(pBlock->Lock)//分块锁定则返回失败
			return fail;
	}

	//设置用户属性
	if(g_pUserData[result]->pLockBlock
		&& g_pUserData[result]->pLockBlock->Lock && g_pUserData[result]->pLockBlock->LockUser==UserId)
		g_pUserData[result]->pLockBlock->Lock = false;
	g_pUserData[result]->pLockBlock = pBlock;
	//设置节点属性
	if(pBlock->Lock==false)
	{
		//为可更改分块
		pBlock->Lock = true;
		pBlock->LockUser = UserId;
	}//否则为不可更改分块，无须更改其锁定状态

	//将此操作加入拥有者的操作链表
	//将此操作加入其他人的操作链表
	RefreshOperateList(*pMind, 0, Deep, Local, false);

	//返回成功和操作链表
	pReOperateList = &g_pUserData[result]->OperateList;
	return success;
}


//释放分块
enum PacketType FinishBlock(CString &UserId, BlockNode &NewBlock, ListTemplet<OperateNode> * &pReOperateList)
{
	int result;
	//找到用户信息
	if(!BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
		assert(0);
	//检测编辑状态
	if(g_pUserData[result]->Online==false)
		return error;//返回断开连接
	MindNode *pMind = g_pUserData[result]->pEditMind;
	//若非拥有者
	if(UserId!=pMind->UserId)
	{
		int result2;
		if(!BinarySearch(pMind->ShareUser, UserId, 0, pMind->ShareNum-1, result2))
			return nopermission;//返回没有权限
	}

	//刷新用户维持时间
	SYSTEMTIME time;
	GetLocalTime(&time);
	g_pUserData[result]->LastTime = time;

	//查看权限
	BlockNode *pBlock = g_pUserData[result]->pLockBlock;
	if(pBlock==0)
	{
		assert(0);
		return error;
	}
	if(UserId==pMind->UserId)
	{
		//拥有者情况
		if(pBlock->Lock && pBlock->LockUser.GetLength()>0 && pBlock->LockUser!=UserId)
		{
			assert(0);
			return error;
		}
	}
	else
	{
		//共享用户情况
		if(pBlock->Lock==false)//分块未被锁定（被拥有者禁止编辑后解锁）返回失败
		{
			NewBlock.Copy(*pBlock);//拷贝返回
			g_pUserData[result]->pEditMind = 0;
			return fail;
		}
		if(pBlock->LockUser.GetLength()==0)//分块锁定则返回失败
		{
			NewBlock.Copy(*pBlock);//拷贝返回
			g_pUserData[result]->pEditMind = 0;
			return fail;
		}
		if(pMind->ReadOnly)//思维导图只读则返回失败
		{
			pBlock->Lock = false;
			pBlock->LockUser = L"";
			NewBlock.Copy(*pBlock);//拷贝返回
			g_pUserData[result]->pEditMind = 0;
			return fail;
		}
	}

	//设置用户属性
	g_pUserData[result]->pLockBlock = 0;
	//设置节点属性
	if(pBlock->LockUser.GetLength()>0)
	{
		//为可更改分块
		pBlock->Lock = false;
		pBlock->LockUser = L"";
	}//否则为不可更改分块，无须更改其锁定状态

	//有选择的更新分块信息
	pBlock->Shape = NewBlock.Shape;
	pBlock->Dir = NewBlock.Dir;
	pBlock->Color = NewBlock.Color;
	pBlock->Text = NewBlock.Text;
	pBlock->PosX = NewBlock.PosX;
	pBlock->PosY = NewBlock.PosY;
	pBlock->Length = NewBlock.Length;
	pBlock->Width = NewBlock.Width;
	//长宽位置逻辑，使得不超过导图大小和位置限制
	if(pBlock->Length<MIN_BLOCK_LENGTH)
		pBlock->Length = MIN_BLOCK_LENGTH;
	else if(pBlock->Length>pMind->Length)
		pBlock->Length = pMind->Length;
	if(pBlock->Width<MIN_BLOCK_WIDTH)
		pBlock->Width = MIN_BLOCK_WIDTH;
	else if(pBlock->Width>pMind->Width)
		pBlock->Width = pMind->Width;
	if(pBlock->PosX<0)
		pBlock->PosX = 0;
	else if(pBlock->PosX>pMind->Length-pBlock->Length)
	 pBlock->PosX = pMind->Length-pBlock->Length;
	if(pBlock->PosY<0)
		pBlock->PosY = 0;
	else if(pBlock->PosY>pMind->Width-pBlock->Width)
	 pBlock->PosY = pMind->Width-pBlock->Width;
	//更新导图属性
	pMind->Pos_X = pMind->Pos_Y = 0;
	RefreshMaxPos(pMind->BlockTree.qnode->pch, pMind->Pos_X, pMind->Pos_Y);

	//将此操作加入拥有者的操作链表
	//将此操作加入其他人的操作链表
	RefreshOperateList(*g_pUserData[result]->pEditMind, &time, true);
	pMind->BlockTree.pnode = pBlock;
	int deep = pMind->BlockTree.GetLocal(g_IntBuf);
	RefreshOperateList(*pMind, &time, deep, g_IntBuf, false);

	//返回成功和操作链表
	pReOperateList = &g_pUserData[result]->OperateList;
	return success;
}


//添加根分块
enum PacketType BlockAddRoot(CString &UserId, int x, int y, ListTemplet<OperateNode> * &pReOperateList)
{
	int result;
	//找到用户信息
	if(!BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
		assert(0);
	//检测编辑状态
	if(g_pUserData[result]->Online==false)
		return error;//返回断开连接
	if(g_pUserData[result]->pEditMind->UserId!=UserId)
		assert(0);

	//刷新用户维持时间
	SYSTEMTIME time;
	GetLocalTime(&time);
	g_pUserData[result]->LastTime = time;

	MindNode *pMind = g_pUserData[result]->pEditMind;
	//添加节点
	OperateRelation(pMind->BlockTree, addroot, 0, 0);
	BlockNode *pBlock = pMind->BlockTree.pnode;
	//判断节点大小
	if(pMind->Length<pBlock->Length || pMind->Width<pBlock->Width)
	{
		//大小不满足则撤销并返回失败
		pMind->BlockTree.Del();
		return fail;
	}
	if(x<0)
		x = 0;
	else if(x>pMind->Length-pBlock->Length)
		x = pMind->Length-pBlock->Length;
	if(y<0)
		y = 0;
	else if(y>pMind->Width-pBlock->Width)
		y = pMind->Width-pBlock->Width;

	//设置用户属性
	if(g_pUserData[result]->pLockBlock
		&& g_pUserData[result]->pLockBlock->Lock && g_pUserData[result]->pLockBlock->LockUser==UserId)
		g_pUserData[result]->pLockBlock->Lock = false;
	g_pUserData[result]->pLockBlock = pBlock;
	//设置节点属性
	pBlock->PosX = x;
	pBlock->PosY = y;
	pBlock->Lock = true;
	pBlock->LockUser = UserId;
	//更新导图属性
	pMind->Pos_X = pMind->Pos_Y = 0;
	RefreshMaxPos(pMind->BlockTree.qnode->pch, pMind->Pos_X, pMind->Pos_Y);

	//将此操作加入拥有者的操作链表
	//将此操作加入其他人的操作链表
	//更改时间
	RefreshOperateList(*g_pUserData[result]->pEditMind, &time, true);
	RefreshOperateList(*g_pUserData[result]->pEditMind, &time, addroot, 0, 0, 0, 0, 0);
	int deep = pMind->BlockTree.GetLocal(g_IntBuf);
	RefreshOperateList(*g_pUserData[result]->pEditMind, &time, deep, g_IntBuf, false);

	//返回成功和操作链表
	pReOperateList = &g_pUserData[result]->OperateList;
	return success;
}


//删除连带子分块
enum PacketType BlockDelWith(CString &UserId, BlockNode &NewBlock, ListTemplet<OperateNode> * &pReOperateList)
{
	int result;
	//找到用户信息
	if(!BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
		assert(0);
	//检测编辑状态
	if(g_pUserData[result]->Online==false)
		return error;//返回断开连接
	MindNode *pMind = g_pUserData[result]->pEditMind;
	//若非拥有者
	if(UserId!=pMind->UserId)
	{
		int result2;
		if(!BinarySearch(pMind->ShareUser, UserId, 0, pMind->ShareNum-1, result2))
			return nopermission;//返回没有权限
	}

	//刷新用户维持时间
	SYSTEMTIME time;
	GetLocalTime(&time);
	g_pUserData[result]->LastTime = time;

	//查看权限
	BlockNode *pBlock = g_pUserData[result]->pLockBlock;
	if(pBlock==0)
	{
		assert(0);
		return fail;
	}
	if(UserId==pMind->UserId)
	{
		if(pBlock->Lock && pBlock->LockUser.GetLength()>0 && pBlock->LockUser!=UserId)
			assert(0);
	}
	else
	{
		//共享用户情况
		if(pBlock->Lock==false)//分块未被锁定（被拥有者禁止编辑后解锁）返回失败
		{
			NewBlock.Copy(*pBlock);//拷贝返回
			g_pUserData[result]->pEditMind = 0;
			return fail;
		}
		if(pBlock->LockUser.GetLength()==0)//分块锁定则返回失败
		{
			NewBlock.Copy(*pBlock);//拷贝返回
			g_pUserData[result]->pEditMind = 0;
			return fail;
		}
		if(pMind->ReadOnly)//思维导图只读则返回失败
		{
			pBlock->Lock = false;
			pBlock->LockUser = L"";
			NewBlock.Copy(*pBlock);//拷贝返回
			g_pUserData[result]->pEditMind = 0;
			return fail;
		}
	}

	//若子节点被锁定则返回冲突
	if(JudgeBlockLock(pBlock->pch, UserId==pMind->UserId))
	{
		return clash;
	}
	
	pMind->BlockTree.pnode = pBlock;
	int deep = pMind->BlockTree.GetLocal(g_IntBuf);
	//进行操作
	pMind->BlockTree.pnode = pBlock;
	OperateRelation(pMind->BlockTree, delwith, 0, 0);
	//设置用户属性
	g_pUserData[result]->pLockBlock = 0;
	//更新导图属性
	pMind->Pos_X = pMind->Pos_Y = 0;
	RefreshMaxPos(pMind->BlockTree.qnode->pch, pMind->Pos_X, pMind->Pos_Y);

	//将此操作加入拥有者的操作链表
	//将此操作加入其他人的操作链表
	//更改时间
	RefreshOperateList(*g_pUserData[result]->pEditMind, &time, true);
	RefreshOperateList(*pMind, &time, delwith, deep, g_IntBuf, 0, 0, 0);

	//返回成功和操作链表
	pReOperateList = &g_pUserData[result]->OperateList;
	return success;
}


//删除合并子分块
enum PacketType BlockDelCombine(CString &UserId, BlockNode &NewBlock, ListTemplet<OperateNode> * &pReOperateList)
{
	int result;
	//找到用户信息
	if(!BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
		assert(0);
	//检测编辑状态
	if(g_pUserData[result]->Online==false)
		return error;//返回断开连接
	MindNode *pMind = g_pUserData[result]->pEditMind;
	//若非拥有者
	if(UserId!=pMind->UserId)
	{
		int result2;
		if(!BinarySearch(pMind->ShareUser, UserId, 0, pMind->ShareNum-1, result2))
			return nopermission;//返回没有权限
	}

	//刷新用户维持时间
	SYSTEMTIME time;
	GetLocalTime(&time);
	g_pUserData[result]->LastTime = time;

	//查看权限
	BlockNode *pBlock = g_pUserData[result]->pLockBlock;
	if(pBlock==0)
	{
		assert(0);
		return fail;
	}
	if(UserId==pMind->UserId)
	{
		//拥有者情况
		if(pBlock->Lock && pBlock->LockUser.GetLength()>0 && pBlock->LockUser!=UserId)
			assert(0);
	}
	else
	{
		//共享用户情况
		if(pBlock->Lock==false)//分块未被锁定（被拥有者禁止编辑后解锁）返回失败
		{
			NewBlock.Copy(*pBlock);//拷贝返回
			g_pUserData[result]->pEditMind = 0;
			return fail;
		}
		if(pBlock->LockUser.GetLength()==0)//分块锁定则返回失败
		{
			NewBlock.Copy(*pBlock);//拷贝返回
			g_pUserData[result]->pEditMind = 0;
			return fail;
		}
		if(pMind->ReadOnly)//思维导图只读则返回失败
		{
			pBlock->Lock = false;
			pBlock->LockUser = L"";
			NewBlock.Copy(*pBlock);//拷贝返回
			g_pUserData[result]->pEditMind = 0;
			return fail;
		}
	}

	//不会冲突
	
	pMind->BlockTree.pnode = pBlock;
	int deep = pMind->BlockTree.GetLocal(g_IntBuf);
	//进行操作
	pMind->BlockTree.pnode = pBlock;
	int Message = OperateRelation(pMind->BlockTree, delcombine, 0, 0);
	//设置用户属性
	g_pUserData[result]->pLockBlock = 0;
	//更新导图属性
	pMind->Pos_X = pMind->Pos_Y = 0;
	RefreshMaxPos(pMind->BlockTree.qnode->pch, pMind->Pos_X, pMind->Pos_Y);

	//将此操作加入拥有者的操作链表
	//将此操作加入其他人的操作链表
	//更改时间
	RefreshOperateList(*g_pUserData[result]->pEditMind, &time, true);
	RefreshOperateList(*pMind, &time, delcombine, deep, g_IntBuf, 0, 0, Message);

	//返回成功和操作链表
	pReOperateList = &g_pUserData[result]->OperateList;
	return success;
}


//添加子分块
enum PacketType BlockAddChild(CString &UserId, BlockNode &NewBlock, ListTemplet<OperateNode> * &pReOperateList, 
							  int x, int y)
{
	int result;
	//找到用户信息
	if(!BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
		assert(0);
	//检测编辑状态
	if(g_pUserData[result]->Online==false)
		return error;//返回断开连接
	MindNode *pMind = g_pUserData[result]->pEditMind;
	//若非拥有者
	if(UserId!=pMind->UserId)
	{
		int result2;
		if(!BinarySearch(pMind->ShareUser, UserId, 0, pMind->ShareNum-1, result2))
			return nopermission;//返回没有权限
	}

	//刷新用户维持时间
	SYSTEMTIME time;
	GetLocalTime(&time);
	g_pUserData[result]->LastTime = time;

	//查看权限
	BlockNode *pBlock = g_pUserData[result]->pLockBlock;
	if(pBlock==0)
	{
		assert(0);
		return fail;
	}
	if(UserId==pMind->UserId)
	{
		//拥有者情况
		if(pBlock->Lock && pBlock->LockUser.GetLength()>0 && pBlock->LockUser!=UserId)
			assert(0);
	}
	else
	{
		//共享用户情况
		if(pBlock->Lock==false)//分块未被锁定（被拥有者禁止编辑后解锁）返回失败
		{
			NewBlock.Copy(*pBlock);//拷贝返回
			g_pUserData[result]->pEditMind = 0;
			return fail;
		}
		if(pBlock->LockUser.GetLength()==0)//分块锁定则返回失败
		{
			NewBlock.Copy(*pBlock);//拷贝返回
			g_pUserData[result]->pEditMind = 0;
			return fail;
		}
		if(pMind->ReadOnly)//思维导图只读则返回失败
		{
			pBlock->Lock = false;
			pBlock->LockUser = L"";
			NewBlock.Copy(*pBlock);//拷贝返回
			g_pUserData[result]->pEditMind = 0;
			return fail;
		}
	}

	//不会冲突
	
	pMind->BlockTree.pnode = pBlock;
	int deep = pMind->BlockTree.GetLocal(g_IntBuf);
	//进行操作
	pMind->BlockTree.pnode = pBlock;
	OperateRelation(pMind->BlockTree, addchild, g_IntBuf, deep);
	//设置节点属性
	LIST_GO(pBlock, pch);
	srand(time.wMilliseconds);
	int randnum = rand()%4;
	switch(randnum)
	{
	case 0:
		x += DEFAULT_BLOCK_LENGTH*2;
		y += rand()%(DEFAULT_BLOCK_WIDTH*4)-DEFAULT_BLOCK_WIDTH*2;
		break;
	case 1:
		x += -DEFAULT_BLOCK_LENGTH*2;
		y += rand()%(DEFAULT_BLOCK_WIDTH*4)-DEFAULT_BLOCK_WIDTH*2;
		break;
	case 2:
		y += DEFAULT_BLOCK_WIDTH*2;
		x += rand()%(DEFAULT_BLOCK_LENGTH*4)-DEFAULT_BLOCK_LENGTH*2;
		break;
	case 3:
		y += DEFAULT_BLOCK_WIDTH*2;
		x += rand()%(DEFAULT_BLOCK_LENGTH*4)-DEFAULT_BLOCK_LENGTH*2;
		break;
	}
	if(x<0)
		x = 0;
	else if(x>pMind->Length-pBlock->Length)
		x = pMind->Length-pBlock->Length;
	if(y<0)
		y = 0;
	else if(y>pMind->Width-pBlock->Width)
		y = pMind->Width-pBlock->Width;
	pBlock->PosX = x;
	pBlock->PosY = y;
	pBlock->Lock = false;
	//设置用户属性不改变
	//更新导图属性
	pMind->Pos_X = pMind->Pos_Y = 0;
	RefreshMaxPos(pMind->BlockTree.qnode->pch, pMind->Pos_X, pMind->Pos_Y);

	//将此操作加入拥有者的操作链表
	//将此操作加入其他人的操作链表
	//更改时间
	RefreshOperateList(*g_pUserData[result]->pEditMind, &time, true);
	RefreshOperateList(*pMind, &time, addchild, deep, g_IntBuf, 0, 0, 0);
	pMind->BlockTree.pnode = pBlock;
	deep = pMind->BlockTree.GetLocal(g_IntBuf);
	RefreshOperateList(*pMind, &time, deep, g_IntBuf, false);

	//返回成功和操作链表
	pReOperateList = &g_pUserData[result]->OperateList;
	return success;
}


//插入父分块
enum PacketType BlockAddParent(CString &UserId, BlockNode &NewBlock, ListTemplet<OperateNode> * &pReOperateList, 
							   int x, int y)
{
	int result;
	//找到用户信息
	if(!BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
		assert(0);
	//检测编辑状态
	if(g_pUserData[result]->Online==false)
		return error;//返回断开连接
	MindNode *pMind = g_pUserData[result]->pEditMind;
	//若非拥有者
	if(UserId!=pMind->UserId)
	{
		int result2;
		if(!BinarySearch(pMind->ShareUser, UserId, 0, pMind->ShareNum-1, result2))
			return nopermission;//返回没有权限
	}

	//刷新用户维持时间
	SYSTEMTIME time;
	GetLocalTime(&time);
	g_pUserData[result]->LastTime = time;

	//查看权限
	BlockNode *pBlock = g_pUserData[result]->pLockBlock;
	if(pBlock==0)
	{
		assert(0);
		return fail;
	}
	if(UserId==pMind->UserId)
	{
		//拥有者情况
		if(pBlock->Lock && pBlock->LockUser.GetLength()>0 && pBlock->LockUser!=UserId)
			assert(0);
	}
	else
	{
		//共享用户情况
		if(pBlock->Lock==false)//分块未被锁定（被拥有者禁止编辑后解锁）返回失败
		{
			NewBlock.Copy(*pBlock);//拷贝返回
			g_pUserData[result]->pEditMind = 0;
			return fail;
		}
		if(pBlock->LockUser.GetLength()==0)//分块锁定则返回失败
		{
			NewBlock.Copy(*pBlock);//拷贝返回
			g_pUserData[result]->pEditMind = 0;
			return fail;
		}
		if(pMind->ReadOnly)//思维导图只读则返回失败
		{
			pBlock->Lock = false;
			pBlock->LockUser = L"";
			NewBlock.Copy(*pBlock);//拷贝返回
			g_pUserData[result]->pEditMind = 0;
			return fail;
		}
	}

	//不会冲突

	pMind->BlockTree.pnode = pBlock;
	int deep = pMind->BlockTree.GetLocal(g_IntBuf);
	//进行操作
	pMind->BlockTree.pnode = pBlock;
	OperateRelation(pMind->BlockTree, addparent, g_IntBuf, deep);
	//设置节点属性
	LIST_GO(pBlock, ppr);
	if(pBlock->ppr->ppr)
	{
		x = (x+pBlock->ppr->PosX)/2;
		y = (y+pBlock->ppr->PosY)/2;
	}
	else
	{
		srand(time.wMilliseconds);
		int randnum = rand()%4;
		switch(randnum)
		{
		case 0:
			x += DEFAULT_BLOCK_LENGTH*2;
			y += rand()%(DEFAULT_BLOCK_WIDTH*4)-DEFAULT_BLOCK_WIDTH*2;
			break;
		case 1:
			x += -DEFAULT_BLOCK_LENGTH*2;
			y += rand()%(DEFAULT_BLOCK_WIDTH*4)-DEFAULT_BLOCK_WIDTH*2;
			break;
		case 2:
			y += DEFAULT_BLOCK_WIDTH*2;
			x += rand()%(DEFAULT_BLOCK_LENGTH*4)-DEFAULT_BLOCK_LENGTH*2;
			break;
		case 3:
			y += DEFAULT_BLOCK_WIDTH*2;
			x += rand()%(DEFAULT_BLOCK_LENGTH*4)-DEFAULT_BLOCK_LENGTH*2;
			break;
		}
	}
	if(x<0)
		x = 0;
	else if(x>pMind->Length-pBlock->Length)
		x = pMind->Length-pBlock->Length;
	if(y<0)
		y = 0;
	else if(y>pMind->Width-pBlock->Width)
		y = pMind->Width-pBlock->Width;
	pBlock->PosX = x;
	pBlock->PosY = y;
	pBlock->Lock = false;
	//设置用户属性不改变
	//更新导图属性
	pMind->Pos_X = pMind->Pos_Y = 0;
	RefreshMaxPos(pMind->BlockTree.qnode->pch, pMind->Pos_X, pMind->Pos_Y);

	//将此操作加入拥有者的操作链表
	//将此操作加入其他人的操作链表
	//更改时间
	RefreshOperateList(*g_pUserData[result]->pEditMind, &time, true);
	RefreshOperateList(*pMind, &time, addparent, deep, g_IntBuf, 0, 0, 0);
	pMind->BlockTree.pnode = pBlock;
	deep = pMind->BlockTree.GetLocal(g_IntBuf);
	RefreshOperateList(*pMind, &time, deep, g_IntBuf, false);

	//返回成功和操作链表
	pReOperateList = &g_pUserData[result]->OperateList;
	return success;
}


//连接到已有分块
enum PacketType BlockLineInto(CString &UserId, BlockNode &NewBlock, ListTemplet<OperateNode> * &pReOperateList, 
							  int Deep2, int Local2[])
{
	int result;
	//找到用户信息
	if(!BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
		assert(0);
	//检测编辑状态
	if(g_pUserData[result]->Online==false)
		return error;//返回断开连接
	MindNode *pMind = g_pUserData[result]->pEditMind;
	//若非拥有者
	if(UserId!=pMind->UserId)
	{
		int result2;
		if(!BinarySearch(pMind->ShareUser, UserId, 0, pMind->ShareNum-1, result2))
			return nopermission;//返回没有权限
	}

	//刷新用户维持时间
	SYSTEMTIME time;
	GetLocalTime(&time);
	g_pUserData[result]->LastTime = time;

	//查看权限
	BlockNode *pBlock = g_pUserData[result]->pLockBlock;
	if(pBlock==0)
	{
		assert(0);
		return fail;
	}
	if(UserId==pMind->UserId)
	{
		//拥有者情况
		if(pBlock->Lock && pBlock->LockUser.GetLength()>0 && pBlock->LockUser!=UserId)
			assert(0);
	}
	else
	{
		//共享用户情况
		if(pBlock->Lock==false)//分块未被锁定（被拥有者禁止编辑后解锁）返回失败
		{
			NewBlock.Copy(*pBlock);//拷贝返回
			g_pUserData[result]->pEditMind = 0;
			return fail;
		}
		if(pBlock->LockUser.GetLength()==0)//分块锁定则返回失败
		{
			NewBlock.Copy(*pBlock);//拷贝返回
			g_pUserData[result]->pEditMind = 0;
			return fail;
		}
		if(pMind->ReadOnly)//思维导图只读则返回失败
		{
			pBlock->Lock = false;
			pBlock->LockUser = L"";
			NewBlock.Copy(*pBlock);//拷贝返回
			g_pUserData[result]->pEditMind = 0;
			return fail;
		}
	}

	//冲突检测
	RefreshBlockLocal(Local2, Deep2, g_pUserData[result]->OperateList);
	if(Deep2==0)
		return clash;
	if(!pMind->BlockTree.SetLocal(Local2, Deep2))
		assert(0);
	if(UserId!=pMind->UserId && pMind->BlockTree.pnode->Lock && pMind->BlockTree.pnode->LockUser.GetLength()==0)
		return clash;
	pMind->BlockTree.pnode = pBlock;
	int deep = pMind->BlockTree.GetLocal(g_IntBuf);
	//找到共通路径
	int i;
	for(i=1; i<=deep&&i<=Deep2; i++)
	{
		if(g_IntBuf[deep-i]!=Local2[Deep2-i])
			break;
	}
	//删除没了，定位在删除或下方
	if(i>deep)
	{
		return clash;
	}
	
	//进行操作
	pMind->BlockTree.pnode = pBlock;
	OperateRelation(pMind->BlockTree, lineto, Local2, Deep2);
	//设置用户属性不改变
	//更新导图属性不改变

	//将此操作加入拥有者的操作链表
	//将此操作加入其他人的操作链表
	//更改时间
	RefreshOperateList(*g_pUserData[result]->pEditMind, &time, lineto, deep, g_IntBuf, Deep2, Local2, 0);

	//返回成功和操作链表
	pReOperateList = &g_pUserData[result]->OperateList;
	return success;
}


//分离为根分块
enum PacketType BlockSeparateRoot(CString &UserId, BlockNode &NewBlock, ListTemplet<OperateNode> * &pReOperateList)
{
	int result;
	//找到用户信息
	if(!BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
		assert(0);
	//检测编辑状态
	if(g_pUserData[result]->Online==false)
		return error;//返回断开连接
	MindNode *pMind = g_pUserData[result]->pEditMind;
	//若非拥有者
	if(UserId!=pMind->UserId)
	{
		assert(0);
		return error;
	}

	//刷新用户维持时间
	SYSTEMTIME time;
	GetLocalTime(&time);
	g_pUserData[result]->LastTime = time;

	//查看权限
	BlockNode *pBlock = g_pUserData[result]->pLockBlock;
	if(pBlock==0)
	{
		assert(0);
		return fail;
	}
	if(UserId==pMind->UserId)
	{
		//拥有者情况
		if(pBlock->Lock && pBlock->LockUser.GetLength()>0 && pBlock->LockUser!=UserId)
			assert(0);
	}

	//不会冲突
	
	pMind->BlockTree.pnode = pBlock;
	int deep = pMind->BlockTree.GetLocal(g_IntBuf);
	//进行操作
	pMind->BlockTree.pnode = pBlock;
	OperateRelation(pMind->BlockTree, separateroot, 0, 0);
	//设置用户属性不会改变
	//更新导图属性不会改变

	//将此操作加入拥有者的操作链表
	//将此操作加入其他人的操作链表
	//更改时间
	RefreshOperateList(*g_pUserData[result]->pEditMind, &time, lineto, deep, g_IntBuf, 0, 0, 0);

	//返回成功和操作链表
	pReOperateList = &g_pUserData[result]->OperateList;
	return success;
}


//删除分离子分块
enum PacketType BlockDelSeparate(CString &UserId, BlockNode &NewBlock, ListTemplet<OperateNode> * &pReOperateList)
{
	int result;
	//找到用户信息
	if(!BinarySearch(g_pUserData, UserId, 0, g_UserNum-1, result))
		assert(0);
	//检测编辑状态
	if(g_pUserData[result]->Online==false)
		return error;//返回断开连接
	MindNode *pMind = g_pUserData[result]->pEditMind;
	//若非拥有者
	if(UserId!=pMind->UserId)
	{
		assert(0);
		return error;
	}

	//刷新用户维持时间
	SYSTEMTIME time;
	GetLocalTime(&time);
	g_pUserData[result]->LastTime = time;

	//查看权限
	BlockNode *pBlock = g_pUserData[result]->pLockBlock;
	if(pBlock==0)
	{
		assert(0);
		return fail;
	}
	if(UserId==pMind->UserId)
	{
		//拥有者情况
		if(pBlock->Lock && pBlock->LockUser.GetLength()>0 && pBlock->LockUser!=UserId)
			assert(0);
	}

	//不会冲突
	
	pMind->BlockTree.pnode = pBlock;
	int deep = pMind->BlockTree.GetLocal(g_IntBuf);
	//进行操作
	pMind->BlockTree.pnode = pBlock;
	int Message = OperateRelation(pMind->BlockTree, delseparate, g_IntBuf, deep);
	//设置用户属性
	g_pUserData[result]->pLockBlock = 0;
	//更新导图属性
	pMind->Pos_X = pMind->Pos_Y = 0;
	RefreshMaxPos(pMind->BlockTree.qnode->pch, pMind->Pos_X, pMind->Pos_Y);

	//将此操作加入拥有者的操作链表
	//将此操作加入其他人的操作链表
	//更改时间
	RefreshOperateList(*g_pUserData[result]->pEditMind, &time, true);
	RefreshOperateList(*pMind, &time, delseparate, deep, g_IntBuf, 0, 0, Message);

	//返回成功和操作链表
	pReOperateList = &g_pUserData[result]->OperateList;
	return success;
}


//更新操作链表，修改导图编辑时间
//mind更新
void RefreshOperateList(MindNode &TheMind, SYSTEMTIME *ptime, bool OwnUser)
{
	//更新编辑时间
	if(ptime)
		GetAllTime(*ptime, TheMind.EditTime);

	ListTemplet<class OperateNode> *pOperateList;
	//选择更新拥有者
	if(OwnUser)
	{
		int result;
		//找到拥有者用户信息
		if(!BinarySearch(g_pUserData, TheMind.UserId, 0, g_UserNum-1, result))
			assert(0);
		//将此操作加入拥有者的操作链表
		if(g_pUserData[result]->Online)
		{
			//链表尾部添加新操作
			pOperateList = &g_pUserData[result]->OperateList;
			pOperateList->pnode = pOperateList->ptail;
			pOperateList->Add();
			//操作赋值
			pOperateList->pnode->Operate = updatamind;
			pOperateList->pnode->MindData.Copy(TheMind);
		}
	}
	//将此操作加入其他人的操作链表
	int i;
	int result2;
	for(i=0; i<TheMind.ShareNum; i++)
	{
		//如果共享用户在编辑
		if(TheMind.UserStatus[i])
		{
			//找到用户信息
			if(!BinarySearch(g_pUserData, TheMind.ShareUser[i], 0, g_UserNum-1, result2))
				assert(0);
			//链表尾部添加新操作
			pOperateList = &g_pUserData[result2]->OperateList;
			pOperateList->pnode = pOperateList->ptail;
			pOperateList->Add();
			//操作赋值
			pOperateList->pnode->Operate = updatamind;
			pOperateList->pnode->MindData.Copy(TheMind);
		}
	}
}
//Bolck更新
void RefreshOperateList(MindNode &TheMind, SYSTEMTIME *ptime, int Deep, int Local[], bool SetWith)
{
	//更新编辑时间
	if(ptime)
		GetAllTime(*ptime, TheMind.EditTime);
	ListTemplet<class OperateNode> *pOperateList;

	//选择更新拥有者
	int result;
	//找到拥有者用户信息
	if(!BinarySearch(g_pUserData, TheMind.UserId, 0, g_UserNum-1, result))
		assert(0);
	//记录节点指针
	TheMind.BlockTree.SetLocal(Local, Deep);
	BlockNode *pBlock = TheMind.BlockTree.pnode;
	//将此操作加入拥有者的操作链表
	if(TheMind.EditStatus)
	{
		//链表尾部添加新操作
		pOperateList = &g_pUserData[result]->OperateList;
		pOperateList->pnode = pOperateList->ptail;
		pOperateList->Add();
		//操作赋值
		pOperateList->pnode->Operate = updatablock;
		pOperateList->pnode->BlockData.Deep = Deep;
		memcpy(pOperateList->pnode->BlockData.Local, Local, sizeof(int)*Deep);
		pOperateList->pnode->BlockData.Block.Copy(*pBlock);
		pOperateList->pnode->BlockData.SetWith = SetWith;
	}

	//将此操作加入其他人的操作链表
	int i;
	int result2;
	for(i=0; i<TheMind.ShareNum; i++)
	{
		//如果共享用户在编辑
		if(TheMind.UserStatus[i])
		{
			//找到用户信息
			if(!BinarySearch(g_pUserData, TheMind.ShareUser[i], 0, g_UserNum-1, result2))
				assert(0);
			//链表尾部添加新操作
			pOperateList = &g_pUserData[result2]->OperateList;
			pOperateList->pnode = pOperateList->ptail;
			pOperateList->Add();
			//操作赋值
			pOperateList->pnode->Operate = updatablock;
			pOperateList->pnode->BlockData.Deep = Deep;
			memcpy(pOperateList->pnode->BlockData.Local, Local, sizeof(int)*Deep);
			pOperateList->pnode->BlockData.Block.Copy(*pBlock);
			pOperateList->pnode->BlockData.SetWith = SetWith;
		}
	}
}
//Relation更新
void RefreshOperateList(MindNode &TheMind, SYSTEMTIME *ptime, enumRelation Relation,  
						int Deep1, int Local1[], int Deep2, int Local2[], int Message)
{
	//更新编辑时间
	if(ptime)
		GetAllTime(*ptime, TheMind.EditTime);
	ListTemplet<class OperateNode> *pOperateList;

	//选择更新拥有者
	int result;
	//找到拥有者用户信息
	if(!BinarySearch(g_pUserData, TheMind.UserId, 0, g_UserNum-1, result))
		assert(0);
	//将此操作加入拥有者的操作链表
	if(TheMind.EditStatus)
	{
		//链表尾部添加新操作
		pOperateList = &g_pUserData[result]->OperateList;
		pOperateList->pnode = pOperateList->ptail;
		pOperateList->Add();
		//操作赋值
		pOperateList->pnode->Operate = updatarelation;
		pOperateList->pnode->RelationData.Relation = Relation;
		pOperateList->pnode->RelationData.Deep1 = Deep1;
		memcpy(pOperateList->pnode->RelationData.Local1, Local1, sizeof(int)*Deep1);
		pOperateList->pnode->RelationData.Deep2 = Deep2;
		memcpy(pOperateList->pnode->RelationData.Local2, Local2, sizeof(int)*Deep2);
		pOperateList->pnode->RelationData.Message = Message;
	}

	//将此操作加入其他人的操作链表
	int i;
	int result2;
	for(i=0; i<TheMind.ShareNum; i++)
	{
		//如果共享用户在编辑
		if(TheMind.UserStatus[i])
		{
			//找到用户信息
			if(!BinarySearch(g_pUserData, TheMind.ShareUser[i], 0, g_UserNum-1, result2))
				assert(0);
			//链表尾部添加新操作
			pOperateList = &g_pUserData[result2]->OperateList;
			pOperateList->pnode = pOperateList->ptail;
			pOperateList->Add();
			//操作赋值
			pOperateList->pnode->Operate = updatarelation;
			pOperateList->pnode->RelationData.Relation = Relation;
			pOperateList->pnode->RelationData.Deep1 = Deep1;
			memcpy(pOperateList->pnode->RelationData.Local1, Local1, sizeof(int)*Deep1);
			pOperateList->pnode->RelationData.Deep2 = Deep2;
			memcpy(pOperateList->pnode->RelationData.Local2, Local2, sizeof(int)*Deep2);
		pOperateList->pnode->RelationData.Message = Message;
		}
	}
}


//刷新定位信息
void RefreshBlockLocal(int Local[], int &Deep, ListTemplet<OperateNode> &OperateList)
{
	//遍历链表
	for(OperateList.pnode=OperateList.qnode->pt; OperateList.pnode; LIST_GO(OperateList.pnode, pt))
	{
		if(OperateList.pnode->Operate==updatarelation)
		{
			int i, j;
			int temp;
			RelationOperateData *pRelationData = &OperateList.pnode->RelationData;
			switch (pRelationData->Relation)
			{
			//添加根
			case addroot:
				Local[Deep-1] ++;
				break;
			//删除连带
			case delwith:
				//找到共通路径
				for(i=1; i<=Deep&&i<=pRelationData->Deep1; i++)
				{
					if(pRelationData->Local1[pRelationData->Deep1-i]!=Local[Deep-i])
						break;
				}
				//删除没了，定位在删除或下方
				if(i>pRelationData->Deep1)
				{
					Deep = 0;
					return;
				}
				//定位还有而删除恰好剩一个，定位在删除右方
				else if(i<=Deep && i==pRelationData->Deep1
					&& Local[Deep-i]>pRelationData->Local1[pRelationData->Deep1-i])
				{
					Local[Deep-i] --;
				}
				break;
			//删除合并
			case delcombine:
				//找到共通路径
				for(i=1; i<=Deep&&i<=pRelationData->Deep1; i++)
				{
					if(pRelationData->Local1[pRelationData->Deep1-i]!=Local[Deep-i])
						break;
				}
				//删除没了，定位也没了，删除等于定位
				if(i>pRelationData->Deep1 && i>Deep)
				{
					Deep = 0;
					return;
				}
				//删除没了，定位还有，定位在删除下方
				if(i>pRelationData->Deep1)
				{
					for(i--; i>1; i--)
						Local[Deep-i] = Local[Deep-i+1];
					Deep --;
				}
				//定位还有而删除恰好剩一个，定位在删除右方
				else if(i<=Deep && i==pRelationData->Deep1)
				{
					if(Local[Deep-i]>pRelationData->Local1[pRelationData->Deep1-i])
						Local[Deep-i] --;
					Local[Deep-i] += pRelationData->Message;
				}
				break;
			//分块插入子
			case addchild:
				//找到共通路径
				for(i=1; i<=Deep&&i<=pRelationData->Deep1; i++)
				{
					if(pRelationData->Local1[pRelationData->Deep1-i]!=Local[Deep-i])
						break;
				}
				//插入没了，定位还有，定位在插入下方
				if(i>pRelationData->Deep1 && i<=Deep)
				{
					Local[Deep-i] ++;
				}
				break;
			//分块插入父
			case addparent:
				//找到共通路径
				for(i=1; i<=Deep&&i<=pRelationData->Deep1; i++)
				{
					if(pRelationData->Local1[pRelationData->Deep1-i]!=Local[Deep-i])
						break;
				}
				//插入没了，定位在插入或下方
				if(i>pRelationData->Deep1)
				{
					for(j=Deep-1; j>Deep-i; j--)
						Local[j+1] = Local[j];
					Local[Deep-i+1] = 0;
					Local[Deep-i+2] = 0;
					Deep ++;
				}
				//定位还有而插入恰好剩一个，定位在插入左方
				else if(i<=Deep && i==pRelationData->Deep1
					&& Local[Deep-i]<pRelationData->Local1[pRelationData->Deep1-i])
				{
					Local[Deep-i] ++;
				}
				break;
			//链接已有
			case lineto:
				//找到共通路径
				for(i=1; i<=Deep&&i<=pRelationData->Deep1; i++)
				{
					if(pRelationData->Local1[pRelationData->Deep1-i]!=Local[Deep-i])
						break;
				}
				//剪切没了，定位在剪切或下方
				if(i>pRelationData->Deep1)
				{
					Local[Deep-i+1] = 0;
					for(j=0; j<pRelationData->Deep2; j++)
						Local[Deep-i+2+j] = pRelationData->Local2[j];
					Deep = Deep-i+2+pRelationData->Deep2;
					//找到剪切点和连接点的共通路径
					for(i=1; i<=pRelationData->Deep1&&i<=pRelationData->Deep2; i++)
					{
						if(pRelationData->Local2[pRelationData->Deep2-i]
							!=pRelationData->Local1[pRelationData->Deep1-i])
							break;
					}
					//连接还有而剪切恰好剩一个，连接在剪切右方
					if(i<=pRelationData->Deep2 && i==pRelationData->Deep1
						&& (pRelationData->Local2[pRelationData->Deep2-i]>
						pRelationData->Local1[pRelationData->Deep1-i]))
					{
						Local[Deep-i] --;
					}
				}
				else
				{
					temp = -1;
					//定位还有而剪切恰好剩一个，定位在剪切右方
					if(i<=Deep && i==pRelationData->Deep1
						&& Local[Deep-i]>pRelationData->Local1[pRelationData->Deep1-i])
					{
						temp = Deep-i;
					}
					//找到连接点的共通路径
					for(i=1; i<=Deep&&i<=pRelationData->Deep2; i++)
					{
						if(pRelationData->Local2[pRelationData->Deep2-i]!=Local[Deep-i])
							break;
					}
					//连接没了，定位还有，定位在连接下方
					if(i>pRelationData->Deep2 && i<=Deep)
					{
						Local[Deep-i] ++;
					}
					//处理剪切影响
					if(temp>=0)
						Local[temp] --;
				}
				break;
			//分离为根
			case separateroot:
				//找到共通路径
				for(i=1; i<=Deep&&i<=pRelationData->Deep1; i++)
				{
					if(pRelationData->Local1[pRelationData->Deep1-i]!=Local[Deep-i])
						break;
				}
				//剪切没了，定位在剪切或下方
				if(i>pRelationData->Deep1)
				{
					Local[Deep-i+1] = 0;
					Deep = Deep-i+2;
				}
				else
				{
					//定位还有而剪切恰好剩一个，定位在剪切右方
					if(i<=Deep && i==pRelationData->Deep1
						&& Local[Deep-i]>pRelationData->Local1[pRelationData->Deep1-i])
					{
						Local[Deep-i] --;
					}
					Local[Deep-1] ++;
				}
				break;
			//删除分离子
			case delseparate:
				//找到共通路径
				for(i=1; i<=Deep&&i<=pRelationData->Deep1; i++)
				{
					if(pRelationData->Local1[pRelationData->Deep1-i]!=Local[Deep-i])
						break;
				}
				//删除没了，定位也没了，删除等于定位
				if(i>pRelationData->Deep1 && i>Deep)
				{
					Deep = 0;
					return;
				}
				//删除没了，定位还有，定位在删除下方
				if(i>pRelationData->Deep1)
				{
					Deep = Deep-i+1;
				}
				else
				{
					//定位还有而删除恰好剩一个，定位在剪切右方
					if(i<=Deep && i==pRelationData->Deep1
						&& Local[Deep-i]>pRelationData->Local1[pRelationData->Deep1-i])
					{
						Local[Deep-i] --;
					}
					Local[Deep-1] += pRelationData->Message;
				}
				break;
			default:
				assert(0);
				break;
			}
		}
	}
}


//递归刷新分块位置最大值
void RefreshMaxPos(BlockNode *pBlock, int &Pos_X, int &Pos_Y)
{
	while(pBlock)
	{
		//判断子分块
		RefreshMaxPos(pBlock->pch, Pos_X, Pos_Y);

		//判断自身
		if(pBlock->PosX+pBlock->Length>Pos_X)
			Pos_X = pBlock->PosX+pBlock->Length;
		if(pBlock->PosY+pBlock->Width>Pos_Y)
			Pos_Y = pBlock->PosY+pBlock->Width;

		//走向兄弟分块
		LIST_GO(pBlock, pt);
	}
}


//递归判断是否被锁定
bool JudgeBlockLock(BlockNode *pBlock, bool Own)
{
	bool judge = false;
	while(pBlock)
	{
		//判断子分块
		if(JudgeBlockLock(pBlock->pch, Own))
			judge = true;

		//判断自身
		if(Own)
		{
			if(pBlock->Lock && pBlock->LockUser.GetLength()>0)
				judge = true;
		}
		else
		{
			if(pBlock->Lock)
				judge = true;
		}
		//走向兄弟分块
		LIST_GO(pBlock, pt);
	}
	return judge;
}

