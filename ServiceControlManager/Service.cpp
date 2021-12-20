#include<stdio.h>
#include<windows.h>
#include<string.h>
#include <conio.h>
#pragma warning(disable : 4996)

DWORD dwErrCode;                      //��� ������
SERVICE_STATUS ss;                    //������� ���������� �������
SERVICE_STATUS_HANDLE ssHandle;       //���������� ������� �������

LPTSTR MyService = "My Service <3";  //��� �������
VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
void WINAPI ServiceControl(DWORD dwControlCode);
void ReportStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
extern int Server();
extern int ServiceStart();
void ServiceStop();

void addLogMessage(char* text) {
	DWORD res, Sz;
	HANDLE file;
	char buf[256];
	file = CreateFile("C:\\Users\\Tom\\source\\repos\\ServiceControlManager.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (!file) {
		return;
	}
	GetFileSize(file, &Sz);
	SetFilePointer(file, 0, NULL, FILE_END);
	sprintf(buf, "%s\r\n", text);
	WriteFile(file, buf, strlen(buf), &res, NULL);
	CloseHandle(file);
}

void main() {
	char buf[256];
	SERVICE_TABLE_ENTRY DispatcherTable[] =
	{
		{
			MyService,
			(LPSERVICE_MAIN_FUNCTION)ServiceMain
		},
		{
			NULL,
			NULL
		}
	};

	if (!StartServiceCtrlDispatcher(DispatcherTable))
	{
		sprintf(buf, "StartServiceCtrlDispatcher: Error %ld\n", GetLastError());
		addLogMessage(buf);
		return;
	}
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
	char buf[256];
	int res = 0;
	ssHandle = RegisterServiceCtrlHandler(MyService, ServiceControl);
	if (!ssHandle) {
		addLogMessage("ServiceMain, �� ������� ���������������� ������");
		return;
	}
	//��������� ���������, ������������ ��������� �������:
   //������ ����������� ��� ��������� �������
	ss.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	//������������� ��������� �������� ������� �������
	ReportStatus(SERVICE_START_PENDING, NO_ERROR, 30000);
	addLogMessage("ServiceMain, Service starting");
	res = ServiceStart();
	if (res < 0) {
		sprintf(buf, "Error init ServiceMain %d", res);
		addLogMessage(buf);
		ServiceControl(SERVICE_CONTROL_STOP);
		return;
	}

	ReportStatus(SERVICE_RUNNING, NOERROR, 0);
	addLogMessage("Service started!");
	if (Server() > 0)
	{
		addLogMessage("Server MF started!");
	}
	else
	{
		sprintf(buf, "Error starting server %d", res);
		addLogMessage(buf);
		ServiceControl(SERVICE_CONTROL_STOP);
	}
	return;
}
void WINAPI ServiceControl(DWORD dwControlCode)
{
	// ����������� ��� ������� � ��������� ��� �������
	switch (dwControlCode)
	{
		// ������� ��������� �������
	case SERVICE_CONTROL_STOP:
	{
		// ������������� ��������� �������� ���������
		ss.dwCurrentState = SERVICE_STOP_PENDING;
		ReportStatus(ss.dwCurrentState, NOERROR, 0);
		addLogMessage("Service stopping...");
		// ��������� ��������� �������, ������� �������, ������� ��������� ��� ����������� ��� ����� ��������
		ServiceStop();
		// �������� ��������� ��� ������������� ������
		ReportStatus(SERVICE_STOPPED, NOERROR, 0);
		addLogMessage("Service stopped!");
		break;
	}
	// ����������� �������� ��������� �������
	case SERVICE_CONTROL_INTERROGATE:
	{
		// ���������� ������� ��������� �������
		ReportStatus(ss.dwCurrentState, NOERROR, 0);
		break;
	}
	// � ����� �� ������ ������� ������ ���������� ������� ��������� �������
	default:
	{
		ReportStatus(ss.dwCurrentState, NOERROR, 0);
		break;
	}
	}
}
void ReportStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
	// ������� ����� ���������� ��������
	static DWORD dwCheckPoint = 1;
	// ���� ������ �� ��������� � �������� �������, ��� ����� ����������
	if (dwCurrentState == SERVICE_START_PENDING)
		ss.dwControlsAccepted = 0;
	else
		ss.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	// ��������� ���������, ���������� ����� ��������� �������
	ss.dwCurrentState = dwCurrentState;
	ss.dwWin32ExitCode = dwWin32ExitCode;
	ss.dwWaitHint = dwWaitHint;
	// ���� ������ �� �������� � �� ����������, ����������� �������� �������� ����� ���������� ��������
	if ((dwCurrentState == SERVICE_RUNNING) ||
		(dwCurrentState == SERVICE_STOPPED))
		ss.dwCheckPoint = 0;
	else
		ss.dwCheckPoint = dwCheckPoint++;
	// �������� ������� ��������� ���������
	SetServiceStatus(ssHandle, &ss);
}
