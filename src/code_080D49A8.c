/* Linked scene part: wait for the scene object, replace graphics,
 * animate the transition and coordinate its partner. */
#include "code_080D73B8.h"
#include "graham_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Graham_CreateSceneObject(struct EwramData_EntityData *, s32);
extern s32 ObjPalette_FindOrLoadVariant(void *, s32);
extern void GrahamFirstForm_CreateTransitionPart(struct EwramData_EntityData *);
extern u8 sGrahamFirstFormGfx[], sGrahamPalette2[], sGrahamFirstFormSpriteSheet[];

/**
 * @brief Transition state (5): coordinate scene objects, replace graphics
 * and animate the transformation with a linked part. Set root flag 4
 * when that part finishes, free graphics and wait for its removal.
 */
void GrahamFirstForm_UpdateTransition(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part = &gEwramData->entityData[GRAHAM_CHILD_INDEX(entity)];
    struct EwramData_EntityData *owner = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    struct EwramData_EntityData *object = &gEwramData->entityData[(u8)GRAHAM_SCENE_OBJECT_INDEX(owner)];
    switch (entity->subState)
    {
    case 0:
        switch (entity->stage)
        {
        case 0:
            entity->timer = 0x20;
            entity->stage++;
        case 1:
            if (--entity->timer == 0xFF)
                entity->stage++;
            break;
        case 2:
            {
                s32 created;
                if (GRAHAM_FLAGS(owner) & 8)
                    created = Graham_CreateSceneObject(owner, 2);
                else
                    created = Graham_CreateSceneObject(owner, 6);
                if (created)
                    entity->stage++;
                else
                    entity->stage += 2;
            }
            if (entity->animScriptIndex != 0)
                Anim_StartIndexed(entity, 0, 0, 1);
            entity->timer = 0x40;
            break;
        case 3:
            if (object != NULL && (!object->updateFunc || object->unk_518.part8.enemyId != 0x20))
            {
                entity->subState = 1;
                entity->stage = 0;
            }
            break;
        case 4:
            if (--entity->timer == 0xFF)
            {
                entity->subState = 1;
                entity->stage = 0;
            }
            break;
        }
        break;
    case 1:
        {
            s32 i;
            s32 graphics;
            u8 *palette, *animation, *graphicsData;
            s8 slot;
            for (i = 0; i <= 3; i++)
            {
                gEwramData->gameState.sharedObjPaletteSources[i] = 0;
                gEwramData->gameState.sharedObjPaletteVariants[i] = 0;
            }
            Sprite_Free(entity, 1);
            graphicsData = sGrahamFirstFormGfx;
            palette = sGrahamPalette2;
            animation = sGrahamFirstFormSpriteSheet;
            graphics = ObjGfx_LoadResource(graphicsData, 1);
            if (graphics >= 0)
            {
                slot = ObjPalette_FindOrLoadVariant(palette, 6);
                if (slot >= 0)
                {
                    entity->unk_510.part8.basePalette = slot;
                    Sprite_Attach(entity, animation, graphics, entity->unk_510.part8.basePalette);
                    entity->drawFunc = (u32 *)Sprite_DrawEntity;
                }
            }
            Anim_StartIndexed(entity, 0, 1, 0);
            Anim_Update(entity);
            entity->subState = 2;
            entity->stage = 0;
            GrahamFirstForm_CreateTransitionPart(entity);
            entity->posY.part16.integer -= 0x1A;
            PlaySong(0x1DC);
        }
        break;
    case 2:
        if (entity->animStep == 0x11)
            ObjPalette_LoadRange(sGrahamPalette2, 7, 1, entity->palette);
        if (entity->animStep == 0x19)
        {
            PlaySong(0xFE);
            ObjPalette_LoadRange(sGrahamPalette2, 8, 1, entity->palette);
        }
        if (entity->animStep == 0x21)
            ObjPalette_LoadRange(sGrahamPalette2, 5, 1, entity->palette);
        if (entity->animStep <= 0x17 && entity->animTimer == 0)
            entity->posY.part16.integer--;
        if (entity->animStep == 0x29)
            part->hidden = 0;
        if (entity->animDone)
        {
            entity->hidden = 1;
            entity->subState = 3;
            entity->stage = 0;
        }
        break;
    case 3:
        if (part->animDone)
        {
            GRAHAM_FLAGS(owner) |= 4;
            entity->subState = 4;
            entity->stage = 0;
            Sprite_Free(entity, 1);
        }
        return;
    case 4:
        if (!part->updateFunc)
            entity->deleteRequested = 1;
        return;
    }
    if (!entity->hidden)
        Anim_Update(entity);
}


