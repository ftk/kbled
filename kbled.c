#include <windows.h>
#include <stdbool.h>

static const int vkey_scroll = 0x91,
        scan_scroll = 0x46;

static void press_scroll()
{
        keybd_event(vkey_scroll, scan_scroll, KEYEVENTF_EXTENDEDKEY, 0);
        keybd_event(vkey_scroll, scan_scroll, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
}

static bool get_scroll()
{
        return GetKeyState(vkey_scroll) & 1;
}


int main(void)
{
        {
                MSG msg;
                if(PostThreadMessage(GetCurrentThreadId(), 0, 0, 0))
                        GetMessage(&msg, 0, 0, 0);
        }
        
        while(1)
        {
                bool en_us = LOWORD(GetKeyboardLayout(GetWindowThreadProcessId(GetForegroundWindow(), NULL))) == 0x0409;
                if(en_us == get_scroll()) // en_us - scroll off, else on
                {
                        press_scroll();
                }
                Sleep(200);
        }
        return 0;
}



