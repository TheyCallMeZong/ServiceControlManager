#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#pragma warning(disable : 4996)

// �������������� ��������-�������, ������� ������������
// ��� �������������, ������� ��������� - ����� �������
HANDLE hEventSend;
HANDLE hEventRecv;
// ��� ��������-������� ��� ������������� ������ � ������ �� ������������ �����
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
int main(int argc, char* argv[])
{
	CHAR str[80];
	DWORD dwRetCode;
	CHAR size[2];
	printf("Mapped and shared file, event sync, client process in Session 1\n"
		"\n\nEnter  <Exit> to terminate...\n");
	// ��������� �������-������� ��� �������������  ������ � ������
	hEventSend = OpenEvent(EVENT_ALL_ACCESS, FALSE, lpEventSendName);
	hEventRecv = OpenEvent(EVENT_ALL_ACCESS, FALSE, lpEventRecvName);
	if (hEventSend == NULL || hEventRecv == NULL)
	{
		fprintf(stdout, "OpenEvent: Error %ld\n",
			GetLastError());
		getch();
		return-1;
	}
	// ��������� ������-�����������
	hFileMapping = OpenFileMapping(
		FILE_MAP_READ | FILE_MAP_WRITE, FALSE, lpFileShareName);
	hFileSecMapping = CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, 100, lpFileSecName);
	// ���� ������� �� �������, ������� ��� ������
	if (hFileMapping == NULL || hFileSecMapping == NULL)
	{
		fprintf(stdout, "OpenFileMapping: Error %ld\n",
			GetLastError());
		getch();
		return -2;
	}
	// ��������� ����������� ����� �� ������.
	// � ���������� lpFileMap ����� ������� ��������� �� ������������ ������� ������
	lpFileMap = MapViewOfFile(hFileMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	lpFileSecMap = MapViewOfFile(hFileSecMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	// ���� ��������� ����������� �� �������, ������� ��� ������
	if (lpFileMap == 0 || lpFileSecMap == 0)
	{
		fprintf(stdout, "MapViewOfFile: Error %ld\n",
			GetLastError());
		getch();
		return -3;
	}
	// ���� ������/������ ������. ���� ���� ��������� ���� ������,
	// ����� ������������ �������� ������ <Exit> 
	while (TRUE)
	{
		// ������ ��������� ������
		printf("Enter file path: ");
		gets(str);
		// ���������� ������ � ������������ ������, ��������� ���������� ��������
		strcpy((char*)lpFileMap, str);
		// ���� ������� ������� <Exit>, ��������� ����
		if (!strcmp(str, "exit") || !strcmp(str, "Exit") || !strcmp(str, "EXIT"))
			break;
		printf("Enter number of substitutions: ");
		gets(size);
		strcpy((char*)lpFileSecMap, size);
		// ������������� ������-������� � ���������� ���������
		SetEvent(hEventSend);
		// ���� ������
		dwRetCode = WaitForSingleObject(hEventRecv, INFINITE);
		// ���� ����� ������� - �������, ���� ������ - �������
		if (dwRetCode == WAIT_OBJECT_0) {
			printf("Replace in file:");
			puts(((LPSTR)lpFileMap));
		}
		if (dwRetCode == WAIT_ABANDONED_0 || dwRetCode == WAIT_FAILED)
		{
			printf("\nError waiting responce!\n)");
			//break;
		}
	}
	// ��������� �������������� ��������-�������
	CloseHandle(hEventSend);
	CloseHandle(hEventRecv);
	// �������� ����������� �����
	UnmapViewOfFile(lpFileMap);
	// ����������� ������������� ���������� �������-�����������
	CloseHandle(hFileMapping);
	return 0;
}
