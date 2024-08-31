/**
  @file smallinput.h

  Small API for getting keyboard/mouse input, with possiblity to record it and
  play back.

  The Linux Input API requires root pirivileges (sudo).

  by Milsolav "drummyfish" Ciz, released under CC0 1.0 (public domain)
*/

#ifndef _SMALLINPUT_H
#define _SMALLINPUT_H

#include <stdint.h>
#include <stdio.h>

#define SMALLINPUT_MODE_NORMAL 0 ///< Only handle input.
#define SMALLINPUT_MODE_RECORD 1 ///< Handle input and record it.
#define SMALLINPUT_MODE_PLAY   2 ///< Play back recorded input.

#define SMALLINPUT_KEY_NONE    0
#define SMALLINPUT_SPACE       ' '
#define SMALLINPUT_BACKSPACE   8
#define SMALLINPUT_TAB         9
#define SMALLINPUT_RETURN      13
#define SMALLINPUT_SHIFT       14
#define SMALLINPUT_ESCAPE      27
#define SMALLINPUT_DELETE      127
#define SMALLINPUT_ARROW_UP    128
#define SMALLINPUT_ARROW_RIGHT 129
#define SMALLINPUT_ARROW_DOWN  130
#define SMALLINPUT_ARROW_LEFT  131
#define SMALLINPUT_F1          132
#define SMALLINPUT_F2          133
#define SMALLINPUT_F3          134
#define SMALLINPUT_F4          135
#define SMALLINPUT_F5          136
#define SMALLINPUT_F6          137
#define SMALLINPUT_F7          138
#define SMALLINPUT_F8          139
#define SMALLINPUT_F9          140
#define SMALLINPUT_F10         141
#define SMALLINPUT_F11         142
#define SMALLINPUT_F12         143
#define SMALLINPUT_CTRL        144
#define SMALLINPUT_MOUSE_L     253
#define SMALLINPUT_MOUSE_M     254
#define SMALLINPUT_MOUSE_R     255

#define SMALLINPUT_RECORD_KEY_DOWN 1   ///< Mouse down event, followed by code.
#define SMALLINPUT_RECORD_KEY_UP   2   ///< Moue up event, followed by code.
#define SMALLINPUT_RECORD_MOUSE_X  3   ///< Mouse x move, followed by s32 value.
#define SMALLINPUT_RECORD_MOUSE_Y  4   ///< Mouse y move, followed by s32 value.
#define SMALLINPUT_RECORD_END      255 ///< Record end, followed by 4 more same values.

uint8_t input_keyStates[256];
int32_t input_mousePosition[2];

uint8_t input_keyStatesPrevious[256];
int32_t input_mousePositionPrevious[2];

uint8_t input_mode;

uint32_t input_frame = 0;
uint8_t *input_recordData;
uint32_t input_recordPosition;
uint32_t input_recordSize;

#if 1 // TODO: add other options for input handling (SDL, xinput, ...)
  /*
    This is using Linux Input Subsystem API. Defines can be found in
    include/uapi/linux/input-event-codes.h.
  */

  #include <fcntl.h>
  #include <linux/input.h>
  #include <sys/time.h>
  #include <unistd.h>

  typedef struct
  {
    struct timeval time;
    uint16_t type;
    uint16_t code;
    int32_t value;
  } LinuxInputEvent;

  #define INPUT_KEYBOARD_FILE "/dev/input/event0"
  #define INPUT_MOUSE_FILE "/dev/input/event1"

  int input_keyboardFile = 0;
  int input_mouseFile = 0;

  /**
    Maps this library's key codes to linux input key codes.
  */
  static const int input_linuxCodes[256] =
    {
      #define no KEY_RESERVED
      no,no,no,no,no,no,no,no,KEY_BACKSPACE,KEY_TAB,no,no,no,KEY_ENTER,KEY_LEFTSHIFT,no,
      no,no,no,no,no,no,no,no,no,no,no,KEY_ESC,no,no,no,no,
      KEY_SPACE,no,no,no,no,no,no,no,no,no,no,no,KEY_COMMA,no,KEY_DOT,no,
      KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9,no,KEY_SEMICOLON,no,KEY_EQUAL,no,KEY_QUESTION,
      no,KEY_A,KEY_B,KEY_C,KEY_D,KEY_E,KEY_F,KEY_G,KEY_H,KEY_I,KEY_J,KEY_K,KEY_L,KEY_M,KEY_N,KEY_O,
      KEY_P,KEY_Q,KEY_R,KEY_S,KEY_T,KEY_U,KEY_V,KEY_W,KEY_X,KEY_Y,KEY_Z,no,no,no,no,no,
      no,KEY_A,KEY_B,KEY_C,KEY_D,KEY_E,KEY_F,KEY_G,KEY_H,KEY_I,KEY_J,KEY_K,KEY_L,KEY_M,KEY_N,KEY_O,
      KEY_P,KEY_Q,KEY_R,KEY_S,KEY_T,KEY_U,KEY_V,KEY_W,KEY_X,KEY_Y,KEY_Z,no,no,no,no,KEY_DELETE,
      KEY_UP,KEY_RIGHT,KEY_DOWN,KEY_LEFT,KEY_F1,KEY_F2,KEY_F3,KEY_F4,KEY_F5,KEY_F6,KEY_F7,KEY_F8,KEY_F9,KEY_F10,KEY_F11,KEY_F12,
      KEY_LEFTCTRL,no,no,no,no,no,no,no,no,no,no,no,no,no,no,no,
      no,no,no,no,no,no,no,no,no,no,no,no,no,no,no,no,
      no,no,no,no,no,no,no,no,no,no,no,no,no,no,no,no,
      no,no,no,no,no,no,no,no,no,no,no,no,no,no,no,no,
      no,no,no,no,no,no,no,no,no,no,no,no,no,no,no,no,
      no,no,no,no,no,no,no,no,no,no,no,no,no,no,no,no,
      no,no,no,no,no,no,no,no,no,no,no,no,no,no,no,no
      #undef no
    };
#endif

void input_recordU8(uint8_t value)
{
  if (input_recordPosition < (input_recordSize - 1))
  {
    input_recordData[input_recordPosition] = value;
    input_recordPosition++;
  }
}

void input_recordU32(uint32_t value)
{
  for (uint8_t i = 0; i < 4; ++i)
  {
    input_recordU8(value % 256);
    value /= 256;
  }
}

uint32_t input_readU32(uint8_t *data)
{
  uint32_t result = 0;

  for (uint8_t i = 0; i < 4; ++i)
    result = result * 256 + data[3 - i];

  return result;
}

/**
  Initializes the library with given mode (SMALLINPUT_MODE_*).
*/
uint8_t input_init(uint8_t mode, uint8_t *recordData, uint32_t recordDataLength)
{
  input_mode = mode;

  input_mousePosition[0] = 0;
  input_mousePosition[1] = 0;

  input_mousePositionPrevious[0] = 0;
  input_mousePositionPrevious[1] = 0;

  input_frame = 0;
  input_recordData = recordData;
  input_recordPosition = 0;
  input_recordSize = recordDataLength;

  for (int16_t i = 0; i < 256; ++i)
  {
    input_keyStates[i] = 0;
    input_keyStatesPrevious[i] = 0;
  }

  uint8_t result = 1;

  if (input_mode != SMALLINPUT_MODE_PLAY)
  {
    input_keyboardFile = open(INPUT_KEYBOARD_FILE, O_RDONLY);

    result = input_keyboardFile >= 0;

    if (result)
    {
      fcntl(input_keyboardFile, F_SETFL, O_NONBLOCK); 

      input_mouseFile = open(INPUT_MOUSE_FILE, O_RDONLY);
    
      result = input_mouseFile >= 0;
   
      if (result)
        fcntl(input_mouseFile, F_SETFL, O_NONBLOCK); 
    }

    if (!result)
      puts("could not open device file (are you root?)");
  }

  return result;
}

void input_end(void)
{
  if (input_mode == SMALLINPUT_MODE_RECORD)
    for (uint8_t i = 0; i < 5; ++i)
      input_recordU8(SMALLINPUT_RECORD_END);

  close(input_keyboardFile);
  close(input_mouseFile);
}

/**
  Should be called once every main loop iteration to retrieve current input
  state.
*/
void input_update(void)
{
  LinuxInputEvent event;

  if (input_mode == SMALLINPUT_MODE_PLAY)
  {
    while (input_recordPosition < input_recordSize)
    {
      uint32_t nextFrame = input_readU32(input_recordData + input_recordPosition);

      if (input_frame >= nextFrame)
      {
        input_recordPosition += 4;

        uint8_t rec = input_recordData[input_recordPosition];

        switch (rec)
        { 
          case SMALLINPUT_RECORD_KEY_DOWN: 
          case SMALLINPUT_RECORD_KEY_UP:
            input_recordPosition++;
            input_keyStates[input_recordData[input_recordPosition]] = rec == SMALLINPUT_RECORD_KEY_DOWN; 
            input_recordPosition++;
            break;

          case SMALLINPUT_RECORD_MOUSE_X: 
          case SMALLINPUT_RECORD_MOUSE_Y:
            input_recordPosition++;
            input_mousePosition[rec == SMALLINPUT_RECORD_MOUSE_Y] = 
              input_readU32(input_recordData + input_recordPosition);
            input_recordPosition += 4;
            break;

          case SMALLINPUT_RECORD_END:
            input_recordPosition = input_recordSize;
            break;

          default: /*printf("corrupt record\n");*/ break;
        }
      }
      else
        break;
    }
  }
  else
  {
    while (1) // keyboard
    {
      if (read(input_keyboardFile, &event, sizeof(event)) <= 0)
        break;

      if (event.type == EV_KEY && (event.value == 1 || event.value == 0))
        for (uint16_t i = 0; i < 256; ++i)
          if (event.code == input_linuxCodes[i])
          {
            input_keyStates[i] = event.value;
            break;
          }
    }

    while (1) // mouse
    {
      if (read(input_mouseFile, &event, sizeof(event)) <= 0)
        break;

      if (event.type == EV_REL)
        input_mousePosition[event.code % 2] += event.value;
      else if (event.type == EV_KEY)
      {
        input_keyStates[  
          event.code == BTN_LEFT ? SMALLINPUT_MOUSE_L :
          (event.code == BTN_RIGHT ? SMALLINPUT_MOUSE_R : SMALLINPUT_MOUSE_M)]
          = event.value; 
      }
    }
  }

  for (uint16_t i = 0; i < 256; ++i)
    if (input_keyStates[i] && input_keyStates[i] < 255)
      input_keyStates[i]++;

  if (input_mode == SMALLINPUT_MODE_RECORD)
  {
    for (uint8_t i = 0; i < 2; ++i) // record mouse events
      if (input_mousePositionPrevious[i] != input_mousePosition[i])
      {
        input_recordU32(input_frame + 1);
        input_recordU8((i == 0) ? SMALLINPUT_RECORD_MOUSE_X : SMALLINPUT_RECORD_MOUSE_Y);
        input_recordU32(input_mousePosition[i]);
    
        input_mousePositionPrevious[i] = input_mousePosition[i];
      }

    for (uint16_t i = 0; i < 256; ++i) // record key events
    {
      uint8_t a = input_keyStates[i] > 0;
      uint8_t b = input_keyStatesPrevious[i] > 0;

      if (a != b)
      {
        input_recordU32(input_frame + 1);
        input_recordU8(a ? SMALLINPUT_RECORD_KEY_DOWN : SMALLINPUT_RECORD_KEY_UP);
        input_recordU8(i);

        input_keyStatesPrevious[i] = input_keyStates[i];
      }
    }
  }

  input_frame++;
}

/**
  Returns the number of input frames for which given key has been pressed (> 1:
  key is pressed, == 1: key was just pressed, == 0: key is not pressed).
*/
static inline uint8_t input_getKey(uint8_t key)
{
  if (key >= 'a' && key <= 'z')
    key = 'A' + (key - 'a');

  return input_keyStates[key];
}

/**
  Gets the mouse position.
*/
static inline void input_getMousePos(int32_t *x, int32_t *y)
{
  *x = input_mousePosition[0];
  *y = input_mousePosition[1];
}

static inline void input_setMousePos(int32_t x, int32_t y)
{
  input_mousePosition[0] = x;
  input_mousePosition[1] = y;
}

/**
  Prints the current input state.
*/
void input_print()
{
  printf("frame: %d\nmouse pos: %d %d",input_frame,input_mousePosition[0],input_mousePosition[1]);

  for (uint16_t i = 0; i < 256; ++i)
  {
    if (i % 8 == 0)
      putchar('\n');

    char c = (i > ' ' && i <= 126) ? i : '?';

    uint8_t n = input_getKey(i);

    printf("%s",n ? " [" : "  ");
    printf("%03d (\'%c\'): %03d",i,c,input_getKey(i));
    printf("%s",n ? "] " : "  ");
  }

  putchar('\n');
}

void input_printRecord()
{
  for (uint32_t i = 0; i < input_recordPosition; ++i)
  {
    if (i % 32 == 0)
      putchar('\n');

    printf("%d,",input_recordData[i]);
  }
}

uint32_t input_hash()
{
  uint32_t result = 0;

  for (uint16_t i = 0; i < 256; ++i)
    result += input_getKey(i) * (i + 1);

  int32_t x, y;

  input_getMousePos(&x,&y);

  result += x + y << 16;

  return result;
}

#endif // guard
