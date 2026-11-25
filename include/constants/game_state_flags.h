#ifndef GAME_STATE_FLAGS_CONSTANTS_H
#define GAME_STATE_FLAGS_CONSTANTS_H

/*
 * Bits of gameState.stateFlags (runtime state, not saved) whose meaning is
 * clear from the code that sets and tests them. Unnamed bits stay numeric.
 */
#define STATE_EVENT_RUNNING         0x00000001 /* scene, door, save point, shop or text interaction holds the player */
#define STATE_BOSS_FIGHT            0x00000002 /* set by the boss create functions, cleared on the defeat */
#define STATE_BOSS_DEFEAT_TILES     0x00000008 /* BossDefeat_LoadEffectTiles: effect tiles replace VRAM, restore pending */
#define STATE_PLAYER_DEAD           0x00000010 /* Player_ApplyContactDamage; boss defeat effects stop */
#define STATE_HUD_HIDDEN            0x00000020 /* text windows and ending scenes */
#define STATE_WARP_ACTIVE           0x00000040 /* WarpRoom_Update */
#define STATE_AREA_NAME_SHOWN       0x00000080 /* area name banner on screen (AreaName_Update) */
#define STATE_DOOR_ACTIVE           0x00000100 /* gates and boss doors while they open or close */
#define STATE_HUD_MESSAGES_BLOCKED  0x00000200 /* event text layer: no enemy names / pickup messages */
#define STATE_BOSS_RUSH_CLEARED     0x00000400 /* end-of-run pickup (subState bit 0) collected; BossRushResultUpdate starts */
#define STATE_ROOM_EFFECT           0x00000800 /* RoomEffectCreate; Room_Load keeps the effect */
#define STATE_JULIUS_ARENA          0x00001000 /* Julius fight: arena props react (Belmont_UpdateCombat) */
#define STATE_ENDING_GRAHAM         0x00002000 /* Graham defeated: EventSceneReturnHome ending */
#define STATE_ENDING_CHAOS          0x00004000 /* Chaos defeated: EventSceneEnding, credits, EventSceneEndingWakeUp */
#define STATE_ENDING_JULIUS         0x00008000 /* Graham defeated in Julius mode: EventSceneJuliusModeEnding */
#define STATE_ENDING_SEQUENCE       0x00010000 /* ending scenes running; story scenes and Graham do not start */
#define STATE_WARP_PENDING          0x00020000 /* WarpRoom_Update -> GameModeInGameUpdate jumps rooms */
#define STATE_MUSIC_COMMAND_SENT    0x00040000 /* Music_SubmitCommandOnce already faded the music; room music and time stop keep off */
#define STATE_ENDING_SCENE          0x00080000 /* set by EventSceneEnding_Init; never tested */
#define STATE_ROOM_TRANSITION       0x00100000 /* RoomTransition_Begin .. RoomTransition_Update */
#define STATE_CHAOS_FIGHT           0x00200000 /* EnemyChaosCreate, Chaos_StartSecondForm */
#define STATE_BOSS_DOOR_KEEPS_MUSIC 0x00400000 /* after the Julius fight: opening boss doors does not fade the music */
#define STATE_ROOM_JUMP_NO_PAUSE    0x00800000 /* the pending room jump leaves start/pause disabled (RoomJump_Update) */
#define STATE_LARGE_WINDOW_OPEN     0x01000000 /* Hud_ShowLargeSoulWindow */
#define STATE_MESSAGE_WINDOW_CLOSED 0x02000000 /* Hud_CloseMessageWindow */

/*
 * Bits of gameState.bossEventFlags (saved): one per defeated boss, set by
 * the boss recovery pickup (1 << event) or the boss itself and tested by
 * its create function (no respawn), doors, music, shops and conversations.
 */
#define BOSS_DEFEATED_GRAHAM        0x0001
#define BOSS_DEFEATED_DEATH         0x0002
#define BOSS_DEFEATED_JULIUS        0x0004
#define BOSS_DEFEATED_HEADHUNTER    0x0010
#define BOSS_DEFEATED_LEGION        0x0020
#define BOSS_DEFEATED_BALORE        0x0040
#define BOSS_DEFEATED_CHAOS         0x0080
#define BOSS_DEFEATED_MINOTAUR      0x0100
#define BOSS_DEFEATED_MANTICORE     0x0400
#define BOSS_DEFEATED_CREAKING_SKULL 0x0800
#define BOSS_DEFEATED_BIG_GOLEM     0x1000
#define BOSS_DEFEATED_MAN_EATER     0x2000

#endif /* GAME_STATE_FLAGS_CONSTANTS_H */
