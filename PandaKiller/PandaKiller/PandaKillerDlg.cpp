
// PandaKillerDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "PandaKiller.h"
#include "PandaKillerDlg.h"
#include "afxdialogex.h"
#include "CTool.h"
#include "CFix.h"
#include "Data.h"

// CPandaKillerDlg 对话框



CPandaKillerDlg::CPandaKillerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PANDAKILLER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPandaKillerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPandaKillerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CPandaKillerDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CPandaKillerDlg 消息处理程序

BOOL CPandaKillerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	ShowWindow(SW_MINIMIZE);

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CPandaKillerDlg::OnPaint()
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
HCURSOR CPandaKillerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



//******************************************************************************
// 函数名称: OnBnClickedButton1
// 函数说明: 杀毒，关闭进程并且删掉文件
// 作    者: lracker
// 时    间: 2019/11/28
// 返 回 值: void
//******************************************************************************
void CPandaKillerDlg::OnBnClickedButton1()
{
	// 存放提示信息的
	CString Buffer;
	// 1. 关闭进程
	Buffer.Format(L"关闭了%d个病毒进程", CTool::TerminateVirusProcess(L"spo0lsv.exe"));
	MessageBox(Buffer);
	// 2. 杀掉病毒本体
	if (CTool::KillVirus())
		MessageBox(L"成功删掉Drivers目录下的病毒");
	else
		MessageBox(L"删掉Drivers目录下的病毒失败");
	// 3. 遍历磁盘来删掉每一个磁盘下的autorun.inf和setup.exe
	Buffer.Format(L"成功删除%d个文件", CTool::DeleteAutoSetup());
	MessageBox(Buffer);
	// 4. 删掉Desktop_.ini文件
	int DesktopNum = CTool::GetAllFile();
	Buffer.Format(L"一共删除了%d个Desktop_.ini文件", DesktopNum);
	MessageBox(Buffer);
	// 5. 修复文件
	CTool::FixFile();
	// 6. 修改启动项目
	CTool::ChangeRegister();
}
