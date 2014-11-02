#include <windows.h>
#include <stdbool.h>
#include <winioctl.h>
#include <assert.h>
#include <stdint.h>
#include <signal.h>
#include "keyboard.h"

#define IOCTL_KEYBOARD_SET_INDICATORS        CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0002, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KEYBOARD_QUERY_TYPEMATIC       CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0008, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KEYBOARD_QUERY_INDICATORS      CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0010, METHOD_BUFFERED, FILE_ANY_ACCESS)


static HANDLE kbd;


void OpenKeyboardDevice()
{
  if(!DefineDosDevice(DDD_RAW_TARGET_PATH, "Kbd000000",
		      "\\Device\\KeyboardClass0"))
  {
    assert(false);
  }

  kbd = CreateFile("\\\\.\\Kbd000000", GENERIC_WRITE, 0,
		   NULL, OPEN_EXISTING, 0, NULL);
  assert(kbd);
}


void CloseKeyboardDevice()
{
  DefineDosDevice(DDD_REMOVE_DEFINITION, "Kbd000000", NULL);
  CloseHandle(kbd);
}



int toggle_led(bool toggle, int led)
{
  uint32_t input = 0, output = 0;

  DWORD len;
  if(!DeviceIoControl(kbd, IOCTL_KEYBOARD_QUERY_INDICATORS,
		      &input, sizeof(input),
		      &output, sizeof(output),
		      &len, NULL))
    return GetLastError();

  input = output;
  input &= ~(led << 16);
  if(toggle)
    input |= led << 16;

  if(!DeviceIoControl(kbd, IOCTL_KEYBOARD_SET_INDICATORS,
		      &input, sizeof(input),
		      NULL, 0,
		      &len, NULL))
    return GetLastError();
  return 0;
}

