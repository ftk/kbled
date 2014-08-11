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
                        //ActivateKeyboardLayout(prev_hkl, KLF_ACTIVATE | KLF_SETFORPROCESS); - doesn't work (changes for our process probably)
                        
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
                        // send all windows message to switch their layout
                        // (by default windows sends it only to the current window (?))
                        PostMessage(HWND_BROADCAST,
                                    WM_INPUTLANGCHANGEREQUEST,
                                    0,
                                    (LPARAM)cur_hkl);
                }
#endif

                static bool tried_pressing_scroll = false;
                if(prev_en_us != en_us && prev_thread == cur_thread)
                        tried_pressing_scroll = false;
                // setup light
                if(!tried_pressing_scroll && en_us == get_scroll()) // en_us - scroll off, else on
                {
                        press_scroll();
                        tried_pressing_scroll = true; // dont spam scroll presses if fg program doesnt handle it correctly (emacs...)
                }

 
                prev_en_us = en_us;
                prev_thread = cur_thread;
                prev_hkl = cur_hkl;
                
        }
        return 0;
}


