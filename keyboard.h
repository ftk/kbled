#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>

#define KEYBOARD_CAPS_LOCK_ON     4
#define KEYBOARD_NUM_LOCK_ON      2
#define KEYBOARD_SCROLL_LOCK_ON   1


void OpenKeyboardDevice();
void CloseKeyboardDevice();
int toggle_led(bool toggle, int led);


#endif /* KEYBOARD_H */
