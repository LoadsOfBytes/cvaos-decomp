#ifndef AGB_PRINT_H
#define AGB_PRINT_H

/* Functions of agb_print.c used by other files. */

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void AGBAssert(const char *pFile, int nLine, const char *pExpression, int nStopProgram);

#endif /* AGB_PRINT_H */
