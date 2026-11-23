#ifndef PALETTE_STATE_H
#define PALETTE_STATE_H

/* Functions of engine/palette_state.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void Palette_SaveState(void);
extern void Palette_RestoreState(void);

#endif /* PALETTE_STATE_H */
