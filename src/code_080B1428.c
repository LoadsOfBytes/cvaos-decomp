/* Disc Armor disc projectile (uses the Disc Armor graphics of EnemyDiscArmorCreate). */
#include "code_08009A0.h"
#include "disc_armor_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *, s32);
extern u8 sDiscArmorGfx[], sDiscArmorPalette[], sDiscArmorSpriteSheet[];

/* State 0 loads graphics, starts the animation selected by unk_4F8 and sets
 * spriteLayer = 0x12 and scrollWithBg.  Every frame (including the first) it then
 * accelerates with 0x80000 and sets deleteRequested when the animation returns 3. */
void DiscArmorSpark_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sDiscArmorGfx, sDiscArmorPalette, 0, sDiscArmorSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        Anim_StartIndexed(entity, (u8)entity->ext.discArmorSpark.anim, 0, 0);
        entity->spriteLayer = 0x12;
        entity->scrollWithBg = 1;
        entity->state++;
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
    }
    if (Anim_Update(entity) == 3)
        entity->deleteRequested = 1;
}
