
// PandaKillerDlg.h: 头文件
//

#pragma once
#include <TlHelp32.h>
#include <vector>
using std::vector;

// CPandaKillerDlg 对话框
class CPandaKillerDlg : public CDialogEx
{
// 构造
public:
	CPandaKillerDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PANDAKILLER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	// 关闭进程并且杀掉病毒有关的内容
	afx_msg void OnBnClickedButton1();
};
