
// OurMindServerDlg.h : 头文件
//

#pragma once


#define WM_LAND_DEBUG WM_USER+1


// COurMindServerDlg 对话框
class COurMindServerDlg : public CDialogEx
{
// 构造
public:
	COurMindServerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_OURMINDSERVER_DIALOG };
	
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


	//变量
private:
	CWinThread *pNetThread;//网络工作者线程指针
	CString LandString;//LandDebug字符串


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
protected:
	afx_msg LRESULT OnLandDebug(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnBnClickedCancel();
};
