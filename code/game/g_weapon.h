#ifndef G_WEAPON_H
#define G_WEAPON_H

// JT - Prototypes from g_weapon.c

void weapon_supershotgun_fire (struct gentity_s *ent);
void weapon_shotgun_fire (struct gentity_s *ent);
void weapon_rocketlauncher_fire (struct gentity_s *ent);
void weapon_grenadelauncher_fire (struct gentity_s *ent);
void weapon_nailgun_fire(struct gentity_s *ent);
void weapon_supernailgun_fire (struct gentity_s *ent);
void weapon_railgun_fire(struct gentity_s *ent);
void Weapon_Flamethrower_Fire(struct gentity_s *ent);

// JT - Prototypes from g_missile.c

void G_ExplodeMissile( gentity_t *ent);

#endif
