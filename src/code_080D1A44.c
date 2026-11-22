/* Belmont hit response, defeat notification and repeated-hit stun. */
#include "code_08009A0.h"
#include "belmont_entity.h"
#include "code_080D73B8.h"
#include "code_08013960.h"
#include "structs/ewram.h"
extern s32 Enemy_TakeDamage(struct EwramData_EntityData *,s32,u16 *);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
/**
 * @brief Apply enemy damage, trigger the defeat reaction and reward drop
 * when reported, or count repeated hits toward a flashing stun reaction
 * while the powered flag is clear.
 */
void Belmont_OnHit(struct EwramData_EntityData *entity,s32 kind,u16 *hit)
{
    if(Enemy_TakeDamage(entity,kind,hit))
    {
        PlaySong(0x1E1);
        Music_SetCurrent(0x25);
        entity->state=11;
        entity->stage=0;
        entity->subState=0;
        if((s16)entity->unk_518.part16.hp<=0)
            Enemy_DropRewards(entity);
    }
    else
    {
        u8 mask=BELMONT_FLAGS(entity) & 8;
        if(mask==0 && hit[0]==2)
        {
            if(BELMONT_HITS(entity)>2)
            {
                BELMONT_FLASH(entity)=32;
                BELMONT_HITS(entity)=mask;
                entity->state=11;
                entity->stage=mask;
                entity->subState=mask;
                entity->attack=BELMONT_ATTACK_OFF(entity);
                if(RandomNumberGenerator()%2==0)
                    PlaySong(0x1E3);
                else
                    PlaySong(0x1E2);
            }
            else
                BELMONT_HITS(entity)++;
        }
    }
}
