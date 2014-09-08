#include <windows.h>
#include <stdbool.h>

static const int vkey_scroll = 0x91, // scroll lock virtual key
        scan_scroll = 0x46, // scroll lock scancode
        en_us_layout = 0x0409; // en_US layout (from MSDN)

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
                // stop loading animation
                MSG msg;
                if(PostThreadMessage(GetCurrentThreadId(), 0, 0, 0))
                        GetMessage(&msg, 0, 0, 0);
        }

        HKL prev_hkl;
        DWORD prev_thread; // thread id
        bool prev_en_us = true;
        while(1)
        {
                Sleep(100);

                HWND cur_window = GetForegroundWindow();
                if(cur_window == NULL) // switching windows
                        continue;
                DWORD cur_thread = GetWindowThreadProcessId(cur_window, NULL);
                HKL cur_hkl = GetKeyboardLayout(cur_thread);
                bool en_us = LOWORD(cur_hkl) == en_us_layout;

#if 0 // method 1
                if(prev_en_us != en_us && cur_thread != prev_thread) // switched to other process with different layout
                {
                        // change fg window layout
                        PostMessage(cur_window,
                                    WM_INPUTLANGCHANGEREQUEST,
                                    0,
                                    (LPARAM)prev_hkl);
                        en_us = prev_en_us;
                        cur_hkl = prev_hkl;
                }
#else // method 2
                if(prev_en_us != en_us && cur_thread == prev_thread) // switched layout manually
                {
                        SystemParametersInfo(SPI_SETDEFAULTINPUTLANG, 0, &cur_hkl, 0);
                        
                        // send all windows message to switch their layout
                        // (by default windows sends it only to the current window (?))
                        PostMessage(HWND_BROADCAST,
                                    WM_INPUTLANGCHANGEREQUEST,
                                    0,
                                    (LPARAM)cur_hkl);
                }
#endif

                // setup led

                // press scroll only if changed layout and current led status is incorrect
                if(prev_en_us != en_us && prev_thread == cur_thread && en_us == get_scroll()) // en_us - scroll off, else on
                {
                        press_scroll();
                }

 
                prev_en_us = en_us;
                prev_thread = cur_thread;
                prev_hkl = cur_hkl;
                
        }
        return 0;
}


