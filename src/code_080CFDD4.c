/* Belmont scene setup and player position at the room boundary. */
#include "code_08009A0.h"
#include "belmont_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *,u8 *,void *,u8,void *);
extern void Belmont_CreateSceneObject(struct EwramData_EntityData *,s32);
extern u8 sJuliusEntityPalette[],sJuliusSpriteSheet[],sBelmontGfx[];
#define PLAYER (gEwramData->playerState.player)
/**
 * @brief Scene state (13): load the display animation, create scene entry
 * 2 at the player boundary and hold the player there while the object
 * remains active.
 */
void Belmont_UpdateSceneBoundary(struct EwramData_EntityData *entity)
{
    void *palette=sJuliusEntityPalette;
    void *sprite=sJuliusSpriteSheet;
    u8 *graphics=sBelmontGfx;
    struct BgLayerState *bg=&gEwramData->bgInfo[1];
    u16 boundary=192-bg->xPos.part16.integer;
    {
        s32 state=entity->subState;
        if(state==1) goto active;
        if(state>1) return;
        if(state!=0) return;
            if(!Entity_LoadGraphics(entity,graphics,palette,0,sprite))
            {
                EntityDelete(entity);
                return;
            }
            if(entity->animScriptIndex!=4)
                Anim_StartIndexed(entity,4,1,0);
            Anim_Update(entity);
            entity->spriteLayer=17;
            entity->subState++;
active:
            if((s16)PLAYER->posX.part16.integer<(s16)boundary)
            {
                struct EwramData_EntityData *object;
                if(entity->stage==0)
                {
                    Belmont_CreateSceneObject(entity,2);
                    entity->stage++;
                }
                object=&gEwramData->entityData[BELMONT_SCENE_OBJECT_INDEX(entity)];
                if(object->updateFunc!=NULL)
                {
                    PLAYER->posX.part16.integer=boundary;
                    PLAYER->velX.whole=0;
                    PLAYER->accelX.whole=0;
                    PLAYER->flipX=1;
                }
            }
    }
}

