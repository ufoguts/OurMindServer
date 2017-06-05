
// 网络线程
//

#pragma once

/*
第一线程：
负责处理界面，通过消息传递其显示信息

第二线程：
接收网络消息并回复
对于每个用户，当其在线编辑时，维护一个操作队列
上传信息时，给此图每个用户更新操作队列
请求更新时，发送操作队列并清空
注意上传分块操作和关系操作不一定定位数组正确
要根据此用户的待发送操作队列进行定位数组转换

第三线程：
定时循环查找思维导图
若用户同步时间超时2s，则结束其编辑

访问权限逻辑：
当用户在线编辑时被取消权限，
清除思维导图分享对应项
清除该用户参与分享对应项
若该用户正在编辑状态，清除操作链表
不清除该用户在线状态和编辑导图指针
用户再次请求更新时，若没有参与分享项，认为取消权限
若未等到用户请求更新就又加入为共享用户
判断其在线状态和编辑导图指针，发现异常，返回其断线
强迫退出重连，避免操作链表不匹配

分块锁定逻辑：
若分块锁定且有锁定用户则为用户编辑锁定
没有锁定用户为拥有者设置不可更改
拥有者可以改变不可更改的分块
对于可更改分块：
{
申请分块时锁定，标记锁定用户
同时释放该用户的之前锁定分块（如果存在）
释放时释放该用户的之前锁定分块
}
对于不可更改分块：
{
只有拥有者可以申请
申请时不记录锁定用户
释放时不解除锁定
}


*/


/****************************************************************/
/*************************服务器同步开始*************************/
/****************************************************************/

#define SERVER_PORT "12345"


//tcp包上级协议
enum PacketType
{
	//标志位
	defaultvalue,//缺省值
	error,//连接错误
	success,//成功
	fail,//失败
	nopermission,//没有权限
	clash,//冲突

	//用户信息操作
	hello,//连接服务器
	login,//登录
	regist,//注册
	change,//修改信息
	logout,//注销
	getlist,//请求获取思维导图列表
	newmind,//新建思维导图
	selmind,//选择思维导图
	delmind,//删除思维导图
	editend,//结束编辑

	//导图编辑操作
	changename,//更改名字
	addshare,//添加共享用户
	delshare,//删除共享用户
	lockblock,//设置锁定，解锁
	upmind,//上传思维导图
	keepmind,//无改变思维导图
	downmind,//下载思维导图
	applyblock,//申请编辑分块
	finishblock,//完成分块编辑
	blockaddroot,//添加根分块
	blockdelwith,//分块删除连带子分块
	blockdelcombine,//分块删除合并子分块
	blockaddchild,//分块插入子分块
	blockaddparent,//分块插入父分块
	blocklineto,//分块连接到新分块
	blockseparateroot,//分块分离为根分块
	blockdelseparate,//分块删除分离子分块

};


//保证接收长度函数
void CertainRecv(SOCKET sock, void *buf, int len);


//发送CString
void SendCString(SOCKET sock, CString &string);


//接收CString
void RecvCString(SOCKET sock, CString &string);

/****************************************************************/
/*************************服务器同步结束*************************/
/****************************************************************/


//网络初始化函数
void ConnectInit();


//网络线程回调函数
UINT NetThreadFunc(LPVOID pParam);


//响应hello
void RespondHello(SOCKET sock);


//响应login
void RespondLogin(SOCKET sock);


//响应regist
void RespondRegist(SOCKET sock);


//响应change
void RespondChange(SOCKET sock);


//响应get
void RespondGetList(SOCKET sock);


//响应newmind
void RespondNewMind(SOCKET sock);


//响应selmind
void RespondSelMind(SOCKET sock);


//响应delmind
void RespondDelMind(SOCKET sock);


//响应editend
void RespondEditEnd(SOCKET sock);


//响应changename
void RespondChangeName(SOCKET sock);


//响应addshare
void RespondAddShare(SOCKET sock);


//响应delshare
void RespondDelShare(SOCKET sock);


//响应lockblock
void RespondLockBlock(SOCKET sock);


//响应upmind
void RespondUpMind(SOCKET sock);


//响应keepmind
void RespondKeepMind(SOCKET sock);


//响应downmind
void RespondDownMind(SOCKET sock);


//响应applyblock
void RespondApplyBlock(SOCKET sock);


//响应finishblock
void RespondFinishBlock(SOCKET sock);


//响应blockaddroot
void RespondBlockAddRoot(SOCKET sock);


//响应blockdelwith
void RespondBlockDelWith(SOCKET sock);


//响应blockdelcombine
void RespondBlockDelCombine(SOCKET sock);


//响应blockaddchild
void RespondBlockAddChild(SOCKET sock);


//响应blockaddparent
void RespondBlockAddParent(SOCKET sock);


//响应blocklineto
void RespondBlockLineInto(SOCKET sock);


//响应blockseparateroot
void RespondBlockSeparateRoot(SOCKET sock);


//响应blockdelseparate
void RespondBlockDelSeparate(SOCKET sock);


//显示响应事件
void ShowResiond(CString typestring, CString UserId, PacketType result);
