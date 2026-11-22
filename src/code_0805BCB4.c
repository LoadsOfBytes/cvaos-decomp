#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void TextWindow_Begin(s32);
extern s32 TextWindow_Update(void);
extern void Interaction_End(struct EwramData_EntityData *, s32);

/**
 * @brief Update of event scene variant 0 (sEventSceneUpdateFuncs[0]); returns the player
 * input mask. Cutscene object for conversation 0xC: places the player at
 * x <= 0x4F, then runs its own animation script in subState (jump away
 * to the right, returns 0x200 / 0x10 while moving). The two volatile
 * reads of subState reproduce the original's reloads.
 */
s32 EventScene00_Update(struct EwramData_EntityData *entity)
{
    struct EwramData **ewp;
    struct EwramData_EntityData *player;
    s32 ret;
    u8 step;

    ewp = &gEwramData;
    player = (*ewp)->playerState.player;
    ret = 0;
    switch (entity->state)
    {
    case 0:
    {
        u16 *px = &player->posX.part16.integer;
        if ((s16)*px > 0x4F)
            *px = 0x4F;
        player->flipX = 0;
        entity->flipX = 1;
        TextWindow_Begin(0xC);
        entity->state++;
        break;
    }
    case 1:
        if (entity->subState != 3 && TextWindow_Update())
        {
            (*ewp)->gameState.stateFlags &= ~1;
            entity->state++;
        }
        step = *(vu8 *)&entity->subState;
        if (step == 1)
            entity->state = 0x80;
        switch (*(vu8 *)&entity->subState)
        {
        case 1:
            entity->flipX = 0;
            break;
        case 2:
            Anim_StartIndexed(entity, 2, 2, 0);
            goto next;
        case 3:
            if (entity->animDone)
            {
                Anim_StartIndexed(entity, 3, 2, 0);
                entity->subState++;
                entity->velX.whole = -0x20000;
                entity->accelX.whole = 0x800;
            }
            break;
        case 4:
            if (entity->animStep == 1)
                entity->animFlags |= 2;
            entity->posX.whole += entity->velX.whole;
            entity->velX.whole += entity->accelX.whole;
            if (entity->velX.whole > -0x18000)
                ret = 0x200;
            if (entity->velX.whole > -0x1000)
            {
            next:
                entity->subState++;
            }
            break;
        case 5:
            Anim_StartIndexed(entity, 2, 2, 0);
            break;
        case 6:
            entity->posX.whole += 0x40000;
            entity->posY.whole += -0x8000;
            if ((s16)entity->posX.part16.integer > 0x100)
                entity->subState = step + 1;
            ret = 0x10;
            break;
        case 7:
            break;
        }
        break;
    case 0x80:
        Interaction_End(entity, 1);
        break;
    }
    return ret;
}
