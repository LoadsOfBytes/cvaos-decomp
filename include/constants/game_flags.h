#ifndef GAME_FLAGS_CONSTANTS_H
#define GAME_FLAGS_CONSTANTS_H

/*
 * Story and conversation flags of GameFlag_Set / GameFlag_GetMask /
 * GameFlag_TestAndSet / GameFlag_Clear (bit index into gameState.gameFlags,
 * so 0x20 is bit 0 of the second word). Names come from the event scene or
 * object that sets the flag; the conversation flags remember which line of
 * Hammer's or Mina's talk was already shown.
 */

#define FLAG_PROLOGUE_DONE              0x20 /* EventScenePrologueBattle; also set by the debug menus */
#define FLAG_MET_GRAHAM                 0x21 /* EventSceneMeetGraham */
#define FLAG_MET_YOKO                   0x22 /* EventSceneMeetYoko */
#define FLAG_MET_J                      0x23 /* EventSceneMeetJ */
#define FLAG_GRAHAM_CLAIMS_DRACULA      0x24 /* EventSceneGrahamClaimsDracula */
#define FLAG_YOKO_PLANS_TRAP            0x25 /* EventSceneYokoPlansTrap */
#define FLAG_YOKO_RESCUED               0x26 /* EventSceneRescueYoko */
#define FLAG_J_REGAINS_MEMORIES         0x27 /* EventSceneJRegainsMemories */
#define FLAG_GRAHAM_BATTLE_STARTED      0x28 /* EventSceneGrahamBattleStart; cleared again after an ending */
#define FLAG_JULIUS_BATTLE_STARTED      0x29 /* EventSceneJuliusBattleStart, Belmont_UpdateDefeat */
#define FLAG_JULIUS_BATTLE_ENDED        0x2A /* EventSceneJuliusBattleEnd */
#define FLAG_CHAOTIC_REALM_MESSAGES     0x2B /* EventSceneChaoticRealmMessages */
#define FLAG_RETURNED_HOME              0x2C /* EventSceneReturnHome */
#define FLAG_ENDING_SEEN                0x2D /* EventSceneEndingWakeUp, EventSceneJuliusModeEnding, EventSceneJuliusKeepsPromise */
#define FLAG_MET_HAMMER                 0x2E /* EventSceneMeetHammer */
#define FLAG_HAMMER_TALK_MET            0x2F /* EventSceneHammerTalk text 0x29 (after FLAG_MET_HAMMER) */
#define FLAG_HAMMER_TALK_TRAP           0x30 /* EventSceneHammerTalk text 0x2A (after FLAG_YOKO_PLANS_TRAP) */
#define FLAG_HAMMER_TALK_RESCUE         0x31 /* EventSceneHammerTalk text 0x2B (after FLAG_YOKO_RESCUED) */
#define FLAG_HAMMER_TALK_AWAKENED       0x32 /* EventSceneHammerTalk text 0x2C (after FLAG_SOMA_AWAKENED) */
#define FLAG_MINA_TALK_PROLOGUE         0x33 /* EventSceneMinaTalk text 0x2E (after FLAG_PROLOGUE_DONE) */
#define FLAG_MINA_TALK_HAMMER           0x34 /* EventSceneMinaTalk text 0x2F (after FLAG_MET_HAMMER) */
#define FLAG_MINA_TALK_RESCUE           0x35 /* EventSceneMinaTalk text 0x30 (after FLAG_YOKO_RESCUED) */
#define FLAG_MINA_TALK_AWAKENED         0x36 /* EventSceneMinaTalk text 0x43 (after FLAG_SOMA_AWAKENED) */
#define FLAG_MINA_TALK_GRAHAM           0x37 /* EventSceneMinaTalk text 0x53 (after FLAG_MET_GRAHAM) */
#define FLAG_UNCONSCIOUS_YOKO           0x38 /* EventSceneUnconsciousYoko */
#define FLAG_WARP_ROOM_USED             0x39 /* WarpRoom_Update (first warp); Mina skips talk entry 7 */
#define FLAG_SOMA_AWAKENED              0x3A /* EventSceneSomaAwakens (after Graham's defeat) */
#define FLAG_MINA_TALK_ITEMS            0x3B /* EventSceneMinaTalk with items 0x1A, 0x1B, 0x1C; set on a new game */
#define FLAG_FLOATING_GARDEN_ROOM1      0x3C /* RoomTransition_Update: entered room 1 of area 5 */
#define FLAG_FLOATING_GARDEN_ROOM5      0x3D /* RoomTransition_Update: entered room 5 of area 5; Mina skips talk entry 9 */

/* Flags read directly through gameFlags / gameFlagsTail. */
#define FLAG_NAME_NOUSE                 0x54 /* name code NOUSE: items cannot be used; kept on a new game */
#define FLAG_NAME_NOSOUL                0x55 /* name code NOSOUL; kept on a new game */
#define FLAG_HEADHUNTER_ROOM_CLEARED    0x72 /* set by the Headhunter defeat; room BG effect 19 hides BG2 */
#define FLAG_JULIUS_ROOM_CLEARED        0x79 /* set by the Julius (Belmont) defeat; room BG effect 10 */

/* Word index into gameState.gameFlags[] (gameFlagsTail[] starts at word 2) and bit mask of a flag. */
#define GAME_FLAG_WORD(flag) ((flag) >> 5)
#define GAME_FLAG_BIT(flag)  (1 << ((flag) & 0x1F))

#endif /* GAME_FLAGS_CONSTANTS_H */
