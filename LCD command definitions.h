
// Commands to LCD module
#define LCD_CMD_CLEAR           0x01
#define LCD_CMD_HOME            0x02
#define LCD_CMD_ENTRY           0x04
#define LCD_CMD_DISPLAY         0x08
#define LCD_CMD_CD_SHIFT        0x10
#define LCD_CMD_FUNCTION        0x20
#define LCD_CMD_CGRAMADDR       0x40
#define LCD_CMD_SET_DDADDR      0x80

// Settings for LCD_CMD_ENTRY
#define LCD_ENTRY_MOVE_DISPLAY  0x01
#define LCD_ENTRY_MOVE_CURSOR   0x00
#define LCD_ENTRY_INC           0x02    
#define LCD_ENTRY_DEC           0x00

// Settings for LCD_CMD_DISPLAY
#define LCD_DISPLAY_BLINK       0x01
#define LCD_DISPLAY_NOBLINK     0x00
#define LCD_DISPLAY_CURSOR      0x02
#define LCD_DISPLAY_NOCURSOR    0x00
#define LCD_DISPLAY_ON          0x04
#define LCD_DISPLAY_OFF         0x00

// Settings for LCD_CMD_CD_SHIFT (shift cursor or display without changing data)
#define LCD_CD_SHIFT_RIGHT      0x04
#define LCD_CD_SHIFT_LEFT       0x00
#define LCD_CD_SHIFT_DISPLAY    0x08
#define LCD_CD_SHIFT_CURSOR     0x00

// Settings for LCD_CMD_FUNCTION
#define LCD_FUNCTION_5X10FONT   0x04
#define LCD_FUNCTION_5X8FONT    0x00
#define LCD_FUNCTION_2LINES     0x08
#define LCD_FUNCTION_1LINE      0x00
#define LCD_FUNCTION_8BIT       0x10
#define LCD_FUNCTION_4BIT       0x00

