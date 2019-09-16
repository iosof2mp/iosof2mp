/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// tr_we_backend.c - Implementation of the world effect system backend.

#include "tr_local.h"

/*
=============================================
----------------
Misty fog effect
----------------
=============================================
*/

/*
==================
RB_LoadMistyFogImage

TODO: Implement.
==================
*/

qboolean RB_LoadMistyFogImage(mistyFogImage_t *fogImage, char *fileName)
{
    return qfalse;
}

/*
==================
RB_CreateMistyFogTextureCoords

TODO: Implement.
==================
*/

void RB_CreateMistyFogTextureCoords(mistyFogImage_t *fogImage)
{

}

/*
==================
RB_MistyFogEffectUpdate

TODO: Implement.
==================
*/

void RB_MistyFogEffectUpdate(worldEffectSystem_t *weSystem, worldEffect_t *effect, float elapsedTime)
{

}

/*
==================
RB_MistyFogEffectRender

TODO: Implement.
==================
*/

void RB_MistyFogEffectRender(worldEffectSystem_t *weSystem, worldEffect_t *effect)
{

}

/*
=============================================
-----------
Wind effect
-----------
=============================================
*/

/*
==================
RB_WindEffectUpdate

TODO: Implement.
==================
*/

void RB_WindEffectUpdate(worldEffectSystem_t *weSystem, worldEffect_t *effect, float elapsedTime)
{

}

/*
=============================================
-----------
Rain system
-----------
=============================================
*/

/*
==================
RB_LoadRainImage

TODO: Implement.
==================
*/

void RB_LoadRainImage(rainSystem_t *rainSystem, const char *fileName)
{

}
/*
==================
RB_RainSystemUpdate

TODO: Implement.
==================
*/

void RB_RainSystemUpdate(worldEffectSystem_t *weSystem, float elapsedTime)
{

}

/*
==================
RB_RainSystemRender

TODO: Implement.
==================
*/

void RB_RainSystemRender(worldEffectSystem_t *weSystem)
{

}

/*
=============================================
-----------
Snow system
-----------
=============================================
*/

/*
==================
RB_SnowSystemUpdate

TODO: Implement.
==================
*/

void RB_SnowSystemUpdate(worldEffectSystem_t *weSystem, float elapsedTime)
{

}

/*
==================
RB_SnowSystemRender

TODO: Implement.
==================
*/

void RB_SnowSystemRender(worldEffectSystem_t *weSystem)
{

}
