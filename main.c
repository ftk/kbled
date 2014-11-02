#include <windows.h>
#include <stdbool.h>
#include <assert.h>
#include <signal.h>
#include "keyboard.h"
#include <string.h>

static const int en_us_layout = 0x0409; // en_US layout (from MSDN)


static bool has_locks_changed()
{
  static bool num_state, caps_state, scroll_state;
  bool new_num_state = GetKeyState(VK_NUMLOCK) & 1,
    new_caps_state = GetKeyState(VK_CAPITAL) & 1,
    new_scroll_state = GetKeyState(VK_SCROLL) & 1;
  bool changed = num_state != new_num_state
    || caps_state != new_caps_state
    || scroll_state != new_scroll_state;
  num_state = new_num_state;
  caps_state = new_caps_state;
  scroll_state = new_scroll_state;
  return changed;
}



int main(int argc, char * argv[])
{
  {
    // stop loading animation
    MSG msg;
    if(PostThreadMessage(GetCurrentThreadId(), 0, 0, 0))
      GetMessage(&msg, 0, 0, 0);
  }

  int global_switch = 0;
  int indicator = 0;

  for(int arg = 1; arg < argc; arg++)
  {
    if(!strcmp(argv[arg], "--scroll"))
      indicator |= KEYBOARD_SCROLL_LOCK_ON;
    else if(!strcmp(argv[arg], "--num"))
      indicator |= KEYBOARD_NUM_LOCK_ON;
    else if(!strcmp(argv[arg], "--caps"))
      indicator |= KEYBOARD_CAPS_LOCK_ON;
    else if(!strcmp(argv[arg], "--global1"))
      global_switch = 1;
    else if(!strcmp(argv[arg], "--global2"))
      global_switch = 2;
  }
  if(!indicator && !global_switch)
  {
    // no args
    return 1;
  }

  if(indicator)
  {
    // open keyboard
    OpenKeyboardDevice();

    signal(SIGTERM, &CloseKeyboardDevice);
    signal(SIGINT, &CloseKeyboardDevice);
  }

  HKL prev_hkl = 0;
  DWORD prev_thread = 0; // thread id
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

    if(global_switch == 1) // method 1
    {
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
    }
    else if(global_switch == 2)// method 2
    {
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
    }

    if(indicator)
    {
      // setup led

      // press scroll only if changed layout

      static bool change = false; // dirty hack
      if((prev_en_us != en_us && prev_thread == cur_thread) || change)
      {
	toggle_led(!en_us, indicator);
      }
      change = has_locks_changed();
    }

    prev_en_us = en_us;
    prev_thread = cur_thread;
    prev_hkl = cur_hkl;
  }
  // unreachable
}
