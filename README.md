# ETF 2.0

[![build](https://github.com/etfdevs/ETF/actions/workflows/build.yml/badge.svg)](https://github.com/etfdevs/ETF/actions/workflows/build.yml) * <a href="https://discord.com/channels/253600486219972608/401475882897899523"><img src="https://img.shields.io/discord/253600486219972608?color=7289da&logo=discord&logoColor=white" alt="Discord server" /></a>

This is an early upload of the ETF mod post 1.6 release with some experimental code added that was not part of any original release but was my testing purposes.

Notes:

* Requires additional assets from [etf-pak6](https://github.com/etfdevs/etf-pak6/releases/tag/latest) as well as full 1.6 release
* 64-bit is now supported (may not have exactly same results)
* macOS (64-bits) is now supported in compilation but nobody has tested it
* Some code is used from other trees like OpenJK q_math and such for convenience this may be replaced or reverted at some point.
* Compilation uses CMAKE, use mingw on windows to build for real. MSVC will probably produce incorrect results with movement.
* Some features I added are WIP and need removing
* Code is released under GPLv3 terms same as original ET per Splash Damage's request.


## Changes over 1.6:
* Fixed weapon description menu crash due to utf8 files used in text description files
* Upgrade, Repair, Refill floating icons over engineer buildings
* Supply stations now have upgrades to level 2 and 3
    * Level 2 - More health and refills faster
    * Level 3 - Same health as level 3 sentry, refills faster, and fills 1 grenade (maximum storage 2) every minute. Counter starts at 0 when first at level 3
* Supply stations now show crosshair stats and level when pointed at by friendly engineers similar to autosentries (with color)
* Fixed supply station ammo overflow if player had used /give cheats to get 999 ammo and then attempted a supply ammo
* Fixed rare case where recons could attempt to disarm a charge while it was still being planted
* Fixed ally hitsound playing when detonating pipes on a HE charge
* Autosentry shell and rocket ammo on crosshair now shows orange and red color respectively
* Sniper rifle leg shot affects movement speed again. Obtaining health fixes them or being healed by a paramedic.
* Melee weapon sounds play correctly for all clients not just client 0
* Quad colors are matched based on the client owner of missile and client shader now, not just client 0
* Bugfixes for certain map entities and ceasefire toggle (such as doors)
* Bugfixes for certain map entities with wait times not handled properly (such as shootable buttons)
* Forcefields properly allow civilians through and prediction is fixed for civilians too
* Napalm grenades affect func_damage now
* Maps can now give or remove disease effect such as curing all effects on capture
* New map entities: func_noannoyances, func_visibility + improved target_blackhole
* HE Charge timer on HUD support
* Flags should no longer stick in certain facing walls
* Tons of other small misc. bugfixes across the codebase, still compiling list from over the years
* Ladder speeds now same as normal run speed
* Cured messages no longer show up on capture if you didn't have that ailment active
* Servers can now set g_spawnFullStats to spawn with full stats except for gren2s
* Paramedic syringe stab kills now register with their own means of death separate from disease and failed operation
* Alternative obits improved (could use more with icons in future)
* Re-enabled flares and sunflares from Q3F2 in some maps, including support for adding them with a .flr file
* Fixed agent disguise as civilian showing the backpack
* Improve precaching for all class assets and sounds so there is fewer hitches during gameplay when new assets are used
* Voice comms are precached for less disk hits/hitching during gameplay
* Fixed team allies not showing classes on scoreboard for the other teams which are also your allies
* Fixed forcefield spirit effects rendering black color on invalid forcefield surface entities
* Fixed nailbomb explosion rendering twice and in wrong position
* Fixed nailbomb nails quad state rendering
* Fixed flamethrower and minigun playing fire windup effects when disguised and attacking to remove disguise
* Fixed callvote exploits common to Q3 engine in game code
* Added ignore commands from ETmain
* Added mute commands for admin and rcon
* Fixed some commands showing up during intermission in chat box or when using menus after chat text
* Add worldspawn/mapinfo overrides for "nofallingdmg" and "noselfdmg". Useful for fun/trick/jump maps
* Flags now remember their original angles when returning to base from drop or capture
* New chat tokens: $1, $2 - shows string of grenade type in gren1 and gren2 slots respectively for your current class
* New chat tokens: $z, $x, $v - shows location of sentry, supplystation and HE charge respectively
* Callvote timelimit with + or - adds or subtracts minutes from current server timelimit
* Callvote capturelimit with + or - adds or subtracts captures from current server capturelimit
* Fixed weapon icons in weapon select box showing smaller size when out of ammo
* Godmode cheat is retained across target_respawn respawns