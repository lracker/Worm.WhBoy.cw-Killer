// CFix.cpp: 实现文件
//

#include "pch.h"
#include "PandaKiller.h"
#include "CFix.h"
#include "afxdialogex.h"
#include "Data.h"

// CFix 对话框

IMPLEMENT_DYNAMIC(CFix, CDialogEx)

CFix::CFix(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FIXFILE, pParent)
	, m_FixFile(_T(""))
{

}

CFix::~CFix()
{
}

void CFix::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_FixFile);
}


BEGIN_MESSAGE_MAP(CFix, CDialogEx)
	ON_MESSAGE(UM_FIXFILE, &CFix::OnUmFixfile)
END_MESSAGE_MAP()


// CFix 消息处理程序


afx_msg LRESULT CFix::OnUmFixfile(WPARAM wParam, LPARAM lParam)
{
	CString* str = (CString*)wParam;
	UpdateData(TRUE);
	m_FixFile += *str;
	m_FixFile += "\r\n";
	UpdateData(FALSE);
	return 0;
}
