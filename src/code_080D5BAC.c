/* Delayed fragments become visible, accelerate downward and finish animating. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern u8 sCommonObjectGfx[],sDefeatParticleSpriteSheet[];
/**
 * @brief Wait for a random delay while hidden, then reveal, move under
 * acceleration and delete when the fragment animation completes.
 */
void GrahamBreakupFragment_Update(struct EwramData_EntityData *entity)
{
    switch(entity->state)
    {
        case 0:
        {
            s32 graphics=ObjGfx_Find(sCommonObjectGfx);
            Sprite_Attach(entity,sDefeatParticleSpriteSheet,graphics,6);
            Anim_StartIndexed(entity,0,0,0);
            entity->drawFunc=(u32 *)Sprite_DrawEntity;
            entity->spriteLayer=16;
            entity->objMode=1;
            entity->state++;
            entity->timer=RandomNumberGenerator()%32;
            entity->hidden=1;
        }
        case 1:
            if(--entity->timer==255)
            {
                entity->hidden=0;
                entity->state++;
            }
            break;
        case 2:
            EntityAccelerateAndMove(entity,0x80000);
            if(Anim_Update(entity)==3)
                EntityDelete(entity);
            break;
    }
}
