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
// ����̷�
vector<CString> g_Drivers;
// ��������в��������ļ���
vector<CString> g_Folder;
// ���WHBOY���
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
// ��������: GetAllProcess
// ����˵��: ��ȡ���еĽ���
// ��    ��: lracker
// ʱ    ��: 2019/11/28
// ��    ��: vector<PROCESSENTRY32> & ProcessList
// �� �� ֵ: void
//******************************************************************************
void CTool::GetAllProcess(vector<PROCESSENTRY32>& ProcessList)
{
	// ��ȡ���̿���
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe = { sizeof(PROCESSENTRY32) };
	// ��ȡ��һ������
	if (!Process32First(hSnapShot, &pe))
		return;
	do 
	{
		ProcessList.push_back(pe);
	} while (Process32Next(hSnapShot,&pe));
}

//******************************************************************************
// ��������: TerminateVirusProcess
// ����˵��: �رս���
// ��    ��: lracker
// ʱ    ��: 2019/11/28
// ��    ��: CString ProcessName
// �� �� ֵ: int �رս��̸���
//******************************************************************************
int CTool::TerminateVirusProcess(CString ProcessName)
{
	Tool.m_ProcessList.clear();
	// ��ȡ���еĽ���
	CTool::GetAllProcess(Tool.m_ProcessList);
	int nCount = 0;
	for (auto& i : Tool.m_ProcessList)
	{
		// ���������Ϊspo0lsv.exe,��ر���
		if (!wcscmp(i.szExeFile, L"spo0lsv.exe"))
		{
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, i.th32ProcessID);
			// �رս���
			if (TerminateProcess(hProcess, 0))
			{
				nCount++;
			}
		}
	}
	return nCount;

}

//******************************************************************************
// ��������: DeleteAutoSetup
// ����˵��: �������̸�����ɾ��ÿһ�����̵��µ�autorun.inf��setup.exe
// ��    ��: lracker
// ʱ    ��: 2019/11/28
// �� �� ֵ: int
//******************************************************************************
int CTool::DeleteAutoSetup()
{
	int nCount = 0;
	// �ж��ж��ٸ���Ч���̣�����AB��
	for (WCHAR i = 'C'; i <= 'Z'; i++)
	{
		CString Driver(i);
		Driver += L":\\";
		int Type = GetDriveType(Driver);
		// �жϴ�������
		if ((Type == 2) | (Type == 3) | (Type == 4))
		{
			// ɾ������Ŀ¼�µ�autorun.inf��setup.exe
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
// ��������: GetAllFile
// ����˵��: ���������е��ļ�
// ��    ��: lracker
// ʱ    ��: 2019/11/28
// ��    ��: void
// �� �� ֵ: void
//******************************************************************************
void CTool::GetFile()
{
	// ��ȡ���̷�
	WaitForSingleObject(g_hMutex2, INFINITE);
	CString szFilePath = g_Drivers.back();
	szFilePath.Append(L":");
	g_Drivers.pop_back();
	ReleaseMutex(g_hMutex2);
	// ���ݻ�ȡ�����̷���ʼ����
	queue<CString> qFolders;
	qFolders.push(szFilePath);
	WIN32_FIND_DATA FindFileData;
	HANDLE hFile = NULL;
	USES_CONVERSION;
	while (qFolders.size() > 0)
	{
		// ��ʼ�������Ŀ¼
		CString TempFolder = qFolders.front();
		TempFolder.Append(L"\\*.*");
		hFile = FindFirstFile(TempFolder.GetBuffer(), &FindFileData);
		do 
		{
			// ƴ��Ϊ����·��
			TempFolder = qFolders.front();
			TempFolder.Append(L"\\");
			TempFolder.Append(FindFileData.cFileName);
			// �ж��ǲ���Ŀ¼
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				// �ж��ǲ��Ǳ���Ŀ¼���ϼ�Ŀ¼�����ƣ��ǵĻ����������ѭ��
				if (!lstrcmp(FindFileData.cFileName, L".") || !lstrcmp(FindFileData.cFileName, L".."))
					continue;
				// ѹ���µ�Ŀ¼
				qFolders.push(TempFolder);
			}
			else
			{
				// �ж������ǲ���Desktop_.ini�ļ�
				if (!lstrcmp(FindFileData.cFileName, L"Desktop_.ini"))
				{
					WaitForSingleObject(g_hMutex, INFINITE);
					SetFileAttributes(TempFolder.GetBuffer(), FILE_ATTRIBUTE_ARCHIVE);
					if (remove(W2A(TempFolder.GetBuffer())) == 0)
						g_FileNum++;
					ReleaseMutex(g_hMutex);
					continue;
				}
				// ��ȡ��׺
				CString Suffix = PathFindExtension(FindFileData.cFileName);
				// �����׺Ϊexe��scr��pif��com��Ҫ�鿴ѹ��������
				if (Suffix.MakeUpper() == L".EXE" || Suffix.MakeUpper() == L".SCR" || Suffix.MakeUpper() == L".PIF" || Suffix.MakeUpper() == L".COM")
				{
					WaitForSingleObject(g_hMutex3, INFINITE);
					Tool.m_PeVector.push_back(TempFolder);
					ReleaseMutex(g_hMutex3);
				}
				// �����׺ΪWEB���ͣ���ѹ��������
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
// ��������: ThreadProc
// ����˵��: �̻߳ص����������ڱ��������µ������ļ�
// ��    ��: lracker
// ʱ    ��: 2019/11/28
// ��    ��: LPVOID args
// �� �� ֵ: DWORD CALLBACK
//******************************************************************************
DWORD CALLBACK ThreadProc(LPVOID args)
{
	CTool::GetFile();
	return 0;
}

//******************************************************************************
// ��������: GetAllFile
// ����˵��: ���������д����µ��ļ�(����A�̺�B��)
// ��    ��: lracker
// ʱ    ��: 2019/11/28
// �� �� ֵ: ����ɾ�����ļ�����
//******************************************************************************
int CTool::GetAllFile()
{
	// ����߳̾��
	HANDLE hThread[24] = {};
	// ���Ȼ�ȡ�ж��ٸ���Ч�Ĵ���,����AB��
	for (WCHAR i = 'C'; i <= 'Z'; i++)
	{
		CString Driver(i);
		Driver += L":\\";
		int Type = GetDriveType(Driver);
		// �жϴ�������
		if ((Type == 2) | (Type == 3) | (Type == 4))
			g_Drivers.push_back((CString)i);
	}
	// �ļ���Ŀ���
	g_FileNum = 0;
	// �����̣߳����ٸ�Ӳ�̾��ж��ٸ��߳�
	for (int i = 0; i < g_Drivers.size(); i++)
		hThread[i] = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
	// �ȴ��߳̽���
	for (int i = 0; i < 4; i++)
		WaitForSingleObject(hThread[i], INFINITE);
	return g_FileNum;
}

//******************************************************************************
// ��������: KillVirus
// ����˵��: ɾ����������
// ��    ��: lracker
// ʱ    ��: 2019/11/28
// �� �� ֵ: BOOL
//******************************************************************************
BOOL CTool::KillVirus()
{
	// ��ȡ��ϵͳ��
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
// ��������: IsInfect
// ����˵��: �鿴�ڴ����Ƿ���WHBOY�ַ���
// ��    ��: lracker
// ʱ    ��: 2019/11/30
// ��    ��: LPVOID Buffer
// ��    ��: CHAR * WHBOY
// �� �� ֵ: int �����±�
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
// ��������: IsInfect2
// ����˵��: �鿴web�ļ��Ƿ񱻸�Ⱦ��
// ��    ��: lracker
// ʱ    ��: 2019/11/30
// ��    ��: PCHAR Buffer
// ��    ��: PCHAR WHBOY
// �� �� ֵ: int
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
// ��������: FixFile
// ����˵��: �޸��ļ�
// ��    ��: lracker
// ʱ    ��: 2019/11/30
// �� �� ֵ: int
//******************************************************************************
int CTool::FixFile()
{
	HANDLE hPE = NULL;
	HANDLE hPE2 = NULL;
	HANDLE hPE3 = NULL;
	HANDLE hWeb = NULL;
	HANDLE hWeb2 = NULL;
	int nIndex = 0;
	// ��ʼ�޸�WEB�ļ�
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
		// ���Ű�ԭ�ļ���ɾ��
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
	// ��ʼ����PEVector

	for (auto& i : Tool.m_PeVector)
	{
		CString FileName = i.GetBuffer();
		hPE = CreateFile(FileName, GENERIC_ALL, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		CHAR Buffer[286] = {};
		OVERLAPPED overlap1 = { 0 };
		// 1 + 5 + 260 + 1 + 3 + 1 + 3 + 1 + 10 + 1 == 286���ֽ�
		int nVirFileSize = GetFileSize(hPE, NULL);
		int nOffset = nVirFileSize - 286;
		overlap1.Offset = nOffset;
		ReadFile(hPE, Buffer, 286, 0, &overlap1);
		// 2. ��������ļ�û�б���Ⱦ���������һ���ļ�
		nIndex = IsInfect(Buffer, g_PeTab);
		if (hPE)
			CloseHandle(hPE);
		hPE = NULL;
		if (nIndex == 0)
			continue;
		// ��ȡ���ļ���ԭ����С
		int FileSize = CTool::GetVirusSize(FileName);
		// 3. �������Ⱦ�ˣ���ʼ�ָ�
		hPE3 = CreateFile(FileName, GENERIC_ALL, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		OVERLAPPED overlap2 = { 0 };
		//int nVirusSize = 0x7531;
//		int FileSize = nVirFileSize - nVirusSize - 286 + nIndex - 1;
		int nVirusSize = nVirFileSize - 286 + nIndex - 1 - FileSize;
		CString Err;
		Err.Format(L"��Ⱦ�ļ���С%X", nVirFileSize);
		OutputDebugString(Err);
		Err.Format(L"������С%X", nVirusSize);
		OutputDebugString(Err);
		Err.Format(L"nIndex��С%X", nIndex);
		OutputDebugString(Err);
		Err.Format(L"�ļ���С%X", FileSize);
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
		OutputDebugString(L"��ȡ�������");
		// ���Ű�ԭ�ļ���ɾ��
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
// ��������: GetVirusSize
// ����˵��: ��ȡ�����Ĵ�С
// ��    ��: lracker
// ʱ    ��: 2019/11/30
// �� �� ֵ: int
//******************************************************************************
int CTool::GetVirusSize(CString FileName)
{
	HANDLE hFile = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	int Size1 = GetFileSize(hFile, NULL);
	int nOffset = Size1 - 12;
	CString Err;
	Err.Format(L"����%s", FileName.GetBuffer());
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
// ��������: ChangeRegister
// ����˵��: �޸�ע���
// ��    ��: lracker
// ʱ    ��: 2019/11/30
// �� �� ֵ: void
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
				 Err.Format(L"ע����������д����쳣������\r\n");
			}

			lRet = RegDeleteValueA(hKeyHKCU, "svcshare");
			if (lRet == ERROR_SUCCESS)
			{
				Err.Format(L"ɾ���ɹ���\r\n");
			}
			else
			{
				 Err.Format(L"ɾ��ʧ��\r\n");
			}
		}
		else
		{
			Err.Format(L"ע��������쳣������\r\n");
		}
		RegCloseKey(hKeyHKCU);
	}
	else
	{
		Err.Format(L"ע�����������Ϣ��ȡʧ��\r\n");
	}
	OutputDebugString(Err);
	// �������޸��ļ���������ʾ����Ҫ��CheckedValue��ֵ����Ϊ1
	char RegHide[] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\Folder\\Hidden\\SHOWAL";
	HKEY hKeyHKLM = NULL;
	DWORD dwFlag = 1;

	long lRetHide = RegOpenKeyA(HKEY_LOCAL_MACHINE, RegHide, &hKeyHKLM);
	if (lRetHide == ERROR_SUCCESS)
	{
		Err.Format(L"���ע�����ļ�����ѡ��...\r\n");
		if (ERROR_SUCCESS == RegSetValueExA(
			hKeyHKLM,             //subkey handle  
			"CheckedValue",       //value name  
			0,                    //must be zero  
			REG_DWORD,            //value type  
			(CONST BYTE*) & dwFlag, //pointer to value data  
			4))                   //length of value data
		{
			Err.Format(L"ע����޸���ϣ�\r\n");
		}
		else
		{
			Err.Format(L"�޷��ָ�ע�����ļ�����ѡ��\r\n");
		}
	}
	OutputDebugString(Err);
}

