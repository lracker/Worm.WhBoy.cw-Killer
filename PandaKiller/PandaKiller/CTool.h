#pragma once
#include <vector>
#include <TlHelp32.h>
using std::vector;
// 工具类
class CTool
{
public:
	CTool();
	// 存放所有被感染的PE文件
	vector<CString> m_PeVector;
	// 存放所有被感染的WEB文件
	vector<CString> m_WebVector;
	// 用以保存所有进程的列表
	vector<PROCESSENTRY32> m_ProcessList;
	// 获取所有的进程
	static void GetAllProcess(vector<PROCESSENTRY32>& ProcessList);
	// 关闭进程
	static int TerminateVirusProcess(CString ProcessName);
	// 删掉autorun.inf和setup.exe
	static int DeleteAutoSetup();
	// 遍历出该磁盘下的所有的文件
	static void GetFile();
	// 遍历出所有磁盘下的文件(除了A盘和B盘)
	static int GetAllFile();
	// 删除病毒本体
	static BOOL KillVirus();
	// 病毒的大小
	int m_VirusSize;
	// 查看内存中是否拥有WhBoy字符串
	static int IsInfect(PCHAR Buffer, PCHAR WHBOY);
	// 查看内存中是否拥有WEB的字符串
	static BOOL IsInfect2(PCHAR Buffer);
	// 修复文件
	static int FixFile();
	// 获取病毒的大小
	static int GetVirusSize(CString FileName);
	// 修改注册表
	static void ChangeRegister();
};
