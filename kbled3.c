#include <windows.h>
#include <stdbool.h>
#include <winioctl.h>
#include <assert.h>
#include <stdint.h>
#include <signal.h>

static const int en_us_layout = 0x0409; // en_US layout (from MSDN)




HANDLE OpenKeyboardDevice()
{
        if(!DefineDosDevice(DDD_RAW_TARGET_PATH, "Kbd000000",
                            "\\Device\\KeyboardClass0"))
        {
                assert(false);
        }

        return CreateFile("\\\\.\\Kbd000000", GENERIC_WRITE, 0,
                          NULL, OPEN_EXISTING, 0, NULL);
}


int CloseKeyboardDevice(HANDLE hndKbdDev)
{
        int e = 0;

        if(!DefineDosDevice(DDD_REMOVE_DEFINITION, "Kbd000000", NULL))
                e = GetLastError();

        if(!CloseHandle(hndKbdDev))
                e = GetLastError();

        return e;
}

#define IOCTL_KEYBOARD_SET_INDICATORS        CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0002, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KEYBOARD_QUERY_TYPEMATIC       CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0008, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KEYBOARD_QUERY_INDICATORS      CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0010, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define KEYBOARD_CAPS_LOCK_ON     4
#define KEYBOARD_NUM_LOCK_ON      2
#define KEYBOARD_SCROLL_LOCK_ON   1


int toggle_scroll(HANDLE kbddev, bool toggle)
{
        uint32_t input = 0, output = 0;

        DWORD len;
        if(!DeviceIoControl(kbddev, IOCTL_KEYBOARD_QUERY_INDICATORS,
                            &input, sizeof(input),
                            &output, sizeof(output),
                            &len, NULL))
                return GetLastError();

        input = output;
        input &= ~(KEYBOARD_SCROLL_LOCK_ON << 16);
        if(toggle)
                input |= KEYBOARD_SCROLL_LOCK_ON << 16;

        if(!DeviceIoControl(kbddev, IOCTL_KEYBOARD_SET_INDICATORS,
                             &input, sizeof(input),
                             NULL, 0,
                             &len, NULL))
                return GetLastError();
        return 0;
}

static HANDLE kbd;

void signal_handler(int sig)
{
        CloseKeyboardDevice(kbd);
}

int main(void)
{
        {
                // stop loading animation
                MSG msg;
                if(PostThreadMessage(GetCurrentThreadId(), 0, 0, 0))
                        GetMessage(&msg, 0, 0, 0);

        }
        // open keyboard
        kbd = OpenKeyboardDevice();
        assert(kbd);

        signal(SIGTERM, &signal_handler);
        signal(SIGINT, &signal_handler);


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
                if(prev_en_us != en_us && prev_thread == cur_thread) // en_us - scroll off, else on
                {
                        int status = toggle_scroll(kbd, !en_us);
                        assert(status == 0);
                }

 
                prev_en_us = en_us;
                prev_thread = cur_thread;
                prev_hkl = cur_hkl;
                
        }

        CloseKeyboardDevice(kbd);
        return 0;
}


