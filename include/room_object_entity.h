#ifndef ROOM_OBJECT_ENTITY_H
#define ROOM_OBJECT_ENTITY_H

/*
 * Room objects keep their work slots in ext.object<NN> / ext.warpRoom /
 * ext.warpGate / ext.bossRushTimer (include/structs/entity_work.h).
 * Object 0x35 (floor switch) also reads the integer halves of its 16.16 home
 * position (ext.floorSwitchGate.homeX / homeY, room pixels) directly.
 * The macros expand to the original member chains without added parentheses
 * (token-identical preprocessed code); use them with a plain base expression.
 */
#define OBJECT35_HOME_PIXEL_X(e) e->ext.raw.unk_4F4.part16.unk_4F6
#define OBJECT35_HOME_PIXEL_Y(e) e->ext.raw.unk_4F8.part16.unk_4FA

#endif /* ROOM_OBJECT_ENTITY_H */
