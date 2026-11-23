#ifndef TEXT_WINDOW_H
#define TEXT_WINDOW_H

/* Functions of menu/text_window.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void TextWindow_Close(void);
extern void TextWindow_Begin(s32 textId);
extern s32 TextWindow_Update(void);

#endif /* TEXT_WINDOW_H */
