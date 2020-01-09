#pragma once


// CFix 对话框

class CFix : public CDialogEx
{
	DECLARE_DYNAMIC(CFix)

public:
	CFix(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CFix();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FIXFILE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_FixFile;
protected:
	afx_msg LRESULT OnUmFixfile(WPARAM wParam, LPARAM lParam);
};
