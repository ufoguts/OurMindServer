
// OurMindServerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "OurMindServer.h"
#include "OurMindServerDlg.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// COurMindServerDlg 对话框



COurMindServerDlg::COurMindServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(COurMindServerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void COurMindServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(COurMindServerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &COurMindServerDlg::OnBnClickedOk)
	ON_MESSAGE(WM_LAND_DEBUG, &COurMindServerDlg::OnLandDebug)
	ON_BN_CLICKED(IDCANCEL, &COurMindServerDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// COurMindServerDlg 消息处理程序

BOOL COurMindServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	
	//创建工作者线程并运行
	pNetThread = AfxBeginThread(NetThreadFunc, this);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void COurMindServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR COurMindServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//指定控件添加或删除显示字符串
afx_msg LRESULT COurMindServerDlg::OnLandDebug(WPARAM wParam, LPARAM lParam)
{
	CString *pstring = (CString *)wParam;
	if(pstring==0)
		LandString = L"";
	else
	{
		LandString += *pstring;
		delete pstring;
	}

	CEdit *pEdit = (CEdit *)GetDlgItem(IDC_EDIT_LANDDEBUG);
	int Tab[] = {65};
	pEdit->SetTabStops(1, Tab);
	SetDlgItemText(IDC_EDIT_LANDDEBUG, LandString);
	SendDlgItemMessage(IDC_EDIT_LANDDEBUG,WM_VSCROLL, SB_BOTTOM,0);

	return 0;
}




void COurMindServerDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnOK();
}

void COurMindServerDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	
	//保存用户数据
	DataSave();

	CDialogEx::OnCancel();
}
