#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#pragma warning(disable : 4996)

// Идентификаторы объектов-событий, которые используются
// для синхронизации, область видимости - сеанс клиента
HANDLE hEventSend;
HANDLE hEventRecv;
// Имя объектов-событий для синхронизации записи и чтения из отобржаемого файла
CHAR lpEventSendName[] = "Session\\1\\$MylEventSendName$";
CHAR lpEventRecvName[] = "Session\\1\\$MylEventRecvName$";
// Имя отображния файла на память
CHAR lpFileShareName[] = "Session\\1\\$MyFileShareName$";
CHAR lpFileSecName[] = "Session\\1\\$MyFileSecName$";
// Идентификатор отображения файла на память
HANDLE hFileMapping;
HANDLE hFileSecMapping;
// Указатель на отображенную область памяти
LPVOID lpFileMap;
LPVOID lpFileSecMap;
int main(int argc, char* argv[])
{
	CHAR str[80];
	DWORD dwRetCode;
	CHAR size[2];
	printf("Mapped and shared file, event sync, client process in Session 1\n"
		"\n\nEnter  <Exit> to terminate...\n");
	// Открываем объекты-события для синхронизации  чтения и записи
	hEventSend = OpenEvent(EVENT_ALL_ACCESS, FALSE, lpEventSendName);
	hEventRecv = OpenEvent(EVENT_ALL_ACCESS, FALSE, lpEventRecvName);
	if (hEventSend == NULL || hEventRecv == NULL)
	{
		fprintf(stdout, "OpenEvent: Error %ld\n",
			GetLastError());
		getch();
		return-1;
	}
	// Открываем объект-отображение
	hFileMapping = OpenFileMapping(
		FILE_MAP_READ | FILE_MAP_WRITE, FALSE, lpFileShareName);
	hFileSecMapping = CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, 100, lpFileSecName);
	// Если открыть не удалось, выводим код ошибки
	if (hFileMapping == NULL || hFileSecMapping == NULL)
	{
		fprintf(stdout, "OpenFileMapping: Error %ld\n",
			GetLastError());
		getch();
		return -2;
	}
	// Выполняем отображение файла на память.
	// В переменную lpFileMap будет записан указатель на отображаемую область памяти
	lpFileMap = MapViewOfFile(hFileMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	lpFileSecMap = MapViewOfFile(hFileSecMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	// Если выполнить отображение не удалось, выводим код ошибки
	if (lpFileMap == 0 || lpFileSecMap == 0)
	{
		fprintf(stdout, "MapViewOfFile: Error %ld\n",
			GetLastError());
		getch();
		return -3;
	}
	// Цикл чтения/записи данных. Этот цикл завершает свою работу,
	// когда пользователь набирает коанду <Exit> 
	while (TRUE)
	{
		// Читаем введенную строку
		printf("Enter file path: ");
		gets(str);
		// Записываем строку в отображенную память, доступную серверному процессу
		strcpy((char*)lpFileMap, str);
		// Если введена команда <Exit>, прерываем цикл
		if (!strcmp(str, "exit") || !strcmp(str, "Exit") || !strcmp(str, "EXIT"))
			break;
		printf("Enter number of substitutions: ");
		gets(size);
		strcpy((char*)lpFileSecMap, size);
		// Устанавливаем объект-событие в отмеченное состояние
		SetEvent(hEventSend);
		// ждем ответа
		dwRetCode = WaitForSingleObject(hEventRecv, INFINITE);
		// если ответ получен - выводим, если ошибка - выходим
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
	// Закрываем идентификаторы объектов-событий
	CloseHandle(hEventSend);
	CloseHandle(hEventRecv);
	// Отменяем отображение файла
	UnmapViewOfFile(lpFileMap);
	// Освобождаем идентификатор созданного объекта-отображения
	CloseHandle(hFileMapping);
	return 0;
}
