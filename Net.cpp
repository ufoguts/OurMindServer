
// 网络线程
//

#include "stdafx.h"
#include "OurMindServer.h"
#include "OurMindServerDlg.h"



SOCKET g_WelcomeSocket;//服务器socket句柄

static int g_IntBuf[10000];//整形缓存数组



/****************************************************************/
/*************************服务器同步开始*************************/
/****************************************************************/
//保证接收长度函数
void CertainRecv(SOCKET sock, void *buf, int len)
{
	static char buf2[100000];
	int num = 0;
	int temp;
	while(num<len)
	{
		temp = recv(sock, buf2+num, len-num, 0);
		if(temp>0)
			num += temp;
	}
	memcpy(buf, buf2, len);
}


//发送CString
void SendCString(SOCKET sock, CString &string)
{
	int len = string.GetLength()+1;
	send(sock, (char *)&len, sizeof(int), 0);
	send(sock, (char *)(LPCWSTR)string, sizeof(wchar_t)*len, 0);//用户名
}


//接收CString
void RecvCString(SOCKET sock, CString &string)
{
	static wchar_t buf[100000] = {0};
	int len;
	CertainRecv(sock, &len, sizeof(int));
	CertainRecv(sock, buf, sizeof(wchar_t)*len);
	string = buf;
}
/****************************************************************/
/*************************服务器同步结束*************************/
/****************************************************************/


//网络初始化函数
void ConnectInit()
{
    WSADATA wsaData;
    int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
    if ( iResult != NO_ERROR )
        assert(0);

    //create server's socket打开socket，是一个句柄
    g_WelcomeSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    if ( g_WelcomeSocket == INVALID_SOCKET ) {
        //printf( "Error at socket(): %ld\n", WSAGetLastError() );
		assert(0);
        WSACleanup();
    }

    //bind server socket with the port and ip address设置自己的ip端口，即命名socket
    sockaddr_in service;


    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons(atoi(SERVER_PORT));

    if(bind(g_WelcomeSocket, (SOCKADDR*)&service, sizeof(service))==SOCKET_ERROR)
	{
        //printf( "bind() failed.\n" );
		assert(0);
        closesocket(g_WelcomeSocket);
        return;
    }

    //listening socket binded with port准备建立关联，监听连接
    if ( listen( g_WelcomeSocket, 10 ) == SOCKET_ERROR )
        //printf( "Error listening on socket.\n");
		assert(0);
}


//网络线程回调函数
UINT NetThreadFunc(LPVOID pParam)
{
	//初始化数据库
	DataInit();

	//初始化网络
	ConnectInit();

	while(1)//循环建立连接断开连接
	{
		SOCKET ConnectSocket=SOCKET_ERROR;

		while (ConnectSocket==SOCKET_ERROR) //等待连接
		{
			ConnectSocket = accept(g_WelcomeSocket, NULL, NULL);//接受连接：我的socket句柄，返回对方socket句柄
		}

		//接收数据包
		PacketType type;
		CertainRecv(ConnectSocket, &type, sizeof(type));
		//判断数据包类型
		switch(type)
		{
			//用户信息操作
		case hello:
			RespondHello(ConnectSocket);
			break;
		case login:
			RespondLogin(ConnectSocket);
			break;
		case regist:
			RespondRegist(ConnectSocket);
			break;
		case change:
			RespondChange(ConnectSocket);
			break;
		case getlist:
			RespondGetList(ConnectSocket);
			break;
		case newmind:
			RespondNewMind(ConnectSocket);
			break;
		case selmind:
			RespondSelMind(ConnectSocket);
			break;
		case delmind:
			RespondDelMind(ConnectSocket);
			break;
		case editend:
			RespondEditEnd(ConnectSocket);
			break;

			//导图编辑操作
		case changename:
			RespondChangeName(ConnectSocket);
			break;
		case upmind:
			RespondUpMind(ConnectSocket);
			break;
		case keepmind:
			RespondKeepMind(ConnectSocket);
			break;
		case downmind:
			RespondDownMind(ConnectSocket);
			break;
		case addshare:
			RespondAddShare(ConnectSocket);
			break;
		case delshare:
			RespondDelShare(ConnectSocket);
			break;
		case lockblock:
			RespondLockBlock(ConnectSocket);
			break;
		case applyblock:
			RespondApplyBlock(ConnectSocket);
			break;
		case finishblock:
			RespondFinishBlock(ConnectSocket);
			break;
		case blockaddroot:
			RespondBlockAddRoot(ConnectSocket);
			break;
		case blockdelwith:
			RespondBlockDelWith(ConnectSocket);
			break;
		case blockdelcombine:
			RespondBlockDelCombine(ConnectSocket);
			break;
		case blockaddchild:
			RespondBlockAddChild(ConnectSocket);
			break;
		case blockaddparent:
			RespondBlockAddParent(ConnectSocket);
			break;
		case blocklineto:
			RespondBlockLineInto(ConnectSocket);
			break;
		case blockseparateroot:
			RespondBlockSeparateRoot(ConnectSocket);
			break;
		case blockdelseparate:
			RespondBlockDelSeparate(ConnectSocket);
			break;

		default:
			assert(0);
			break;
		}
		//断开连接
		closesocket(ConnectSocket);
	}

	return 0;
}


//响应hello
void RespondHello(SOCKET sock)
{
	//发送
	PacketType type;
	type = success;
	send(sock, (char *)&type, sizeof(type), 0);//标志
	//调试信息
	ShowResiond(L"hello", L"", defaultvalue);
}


//响应login
void RespondLogin(SOCKET sock)
{
	//接收
	CString UserId, Password;
	RecvCString(sock, UserId);//用户名
	RecvCString(sock, Password);//密码
	//处理
	PacketType type;
	if(TestUser(UserId, Password))
		type = success;
	else
		type = fail;
	//发送
	send(sock, (char *)&type, sizeof(type), 0);//标志
	//调试信息
	ShowResiond(L"login", UserId, type);
}


//响应regist
void RespondRegist(SOCKET sock)
{
	//接收
	CString UserId, Password;
	RecvCString(sock, UserId);//用户名
	RecvCString(sock, Password);//密码
	//处理
	PacketType type;
	if(AddUser(UserId, Password))
		type = success;
	else
		type = fail;
	//发送
	send(sock, (char *)&type, sizeof(type), 0);//标志
	//调试信息
	ShowResiond(L"regist", UserId, type);
}


//响应change
void RespondChange(SOCKET sock)
{
	//接收
	CString UserId, Password, Password2;
	RecvCString(sock, UserId);//用户名
	RecvCString(sock, Password);//密码
	RecvCString(sock, Password2);//新密码
	//处理
	PacketType type;
	if(ChangeUser(UserId, Password, Password2))
		type = success;
	else
		type = fail;
	//发送
	send(sock, (char *)&type, sizeof(type), 0);//标志
	//调试信息
	ShowResiond(L"change", UserId, type);
}


//响应getlist
void RespondGetList(SOCKET sock)
{
	//接收
	CString UserId;
	RecvCString(sock, UserId);//用户名
	//调用EditEnd
	PacketType type;
	type = EditEnd(UserId);
	//调试信息
	ShowResiond(L"(editend)", UserId, type);
	//正常处理getlist
	//处理
	ListTemplet<MindNode> *pMindList;
	pMindList = GetOwnMindList(UserId);
	//发送
	pMindList->Send(sock);//拥有思维导图
	//处理
	pMindList = BuildJoinMindList(UserId);
	//发送
	pMindList->Send(sock);//参与思维导图
	delete pMindList;
	//调试信息
	ShowResiond(L"getlist", UserId, defaultvalue);
}


//响应newmind
void RespondNewMind(SOCKET sock)
{
	//接收
	CString UserId;
	RecvCString(sock, UserId);//用户名
	//处理
	MindNode *pTheMind = NewMind(UserId);
	//发送
	pTheMind->Send(sock);//新思维导图
	pTheMind->BlockTree.Send(sock);//新思维导图分块树
	//调试信息
	ShowResiond(L"newmind", UserId, defaultvalue);
}


//响应selmind
void RespondSelMind(SOCKET sock)
{
	//接收
	CString UserId, MindUserId, MindName;
	RecvCString(sock, UserId);//用户名
	RecvCString(sock, MindUserId);//思维导图拥有者
	RecvCString(sock, MindName);//思维导图名称
	//处理
	MindNode *pTheMind = SelMind(UserId, MindUserId, MindName);
	PacketType type;
	if(pTheMind)
	{
		//发送
		type = success;
		send(sock, (char *)&type, sizeof(type), 0);//标志
		pTheMind->Send(sock);//思维导图
		pTheMind->BlockTree.Send(sock);//思维导图分块树
	}
	else
	{
		//发送
		send(sock, (char *)&type, sizeof(type), 0);//标志
		type = fail;
	}
	//调试信息
	ShowResiond(L"selmind", UserId, defaultvalue);
}


//响应delmind
void RespondDelMind(SOCKET sock)
{
	//接收
	CString UserId, MindName;
	RecvCString(sock, UserId);//用户名
	RecvCString(sock, MindName);//思维导图名称
	//处理
	PacketType type;
	if(DelMind(UserId, MindName))
		type = defaultvalue;
	else
		type = fail;
	//调试信息
	ShowResiond(L"delmind", UserId, type);
}


//响应editend
void RespondEditEnd(SOCKET sock)
{
	//接收
	CString UserId;
	RecvCString(sock, UserId);//用户名
	//处理
	PacketType type;
	type = EditEnd(UserId);
	//调试信息
	ShowResiond(L"editend", UserId, type);
}


//响应changename
void RespondChangeName(SOCKET sock)
{
	//接收
	CString UserId, MindName;
	RecvCString(sock, UserId);//用户名
	RecvCString(sock, MindName);//思维导图新名称
	//处理
	PacketType type = ChangeName(UserId, MindName);
	//发送
	send(sock, (char *)&type, sizeof(type), 0);//标志
	//调试信息
	ShowResiond(L"changename", UserId, type);
}


//响应addshare
void RespondAddShare(SOCKET sock)
{
	//接收
	CString UserId, ShareUserId;
	RecvCString(sock, UserId);//用户名
	RecvCString(sock, ShareUserId);//共享用户名称
	//处理
	PacketType type = AddShare(UserId, ShareUserId);
	//发送
	send(sock, (char *)&type, sizeof(type), 0);//标志
	//调试信息
	ShowResiond(L"addshare", UserId, type);
}


//响应delshare
void RespondDelShare(SOCKET sock)
{
	//接收
	CString UserId, ShareUserId;
	RecvCString(sock, UserId);//用户名
	RecvCString(sock, ShareUserId);//共享用户名称
	//处理
	PacketType type = DelShare(UserId, ShareUserId);
	//发送
	send(sock, (char *)&type, sizeof(type), 0);//标志
	//调试信息
	ShowResiond(L"delshare", UserId, type);
}


//响应lockblock
void RespondLockBlock(SOCKET sock)
{
	//接收
	CString UserId;
	int deep;
	bool IsLock, SetWith;
	RecvCString(sock, UserId);//用户名
	CertainRecv(sock, &deep, sizeof(deep));//定位个数
	CertainRecv(sock, g_IntBuf, sizeof(int)*deep);//定位数组
	CertainRecv(sock, &IsLock, sizeof(IsLock));//是否锁定
	CertainRecv(sock, &SetWith, sizeof(SetWith));//设置连带
	//处理
	ListTemplet<OperateNode> *pOperateList;
	PacketType type = LockBlock(UserId, deep, g_IntBuf, pOperateList, IsLock, SetWith);
	//发送
	send(sock, (char *)&type, sizeof(type), 0);//标志
	if(type==success)
	{
		pOperateList->Send(sock);//操作链表
		pOperateList->Clear();//清空操作链表
	}
	//调试信息
	ShowResiond(L"lockblock", UserId, type);
}


//响应upmind
void RespondUpMind(SOCKET sock)
{
	//接收
	CString UserId;
	MindNode TheMind;
	RecvCString(sock, UserId);//用户名
	TheMind.Recv(sock);//思维导图
	//处理
	ListTemplet<OperateNode> *pOperateList;
	PacketType type = UpMind(UserId, TheMind, pOperateList);
	//发送
	send(sock, (char *)&type, sizeof(type), 0);//标志
	if(type==success)
	{
		pOperateList->Send(sock);//操作链表
		//调试信息
		if(pOperateList->num>0)
			ShowResiond(L"upmind", UserId, type);
		else
		{
			//可注释
			ShowResiond(L"upmind", UserId, defaultvalue);
		}
		pOperateList->Clear();//清空操作链表
	}
	else
	{
		//调试信息
		ShowResiond(L"upmind", UserId, type);
	}
}


//响应keepmind
void RespondKeepMind(SOCKET sock)
{
	//接收
	CString UserId;
	RecvCString(sock, UserId);//用户名
	//处理
	ListTemplet<OperateNode> *pOperateList;
	PacketType type = KeepMind(UserId, pOperateList);
	//发送
	send(sock, (char *)&type, sizeof(type), 0);//标志
	if(type==success)
	{
		pOperateList->Send(sock);//操作链表
		//调试信息
		if(pOperateList->num>0)
			ShowResiond(L"keepmind", UserId, type);
		else
		{
			//可注释
			ShowResiond(L"keepmind", UserId, defaultvalue);
		}
		pOperateList->Clear();//清空操作链表
	}
	else
	{
		//调试信息
		ShowResiond(L"keepmind", UserId, type);
	}
}


//响应downmind
void RespondDownMind(SOCKET sock)
{
	//接收
	CString UserId;
	RecvCString(sock, UserId);//用户名
	//处理
	ListTemplet<OperateNode> *pOperateList;
	PacketType type = DownMind(UserId, pOperateList);
	//发送
	send(sock, (char *)&type, sizeof(type), 0);//标志
	if(type==success || type==fail)
	{
		pOperateList->Send(sock);//操作链表
		//调试信息
		if(pOperateList->num>0)
			ShowResiond(L"downmind", UserId, type);
		else
		{
			//可注释
			ShowResiond(L"downmind", UserId, defaultvalue);
		}
		pOperateList->Clear();//清空操作链表
	}
	else
	{
		//调试信息
		ShowResiond(L"downmind", UserId, type);
	}
}


//响应applyblock
void RespondApplyBlock(SOCKET sock)
{
	//接收
	CString UserId;
	int Deep;
	RecvCString(sock, UserId);//用户名
	CertainRecv(sock, &Deep, sizeof(Deep));//深度
	CertainRecv(sock, g_IntBuf, sizeof(int)*Deep);//定位数组
	//处理
	ListTemplet<OperateNode> *pOperateList;
	PacketType type = ApplyBlock(UserId, g_IntBuf, Deep, pOperateList);
	//发送
	send(sock, (char *)&type, sizeof(type), 0);//标志
	if(type==success)
	{
		pOperateList->Send(sock);//操作链表
		//调试信息
		ShowResiond(L"applyblock", UserId, type);
		pOperateList->Clear();//清空操作链表
	}
	else
	{
		//调试信息
		ShowResiond(L"applyblock", UserId, type);
	}
}


//响应finishblock
void RespondFinishBlock(SOCKET sock)
{
	//接收
	CString UserId;
	BlockNode TheBlock;
	RecvCString(sock, UserId);//用户名
	TheBlock.Recv(sock);//分块数据
	//处理
	ListTemplet<OperateNode> *pOperateList;
	PacketType type = FinishBlock(UserId, TheBlock, pOperateList);
	//发送
	send(sock, (char *)&type, sizeof(type), 0);//标志
	if(type==success)
	{
		pOperateList->Send(sock);//操作链表
		//调试信息
		ShowResiond(L"finishblock", UserId, type);
		pOperateList->Clear();//清空操作链表
	}
	else if(type==fail)
	{
		TheBlock.Send(sock);//分块数据
		ShowResiond(L"finishblock", UserId, type);
	}
	else
	{
		//调试信息
		ShowResiond(L"finishblock", UserId, type);
	}
}


//响应blockaddroot
void RespondBlockAddRoot(SOCKET sock)
{
	//接收
	CString UserId;
	int x, y;
	RecvCString(sock, UserId);//用户名
	CertainRecv(sock, &x, sizeof(x));//坐标x
	CertainRecv(sock, &y, sizeof(y));//坐标y
	//处理
	ListTemplet<OperateNode> *pOperateList;
	PacketType type = BlockAddRoot(UserId, x, y, pOperateList);
	//发送
	send(sock, (char *)&type, sizeof(type), 0);//标志
	if(type==success || type==fail)
	{
		pOperateList->Send(sock);//操作链表
		//调试信息
		ShowResiond(L"blockaddroot", UserId, type);
		pOperateList->Clear();//清空操作链表
	}
	else
	{
		//调试信息
		ShowResiond(L"blockaddroot", UserId, type);
	}
}


//响应blockdelwith
void RespondBlockDelWith(SOCKET sock)
{
	//接收
	CString UserId;
	BlockNode TheBlock;
	RecvCString(sock, UserId);//用户名
	//处理
	ListTemplet<OperateNode> *pOperateList;
	PacketType type = BlockDelWith(UserId, TheBlock, pOperateList);
	//发送
	send(sock, (char *)&type, sizeof(type), 0);//标志
	if(type==success)
	{
		pOperateList->Send(sock);//操作链表
		pOperateList->Clear();//清空操作链表
	}
	else if(type==fail)
	{
		TheBlock.Send(sock);//分块数据
	}
	//调试信息
	ShowResiond(L"blockdelwith", UserId, type);
}


//响应blockdelcombine
void RespondBlockDelCombine(SOCKET sock)
{
	//接收
	CString UserId;
	BlockNode TheBlock;
	RecvCString(sock, UserId);//用户名
	//处理
	ListTemplet<OperateNode> *pOperateList;
	PacketType type = BlockDelCombine(UserId, TheBlock, pOperateList);
	//发送
	send(sock, (char *)&type, sizeof(type), 0);//标志
	if(type==success)
	{
		pOperateList->Send(sock);//操作链表
		pOperateList->Clear();//清空操作链表
	}
	else if(type==fail)
	{
		TheBlock.Send(sock);//分块数据
	}
	//调试信息
	ShowResiond(L"blockdelcombine", UserId, type);
}


//响应blockaddchild
void RespondBlockAddChild(SOCKET sock)
{
	//接收
	CString UserId;
	BlockNode TheBlock;
	int x, y;
	RecvCString(sock, UserId);//用户名
	CertainRecv(sock, &x, sizeof(x));//坐标x
	CertainRecv(sock, &y, sizeof(y));//坐标y
	//处理
	ListTemplet<OperateNode> *pOperateList;
	PacketType type = BlockAddChild(UserId, TheBlock, pOperateList, x, y);
	//发送
	send(sock, (char *)&type, sizeof(type), 0);//标志
	if(type==success)
	{
		pOperateList->Send(sock);//操作链表
		pOperateList->Clear();//清空操作链表
	}
	else if(type==fail)
	{
		TheBlock.Send(sock);//分块数据
	}
	//调试信息
	ShowResiond(L"blockaddchild", UserId, type);
}


//响应blockaddparent
void RespondBlockAddParent(SOCKET sock)
{
	//接收
	CString UserId;
	BlockNode TheBlock;
	int x, y;
	RecvCString(sock, UserId);//用户名
	CertainRecv(sock, &x, sizeof(x));//坐标x
	CertainRecv(sock, &y, sizeof(y));//坐标y
	//处理
	ListTemplet<OperateNode> *pOperateList;
	PacketType type = BlockAddParent(UserId, TheBlock, pOperateList, x, y);
	//发送
	send(sock, (char *)&type, sizeof(type), 0);//标志
	if(type==success)
	{
		pOperateList->Send(sock);//操作链表
		pOperateList->Clear();//清空操作链表
	}
	else if(type==fail)
	{
		TheBlock.Send(sock);//分块数据
	}
	//调试信息
	ShowResiond(L"blockaddparent", UserId, type);
}


//响应blocklineto
void RespondBlockLineInto(SOCKET sock)
{
	//接收
	CString UserId;
	BlockNode TheBlock;
	RecvCString(sock, UserId);//用户名
	int deep;
	//处理
	ListTemplet<OperateNode> *pOperateList;
	CertainRecv(sock, &deep, sizeof(deep));//定位个数
	CertainRecv(sock, g_IntBuf, sizeof(int)*deep);//定位数组
	PacketType type = BlockLineInto(UserId, TheBlock, pOperateList, deep, g_IntBuf);
	//发送
	send(sock, (char *)&type, sizeof(type), 0);//标志
	if(type==success)
	{
		pOperateList->Send(sock);//操作链表
		pOperateList->Clear();//清空操作链表
	}
	else if(type==fail)
	{
		TheBlock.Send(sock);//分块数据
	}
	//调试信息
	ShowResiond(L"blocklineto", UserId, type);
}


//响应blockseparateroot
void RespondBlockSeparateRoot(SOCKET sock)
{
	//接收
	CString UserId;
	BlockNode TheBlock;
	RecvCString(sock, UserId);//用户名
	//处理
	ListTemplet<OperateNode> *pOperateList;
	PacketType type = BlockSeparateRoot(UserId, TheBlock, pOperateList);
	//发送
	send(sock, (char *)&type, sizeof(type), 0);//标志
	if(type==success)
	{
		pOperateList->Send(sock);//操作链表
		pOperateList->Clear();//清空操作链表
	}
	else if(type==fail)
	{
		TheBlock.Send(sock);//分块数据
	}
	//调试信息
	ShowResiond(L"blockseparateroot", UserId, type);
}


//响应blockdelseparate
void RespondBlockDelSeparate(SOCKET sock)
{
	//接收
	CString UserId;
	BlockNode TheBlock;
	RecvCString(sock, UserId);//用户名
	//处理
	ListTemplet<OperateNode> *pOperateList;
	PacketType type = BlockDelSeparate(UserId, TheBlock, pOperateList);
	//发送
	send(sock, (char *)&type, sizeof(type), 0);//标志
	if(type==success)
	{
		pOperateList->Send(sock);//操作链表
		pOperateList->Clear();//清空操作链表
	}
	else if(type==fail)
	{
		TheBlock.Send(sock);//分块数据
	}
	//调试信息
	ShowResiond(L"blockdelseparate", UserId, type);
}


//显示响应事件
void ShowResiond(CString typestring, CString UserId, PacketType result)
{
	SYSTEMTIME time;
	GetLocalTime(&time);
	CString *pstring = new CString;
	GetTime(time, *pstring);
	*pstring += L":\t"+typestring;
	*pstring += L",\t"+UserId;
	*pstring += L",\t";
	switch(result)
	{
	case error:
		*pstring += L"error";
		break;
	case success:
		*pstring += L"success";
		break;
	case fail:
		*pstring += L"fail";
		break;
	case nopermission:
		*pstring += L"nopermission";
		break;
	case clash:
		*pstring += L"clash";
		break;
	}
	*pstring += "\r\n";

	static HWND hWnd;//发消息更新进度
	if(hWnd = AfxGetMainWnd()->m_hWnd)
		::SendMessage(hWnd, WM_LAND_DEBUG, (WPARAM)pstring, 0);
}
