#ifndef LAYER_OBJECT_H
#define LAYER_OBJECT_H

/*
 * Objects that move with BG layer 1 (objects 0x29, 0x2A, 0x32, 0x34 and the
 * 0x34 parts) keep their position in layer coordinates (screen position +
 * bgInfo[1] scroll) and derive posX / posY from it every frame; the screen
 * position is also saved for their draw functions.
 * Shuttle platforms (0x29 vertical, 0x2A horizontal; Object32Create fills the
 * same fields) move between the start coordinate and the end coordinate
 * (spawnParam0 << 16); shuttle dir toggles at each end (initially 1 when the
 * start lies beyond the end).
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define BG1_OBJECT_X(e) e->unk_4F4.whole
#define BG1_OBJECT_Y(e) e->unk_4F8.whole
#define BG1_OBJECT_DRAW_X(e) e->unk_508.whole
#define BG1_OBJECT_DRAW_Y(e) e->unk_50C.whole
#define SHUTTLE_START(e) e->unk_4FC.whole
#define SHUTTLE_END(e) e->unk_500.whole
#define SHUTTLE_DIR(e) e->unk_504.part8.unk_504

#endif /* LAYER_OBJECT_H */
