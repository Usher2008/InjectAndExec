// InjectAndExec.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
#include <tlhelp32.h>

typedef HANDLE (WINAPI *__BypassCreateRemoteThread) (
	HANDLE,
	LPSECURITY_ATTRIBUTES,
	SIZE_T,
	LPTHREAD_START_ROUTINE,
	LPVOID,
	DWORD,
	LPDWORD
);

bool injectDll(char sDllPath[], WCHAR run_path[])
{
	STARTUPINFO si = { 0 };
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;
	PROCESS_INFORMATION pi;
	BOOL bRet = ::CreateProcess(NULL, run_path, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);

	HANDLE curProcessHandle = pi.hProcess; //获得当前进程的句柄

	LPVOID pDllPath = VirtualAllocEx(curProcessHandle, NULL, strlen(sDllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
	printf("[%s]pDllPath:0x%p\n", __FUNCTION__, pDllPath);

	WriteProcessMemory(curProcessHandle, pDllPath, sDllPath, strlen(sDllPath) + 1, NULL);

	PTHREAD_START_ROUTINE pfnLoadLib = (PTHREAD_START_ROUTINE)GetProcAddress(
		GetModuleHandle(TEXT("kernel32")), "LoadLibraryA");
	printf("[%s]pfnLoadLib:0x%p\n", __FUNCTION__, pfnLoadLib);

	//HANDLE hNewThread = CreateRemoteThread(curProcessHandle, NULL, 0, pfnLoadLib, pDllPath, 0, NULL);
	__BypassCreateRemoteThread BypassCreateRemoteThread = (__BypassCreateRemoteThread)GetProcAddress(
		GetModuleHandle(L"Kernel32"), "CreateRemoteThread");
	printf("[%s]BypassCreateRemoteThread:0x%p\n", __FUNCTION__, BypassCreateRemoteThread);

	HANDLE hNewThread = BypassCreateRemoteThread(curProcessHandle, NULL, 0, pfnLoadLib, pDllPath, 0, NULL);
	printf("[%s]GetLastError:%d\n", __FUNCTION__,GetLastError());
	printf("[%s]hNewThread:0x%p\n", __FUNCTION__, hNewThread);

	WaitForSingleObject(hNewThread, INFINITE);
	VirtualFreeEx(curProcessHandle, pDllPath, 0, MEM_RELEASE);
	CloseHandle(hNewThread);
	CloseHandle(curProcessHandle);
	ResumeThread(pi.hThread);
	printf("[%s]pi.hThread:0x%p\n", __FUNCTION__, pi.hThread);

	return true;

}

int main()
{
	char sDllPath[] = "MyPvzHack.dll";
	WCHAR run_path[] = L"PlantsVsZombies.exe";
	injectDll(sDllPath, run_path);
}