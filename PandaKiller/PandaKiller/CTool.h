#pragma once
#include <vector>
#include <TlHelp32.h>
using std::vector;
// ������
class CTool
{
public:
	CTool();
	// ������б���Ⱦ��PE�ļ�
	vector<CString> m_PeVector;
	// ������б���Ⱦ��WEB�ļ�
	vector<CString> m_WebVector;
	// ���Ա������н��̵��б�
	vector<PROCESSENTRY32> m_ProcessList;
	// ��ȡ���еĽ���
	static void GetAllProcess(vector<PROCESSENTRY32>& ProcessList);
	// �رս���
	static int TerminateVirusProcess(CString ProcessName);
	// ɾ��autorun.inf��setup.exe
	static int DeleteAutoSetup();
	// �������ô����µ����е��ļ�
	static void GetFile();
	// ���������д����µ��ļ�(����A�̺�B��)
	static int GetAllFile();
	// ɾ����������
	static BOOL KillVirus();
	// �����Ĵ�С
	int m_VirusSize;
	// �鿴�ڴ����Ƿ�ӵ��WhBoy�ַ���
	static int IsInfect(PCHAR Buffer, PCHAR WHBOY);
	// �鿴�ڴ����Ƿ�ӵ��WEB���ַ���
	static BOOL IsInfect2(PCHAR Buffer);
	// �޸��ļ�
	static int FixFile();
	// ��ȡ�����Ĵ�С
	static int GetVirusSize(CString FileName);
	// �޸�ע���
	static void ChangeRegister();
};
