#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <AclAPI.h> 
#pragma warning(disable : 4996)
extern int addLogMessage(const char* text);
int ServiceStart();
void ServiceStop();
HANDLE hEventSend;
HANDLE hEventRecv;
HANDLE hEventTermination;
HANDLE hEvents[2];
HANDLE fileIn;
HANDLE fileOut;
DWORD nIn, nOut;
// ��� �������-������� ��� ������������� ������ � ������ �� ������������� �����
CHAR lpEventSendName[] = "Session\\1\\$MylEventSendName$";
CHAR lpEventRecvName[] = "Session\\1\\$MylEventRecvName$";
// ��� ���������� ����� �� ������
CHAR lpFileShareName[] = "Session\\1\\$MyFileShareName$";
CHAR lpFileSecName[] = "Session\\1\\$MyFileSecName$";
// ������������� ����������� ����� �� ������
HANDLE hFileMapping;
HANDLE hFileSecMapping;
// ��������� �� ������������ ������� ������
LPVOID lpFileMap;
LPVOID lpFileSecMap;
int Server()
{
	DWORD dwRetCode;
	CHAR str[80];
	FILE* hdl;
	DWORD total = 0;
	char newBuf[256];
	char buf[256];
	int size = 4;
	CHAR data[2];
	LPCSTR newFile;
	char message[80] = { 0 };
	addLogMessage("Mapped and shared file, event sync, server process\n");
	addLogMessage("Server Ready!");
	while (TRUE)
	{
		// ��������� �������� ���������� ������ ������ ��������
		addLogMessage("WaitClient...");
		dwRetCode = WaitForSingleObject(hEventSend, INFINITE);
		// ���� �������� ���� ��������, ��� ���� ��������� ������, ��������� ����
		if (dwRetCode == WAIT_ABANDONED_0 || dwRetCode == WAIT_FAILED)
			break;
		// ������ ������ (��� ����� ��� ���������) �� ������������ ������� ������, 
		// ���������� ���� ���������� ���������, � ���������� ��� � ���������� ����
		else
		{
			puts(((LPSTR)lpFileMap));
			puts((LPSTR)lpFileSecMap);
			addLogMessage("Get data!");
			// ��������� ������
			strcpy(str, ((LPSTR)lpFileMap));
			strcpy(data, (LPSTR)lpFileSecMap);
			size = atoi(data);
			addLogMessage(data);
			addLogMessage(str);
			fileIn = CreateFile(str, GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (fileIn == INVALID_HANDLE_VALUE) {
				sprintf(message, "(Server)Can't open %s!", str);
				addLogMessage(message);
				break;
			}
			newFile = "C:\\Users\\Tom\\source\\repos\\out.txt";
			fileOut = CreateFile(newFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (fileOut == INVALID_HANDLE_VALUE) {
				strcpy(str, newFile);
				sprintf(message, "(Server)Can't open %s!", str);
				addLogMessage(message);
				break;
			}
			ReadFile(fileIn, buf, 256, &nIn, NULL);
			for (int i = 0; i < 256; i++)
			{
				if (buf[i] > 48 && total < size) {
					newBuf[i] = ' ';
					total++;
				}
				else {
					newBuf[i] = buf[i];
				}
			}
			WriteFile(fileOut, newBuf, nIn, &nOut, NULL);
			// ��������� � ������� ������ 
			sprintf(message, "(Server): file:%s, replace = %d\n", str, total);
			addLogMessage(message);
			// ��������� � ����� 
			sprintf(message, "%d", (int)total);
			strcpy(((LPSTR)lpFileMap), message);
			// �������� �����
			CloseHandle(fileIn);
			CloseHandle(fileOut);
			addLogMessage("Send responce!");
			SetEvent(hEventRecv);
			total = 0;
		}
	}
	return 0;
}
int ServiceStart()
{
	char message[80] = { 0 };
	DWORD res;
	// ������� �������-������� ��� ������������� ������ � ������ � ������������ ����, ������������ � ������ ���������
	// ����� ����� �������� ������������, ����������� ������������ ������� ���� ������
	// � �� ��������� � ���������������, ��� �� ���������...
	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;
	addLogMessage("Creating security attributes ALL ACCESS for EVERYONE!\n");
	// ������� ���������� ������������
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	// DACL �� ���������� (FALSE) - ������ ���������
	SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
	// ����������� �������� ������������, ��������� ���� ��������� �� ���������� ������������ sd � ������� ������-�������
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = &sd;
	sa.bInheritHandle = FALSE;
	// ��������� ��������� ����������� ������������
	if (!IsValidSecurityDescriptor(&sd))
	{
		res = GetLastError();
		addLogMessage("Security descriptor is invalid.\n");
		sprintf(message, "The last error code: %u\n", res);
		return -(int)res;
	}
	// ������������� ����� ���������� �����
	hEventSend = CreateEvent(&sa, FALSE, FALSE, lpEventSendName);
	hEventRecv = CreateEvent(&sa, FALSE, FALSE, lpEventRecvName);
	hEventSend = CreateEvent(NULL, FALSE, FALSE, lpEventSendName);
	hEventRecv = CreateEvent(NULL, FALSE, FALSE, lpEventRecvName);
	// �������������� ������� ���� ���� �� ���
	SetSecurityInfo(hEventSend, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, NULL, NULL);
	SetSecurityInfo(hEventRecv, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, NULL, NULL);
	// ���� ��������� ������, �������� � ���������� �� ���, � ����� ��������� ������ ����������
	addLogMessage("Creating events\n");
	if (hEventSend == NULL || hEventRecv == NULL)
	{
		sprintf(message, "CreateEvent: Error %ld\n", GetLastError());
		addLogMessage(message);
		return (-1);
	}
	// ������� ������-�����������, ���� �� �������!!!
	hFileMapping = CreateFileMapping((HANDLE)0xFFFFFFFF,
		NULL, PAGE_READWRITE, 0, 100, lpFileShareName);
	hFileSecMapping = CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, 100, lpFileSecName);
	SetSecurityInfo(hFileMapping, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, NULL, NULL);
	SetSecurityInfo(hFileSecMapping, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, NULL, NULL);
	addLogMessage("Creating Mapped file\n");
	// ���� ������� �� �������, ������� ��� ������
	if (hFileMapping == NULL || hFileSecMapping == NULL)
	{
		sprintf(message, "CreateFileMapping: Error %ld\n", GetLastError());
		addLogMessage(message);
		return -2;
	}
	// ��������� ����������� ����� �� ������.
	// � ���������� lpFileMap ����� ������� ��������� �� ������������ ������� ������
	lpFileMap = MapViewOfFile(hFileMapping,
		FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	lpFileSecMap = MapViewOfFile(hFileSecMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	// ���� ��������� ����������� �� �������, ������� ��� ������
	if (lpFileMap == 0 || lpFileSecMap == 0)
	{
		sprintf(message, "MapViewOfFile: Error %ld\n", GetLastError());
		addLogMessage(message);
		return -3;
	}
	return 0;
}
void ServiceStop()
{
	CloseHandle(hEventSend);
	CloseHandle(hEventRecv);
	// �������� ����������� �����
	UnmapViewOfFile(lpFileMap);
	UnmapViewOfFile(lpFileSecMap);
	// ����������� ������������� ���������� �������-�����������
	CloseHandle(hFileMapping);
	CloseHandle(hFileSecMapping);
	addLogMessage("All Kernel objects closed!");
}
