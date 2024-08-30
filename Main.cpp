#include <windows.h>
#include <iostream>

using namespace std;

bool SetWriteProtection(bool enable);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool SetWriteProtection(bool enable) {
    HKEY hKey;
    LONG result;

    LPCWSTR registryPath = L"SYSTEM\\CurrentControlSet\\Control\\StorageDevicePolicies";

    result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, registryPath, 0, KEY_SET_VALUE, &hKey);

    if (result == ERROR_FILE_NOT_FOUND) {
        result = RegCreateKeyExW(HKEY_LOCAL_MACHINE, registryPath, 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL);
        if (result != ERROR_SUCCESS) {
            cerr << "Failed to create registry key. Error code: " << result << endl;
            return false;
        }
    }
    else if (result != ERROR_SUCCESS) {
        cerr << "Failed to open registry key. Error code: " << result << endl;
        return false;
    }

    DWORD value = enable ? 1 : 0;
    result = RegSetValueExW(hKey, L"WriteProtect", 0, REG_DWORD, (const BYTE*)&value, sizeof(value));

    if (result != ERROR_SUCCESS) {
        cerr << "Failed to set registry value. Error code: " << result << endl;
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    return true;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        CreateWindow(L"BUTTON", L"Enable Write Protection",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            50, 50, 200, 30, hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);
        CreateWindow(L"BUTTON", L"Disable Write Protection",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            50, 100, 200, 30, hwnd, (HMENU)2, GetModuleHandle(NULL), NULL);
        break;
    }
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        switch (wmId) {
        case 1:
            if (SetWriteProtection(true)) {
                MessageBox(hwnd, L"Write protection enabled successfully.", L"Success", MB_OK);
            }
            else {
                MessageBox(hwnd, L"Failed to enable write protection.", L"Error", MB_OK | MB_ICONERROR);
            }
            break;
        case 2:
            if (SetWriteProtection(false)) {
                MessageBox(hwnd, L"Write protection disabled successfully.", L"Success", MB_OK);
            }
            else {
                MessageBox(hwnd, L"Failed to disable write protection.", L"Error", MB_OK | MB_ICONERROR);
            }
            break;
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Write Protection Toggle",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 200,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
