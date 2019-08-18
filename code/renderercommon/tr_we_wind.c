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
// tr_we_wind.c - Implementation of the wind world effect.

#include "tr_common.h"
#include "tr_we.h"

/*
==================
R_UpdateWindParams

Updates planes in this wind effect
instance based on the parameters
passed.
==================
*/

void R_UpdateWindParams(windEffect_t *windEffect, vec3_t point, vec3_t velocity, vec3_t size)
{
    vec3_t  normalDistance;
    int     numPlanes;

    numPlanes = 0;

    // Store the new wind parameters in this effect instance.
    VectorCopy(point, windEffect->point);
    VectorCopy(velocity, windEffect->velocity);
    VectorCopy(size, windEffect->size);

    // Update planes based on the new wind parameters.
    windEffect->size[0] /= 2.0f;
    VectorScale(windEffect->size, 2, windEffect->size);

    VectorCopy(velocity, windEffect->planes[numPlanes]);
    VectorNormalize(windEffect->planes[numPlanes]);
    windEffect->planes[numPlanes][3] = DotProduct(windEffect->point, windEffect->planes[numPlanes]);
    windEffect->maxDistance[numPlanes] = windEffect->size[0];
    numPlanes++;

    VectorScale(windEffect->planes[0], windEffect->planes[0][3], normalDistance);
    VectorSubtract(windEffect->point, normalDistance, windEffect->planes[numPlanes]);
    VectorNormalize(windEffect->planes[numPlanes]);
    windEffect->planes[numPlanes][3] = DotProduct(windEffect->point, windEffect->planes[numPlanes]);
    windEffect->maxDistance[numPlanes] = windEffect->size[1];
    numPlanes++;

    CrossProduct(windEffect->planes[0], windEffect->planes[1], windEffect->planes[numPlanes]);
    VectorNormalize(windEffect->planes[numPlanes]);
    windEffect->planes[numPlanes][3] = DotProduct(windEffect->point, windEffect->planes[numPlanes]);
    windEffect->maxDistance[numPlanes] = windEffect->size[2];
    numPlanes++;

    windEffect->planes[0][3] -= (windEffect->size[0] / 2.0f);
    windEffect->planes[1][3] -= (windEffect->size[1] / 2.0f);
    windEffect->planes[2][3] -= (windEffect->size[2] / 2.0f);

    windEffect->numPlanes = numPlanes;
}

/*
==================
R_AddWindEffect

Adds a new wind effect instance
to the specified world effect
system.
==================
*/

windEffect_t *R_AddWindEffect(worldEffectSystem_t *weSystem, qboolean isGlobalEffect)
{
    windEffect_t    *windEffect;

    //
    // Allocate memory for the new wind effect instance.
    //
    windEffect = ri.Malloc(sizeof(windEffect_t));
    Com_Memset(windEffect, 0, sizeof(windEffect_t));

    //
    // Set base world effect information.
    //
    windEffect->base.name = "wind";

    // TODO: Set proper function.
    windEffect->base.Update = NULL;

    //
    // Set wind effect information.
    //
    windEffect->isGlobal = isGlobalEffect;

    windEffect->affectedParticles = ri.Malloc(weSystem->numParticles * sizeof(int));
    Com_Memset(windEffect->affectedParticles, 0, weSystem->numParticles * sizeof(int));

    //
    // Add the new instance to the world effect list
    // of the parent world effect system.
    //
    R_AddWorldEffect(weSystem, (worldEffect_t *)windEffect);

    return windEffect;
}
