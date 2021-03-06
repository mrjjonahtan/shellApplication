// shellApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "PeToolsClass.h"

struct sectionHeader
{
	union
	{
		DWORD PhysicalAddress;
		DWORD VirtualSize;
	} Misc;
	DWORD VirtualAddress;
	DWORD SizeOfRawData;
	DWORD PointerToRawData;
	DWORD Characteristics;
};

BYTE *messageBuffer = NULL;
DWORD messageBufferSize = 0;
DWORD srcSize = 0;
DWORD imageBase = 0;
DWORD sizeOfImage = 0;
DWORD addressOfEntryPoint = 0;

PeToolsClass petc;

//获得有用数据
void getMessage(BYTE *srcPointer)
{
	DWORD ifanew = 0;
	DWORD sizeOfHeaders = 0;
	DWORD numberOfSections = 0;
	DWORD sizeOfOptionalHeader = 0;
	DWORD numberOfSectionsOffSetLocation = 0;

	//
	ifanew = petc.getPELocation(srcPointer);
	numberOfSections = petc.getDWValue((srcPointer + ifanew + 6), 2);
	sizeOfOptionalHeader = petc.getDWValue((srcPointer + ifanew + 20), 2);

	imageBase = petc.getDWValue((srcPointer + ifanew + 24 + 28), 4);
	sizeOfImage = petc.getDWValue((srcPointer + ifanew + 24 + 56), 4);
	sizeOfHeaders = petc.getDWValue((srcPointer + ifanew + 24 + 60), 4);
	addressOfEntryPoint = petc.getDWValue((srcPointer + ifanew + 24 + 16), 4);

	sectionHeader ah[10] = { 0 };

	numberOfSectionsOffSetLocation = ifanew + 24 + sizeOfOptionalHeader;
	for (WORD i = 0; i < numberOfSections; i++)
	{
		ah[i].Misc.VirtualSize = petc.getDWValue((srcPointer + numberOfSectionsOffSetLocation + i * 40 + 8), 4);
		ah[i].VirtualAddress = petc.getDWValue((srcPointer + numberOfSectionsOffSetLocation + i * 40 + 12), 4);
		ah[i].SizeOfRawData = petc.getDWValue((srcPointer + numberOfSectionsOffSetLocation + i * 40 + 16), 4);
		ah[i].PointerToRawData = petc.getDWValue((srcPointer + numberOfSectionsOffSetLocation + i * 40 + 20), 4);
		ah[i].Characteristics = petc.getDWValue((srcPointer + numberOfSectionsOffSetLocation + i * 40 + 36), 4);
	}

	//
	messageBufferSize = petc.getAlignData(sizeOfImage, 0x1000);
	messageBuffer = (BYTE *)malloc(messageBufferSize);
	if (messageBuffer == NULL) {
		return;
	}
	memset(messageBuffer, 0, messageBufferSize);

	memcpy_s(messageBuffer, messageBufferSize, srcPointer, sizeOfHeaders);

	for (WORD i = 0; i < numberOfSections; i++)
	{
		memcpy_s((messageBuffer + ah[i].VirtualAddress), messageBufferSize, (srcPointer + ah[i].PointerToRawData),  ah[i].SizeOfRawData);
	}

	/*FILE *fp = NULL;
	fopen_s(&fp, "C://Users//jonathan//Desktop//calc//shells.exe", "wb");

	if (fp == NULL)
	{
		return;
	}

	if (fwrite(messageBuffer, messageBufferSize, 1, fp) != 1)
	{
		fclose(fp);
		return;
	}

	fclose(fp);

	MessageBox(NULL, L"shell程序已生成。\n路径：C://Users//jonathan//Desktop//calc//shell.exe", L"提示", MB_OK);*/
}

//解密
void decrypt(BYTE *data, DWORD size) {
	if (size == 0 || data == NULL) {
		return;
	}

}

//获得最后节
void getData(char *path)
{
	FILE *openFile = NULL;
	BYTE *message = NULL;
	BYTE *pointer = NULL;
	DWORD size = 0;
	DWORD ifanew = 0;
	DWORD numberOfSections = 0;
	DWORD sizeOfOptionalHeader = 0;
	sectionHeader sh;

	fopen_s(&openFile, path, "rb");
	if (openFile == NULL) {
		return;
	}
	if (fseek(openFile, 0L, SEEK_END)) {
		return;
	}
	size = ftell(openFile);
	if (size == 0) {
		return;
	}

	pointer = (BYTE*)malloc(sizeof(BYTE)*size);
	if (pointer == NULL) {
		return;
	}
	memset(pointer, 0L, sizeof(BYTE)*size);
	fseek(openFile, 0L, 0L);
	if (fread(pointer, sizeof(BYTE)*size, 1, openFile) <= 0) {
		return;
	}
	if (openFile != NULL) {
		fclose(openFile);
		openFile = NULL;
	}

	ifanew = petc.getPELocation(pointer);
	numberOfSections = petc.getDWValue((pointer + ifanew + 6), 2);
	sizeOfOptionalHeader = petc.getDWValue((pointer + ifanew + 20), 2);


	sh.Misc.VirtualSize = petc.getDWValue((pointer + ifanew + 24 + sizeOfOptionalHeader + (numberOfSections - 1) * 40 + 8), 4);
	sh.VirtualAddress = petc.getDWValue((pointer + ifanew + 24 + sizeOfOptionalHeader + (numberOfSections - 1) * 40 + 12), 4);
	sh.SizeOfRawData = petc.getDWValue((pointer + ifanew + 24 + sizeOfOptionalHeader + (numberOfSections - 1) * 40 + 16), 4);
	sh.PointerToRawData = petc.getDWValue((pointer + ifanew + 24 + sizeOfOptionalHeader + (numberOfSections - 1) * 40 + 20), 4);
	sh.Characteristics = petc.getDWValue((pointer + ifanew + 24 + sizeOfOptionalHeader + (numberOfSections - 1) * 40 + 36), 4);

	message = (BYTE *)malloc(sh.SizeOfRawData);
	if (message == NULL) {
		return;
	}
	memset(message, 0, sh.SizeOfRawData);

	memcpy_s(message, sh.SizeOfRawData, pointer + sh.PointerToRawData, sh.SizeOfRawData);

	if (pointer != NULL) {
		free(pointer);
		pointer = NULL;
	}

	srcSize = sh.SizeOfRawData;

	//
	decrypt(message, srcSize);
	//
	getMessage(message);

	if (message != NULL) {
		free(message);
		message = NULL;
	}
}


//改变镜像
BOOL setMirror(char *cpath)
{
	unsigned long BaseAddr;
	STARTUPINFO  si = { 0 };
	PROCESS_INFORMATION pi;
	si.cb = sizeof(si);

	CONTEXT caContext = { 0 };
	caContext.ContextFlags = CONTEXT_FULL;

	unsigned int cSize = strlen(cpath) + 1;
	wchar_t *path = new wchar_t[cSize];
	swprintf(path, cSize, L"%hs", cpath);

	BOOL bol = CreateProcess(NULL, path, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);

	if (!GetThreadContext(pi.hThread, &caContext))
	{
		DWORD error = ::GetLastError();
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		return FALSE;
	}

	ReadProcessMemory(pi.hProcess, (void *)(caContext.Ebx + 8), &BaseAddr, 4, NULL);

	if (!BaseAddr)
	{
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		return FALSE;
	}

	//
	typedef unsigned long(__stdcall *pfZwUnmapViewOfSection)(unsigned long, unsigned long);
	pfZwUnmapViewOfSection ZwUnmapViewOfSection = NULL;
	HMODULE lld = LoadLibrary(L"ntdll.dll");
	if (lld) {
		ZwUnmapViewOfSection = (pfZwUnmapViewOfSection)GetProcAddress(lld, "ZwUnmapViewOfSection");
		if (ZwUnmapViewOfSection((unsigned long)pi.hProcess, BaseAddr))
		{
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
			return FALSE;
		}
	}

	PVOID shellImageBase = VirtualAllocEx(pi.hProcess, (LPVOID)imageBase, messageBufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	
	DWORD retByte = 0;
	//写数据到内存
	WriteProcessMemory(pi.hProcess, shellImageBase, messageBuffer, messageBufferSize, &retByte);


	//重置imagebase
	WriteProcessMemory(pi.hProcess, (void *)(caContext.Ebx + 8), &shellImageBase, 4, NULL);

	//设置入口点
	caContext.Eax = (DWORD)(imageBase + addressOfEntryPoint);

	SetThreadContext(pi.hThread, &caContext);

	ResumeThread(pi.hThread);
	return TRUE;
}

void freeFun() {
	if (messageBuffer != NULL) {
		free(messageBuffer);
		messageBuffer = NULL;
	}
}

int main(int argc, char* argv[])
{

	//char path[50] = "C://Users//jonathan//Desktop//calc//shell.exe";
	getData(argv[0]);
	setMirror(argv[0]);
	freeFun();

	return 0;
}

