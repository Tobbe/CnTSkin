#include <windows.h>
#include <Tlhelp32.h>
#include <vector>
#include <string>

HWND mainHwnd;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HANDLE GetProcessHandle(LPSTR szExeName)
{
	PROCESSENTRY32 pe = { sizeof(PROCESSENTRY32) };
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
	HANDLE pHandle = NULL;

	if (hSnapshot == INVALID_HANDLE_VALUE) {
		MessageBox(NULL, "Error: unable to create toolhelp snapshot", "Loader", NULL);
		return false;
	}

	if (Process32First(hSnapshot, &pe)) {
		do {
			if (strcmp(pe.szExeFile, szExeName) == 0) {
				HANDLE token;
				if (OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &token)) {
					LUID luid;
					if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
						TOKEN_PRIVILEGES tp;
						tp.PrivilegeCount = 1;
						tp.Privileges[0].Luid = luid;
						tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
						if (AdjustTokenPrivileges(token, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
							pHandle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pe.th32ProcessID);
							if (pHandle != NULL) {
								return pHandle;
							}
						}
					}
				}
			}
		} while (Process32Next(hSnapshot, &pe));
	}

	return pHandle;
}

BOOL DllInject(HANDLE hProcess, LPSTR lpszDllPath)
{
	HMODULE hmKernel = GetModuleHandle("kernel32.dll");
	if (hmKernel == NULL || hProcess == NULL) {
		MessageBox(NULL, "Something's NULL", "DllInject", MB_OK);
		return FALSE;
	}

	int nPathLen = lstrlen(lpszDllPath) + 1;
	LPVOID lpvMem = VirtualAllocEx(hProcess, NULL, nPathLen, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
	WriteProcessMemory(hProcess, lpvMem, lpszDllPath, nPathLen, NULL);
	DWORD dwWaitResult, dwExitResult = 0;
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(hmKernel, "LoadLibraryA"), lpvMem, 0, NULL);

	if (hThread != NULL) {
		dwWaitResult = WaitForSingleObject(hThread, 10000);
		GetExitCodeThread(hThread, &dwExitResult);
		CloseHandle(hThread);
	}

	VirtualFreeEx(hProcess, lpvMem, 0, MEM_RELEASE);
	return ((dwWaitResult != WAIT_TIMEOUT) && (dwExitResult > 0));
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   PSTR szCmdLine, int iCmdShow)
{
	static char szAppName[] = "OSK-Launcher";
	HWND hwnd;
	MSG msg;

	WNDCLASSEX wndclass;
	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&wndclass);

	hwnd = CreateWindow(szAppName, "The Hello Program", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 300, 200,
		NULL, NULL, hInstance, NULL);

	mainHwnd = hwnd;

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	ShellExecute(NULL, "open", "C:\\Program\\Click-N-Type\\Click-N-Type.exe", "", "", SW_SHOW);

	HANDLE hProcess;
	do {
		hProcess = GetProcessHandle("Click-N-Type.exe");
		Sleep(1);
	} while (hProcess == NULL);

	if (!DllInject(hProcess, "C:\\Tobbe\\DevProjects\\C++\\CnTSkin\\Debug\\InjectionDLL.dll")) {
		MessageBox(HWND_DESKTOP, "An error occurred injecting DLL!", "Error!", MB_OK | MB_ICONERROR);
	}

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg) {
		case WM_CREATE: {
			HWND hwndBtn = CreateWindow("BUTTON","Press Me", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON
				,100,100,100,40,hwnd,NULL,NULL,NULL);

			HWND osk = FindWindow("OSKMainClass", NULL);
			//SetWindowLong(osk, GWL_STYLE, WS_BORDER | WS_CHILD);
			SetWindowPos(osk, HWND_BOTTOM, 400, 400, 705, 300, SWP_SHOWWINDOW | SWP_NOZORDER | SWP_FRAMECHANGED);
			HWND F1 = FindWindowEx(osk, NULL, "N1", "esc");
			InvalidateRect(F1, NULL, true);
			InvalidateRect(osk, NULL, true);

			return 0;
		}
		case WM_DESTROY:
			HWND cnt = FindWindow("ThunderRT6FormDC", NULL);
			if (cnt != NULL) {
				PostMessage(cnt, WM_CLOSE, 0, 0);
			}

			PostQuitMessage(0);

			return 0;
	}

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
} 