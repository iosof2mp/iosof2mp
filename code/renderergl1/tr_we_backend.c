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
-----------
Snow system
-----------
=============================================
*/

/*
==================
RB_SnowSystemUpdate

Checks player surroundings and updates
particles accordingly.

This can result in new particles being
rendered, particles already rendering
being updated to new positions or even
particles not being rendered anymore
(e.g. out of area or particles no
longer outside).
==================
*/

void RB_SnowSystemUpdate(worldEffectSystem_t *weSystem, float elapsedTime)
{
    snowSystem_t            *snowSystem;
    worldEffectParticle_t   *item;
    vec3_t                  origin, difference;
    vec3_t                  newMins, newMaxs;
    vec3_t                  start;
    qboolean                changePos;
    int                     i;
    int                     x, y, z;

    snowSystem  = (snowSystem_t *)weSystem;

    snowSystem->windChange--;
    if(snowSystem->windChange < 0){
        snowSystem->windDirection[0] = flrand(-1.0f, 1.0f);
        snowSystem->windDirection[1] = flrand(-1.0f, 1.0f);
        snowSystem->windDirection[2] = 0.0f;

        VectorNormalize(snowSystem->windDirection);
        VectorScale(snowSystem->windDirection, 0.025f, snowSystem->windSpeed);

        snowSystem->windChange = irand(200, 450);

        // TODO: Update wind direction for all misty fog effect images.
    }

    // Update all effects if we are rendering.
    if(snowSystem->isRendering){
        R_UpdateWorldEffects(weSystem, elapsedTime);
    }

    VectorCopy(backEnd.viewParms.or.origin, origin);

    // Check when the next wind gust should occur.
    snowSystem->nextWindGust -= elapsedTime;
    if(snowSystem->nextWindGust < 0){
        // TODO: There is currently no wind, disable the effect temporarily.
    }

    if(snowSystem->nextWindGust < snowSystem->windLowSize){
        vec3_t  windDirection;
        vec3_t  windPos;

        windDirection[0] = flrand(-1.0f, 1.0f);
        windDirection[1] = flrand(-1.0f, 1.0f);
        windDirection[2] = 0.0f;

        VectorNormalize(windDirection);
        VectorScale(windDirection, flrand(snowSystem->windMin, snowSystem->windMax), windDirection);

        VectorCopy(origin, windPos);

        // TODO: Update wind effect.
    }

    VectorAdd(snowSystem->minSpread, origin, newMins);
    VectorAdd(snowSystem->maxSpread, origin, newMaxs);

    changePos = qfalse;
    for(i = 0; i < 3; i++){
        difference[i] = newMaxs[i] - snowSystem->maxs[i];

        if(difference[i] >= 0.0f){
            if(difference[i] > newMaxs[i] - newMins[i]){
                difference[i] = newMaxs[i] - newMins[i];
            }

            start[i] = newMaxs[i] - difference[i];
        }else{
            if(difference[i] < newMins[i] - newMaxs[i]){
                difference[i] = newMins[i] - newMaxs[i];
            }

            start[i] = newMins[i] - difference[i];
        }

        if(!changePos && fabs(difference[i]) > 25.0f){
            changePos = qtrue;
        }
    }

    //
    // Re-determine particile positions.
    //
    if(changePos){
        int         *store;
        int         contentsPos, contents;
        vec3_t      pos;
        qboolean    resetFlake;

        for(i = 0; i < 3; i++){
            contentsPos = (origin[i] + snowSystem->minSpread[i]) / snowSystem->contentsSize[i];
            snowSystem->contentsStart[i] = contentsPos * snowSystem->contentsSize[i];
        }

        snowSystem->overallContents = 0;
        store = (int *)snowSystem->contents;

        for(z = 0, pos[2] = snowSystem->contentsStart[2]; z < SNOWCONTENTS_Z_SIZE; z++, pos[2] += snowSystem->contentsSize[2]){
            for(y = 0, pos[1] = snowSystem->contentsStart[1]; y < SNOWCONTENTS_Y_SIZE; y++, pos[1] += snowSystem->contentsSize[1]){
                for(x = 0, pos[0] = snowSystem->contentsStart[0]; x < SNOWCONTENTS_X_SIZE; x++, pos[0] += snowSystem->contentsSize[0]){
                    contents = ri.CM_PointContents(pos, 0);
                    snowSystem->overallContents |= contents;
                    *store++ = contents;
                }
            }
        }

        item = weSystem->particleList;
        for(i = 0; i < weSystem->numParticles; i++){
            resetFlake = qfalse;

            for(x = 0; x < 3; x++){
                if(item->pos[x] < newMins[x] || item->pos[x] > newMaxs[x]){
                    item->pos[x] = flrand(0.0f, difference[x]) + start[x];
                    resetFlake = qtrue;
                }
            }

            if(resetFlake){
                item->velocity[0] = 0.0f;
                item->velocity[1] = 0.0f;
                item->velocity[2] = flrand(snowSystem->maxVelocity[2], snowSystem->minVelocity[2]);
            }

            item++;
        }

        VectorCopy(newMins, snowSystem->mins);
        VectorCopy(newMaxs, snowSystem->maxs);
    }

    // No need to render or update any particle as none of
    // the particles are outside.
    if(!(snowSystem->overallContents & CONTENTS_OUTSIDE)){
        snowSystem->isRendering = qfalse;
        return;
    }

    snowSystem->isRendering     = qtrue;
    item                        = weSystem->particleList;
    for(i = 0; i < weSystem->numParticles; i++){
        for(x = 0; x < 2; x++){
            if(item->velocity[x] < snowSystem->minVelocity[x]){
                item->velocity[x] += snowSystem->velocityStabilize * elapsedTime;
            }else if(item->velocity[x] > snowSystem->maxVelocity[x]){
                item->velocity[x] -= snowSystem->velocityStabilize * elapsedTime;
            }else{
                item->velocity[x] += flrand(-1.4f, 1.4f);
            }
        }

        if(item->velocity[2] > snowSystem->minVelocity[2]){
            item->velocity[2] -= snowSystem->velocityStabilize * 2.0f;
        }

        VectorMA(item->pos, elapsedTime, item->velocity, item->pos);

        // Check if we should render this snowflake.
        x = (item->pos[0] - snowSystem->contentsStart[0]) / snowSystem->contentsSize[0];
        y = (item->pos[1] - snowSystem->contentsStart[1]) / snowSystem->contentsSize[1];
        z = (item->pos[2] - snowSystem->contentsStart[2]) / snowSystem->contentsSize[2];

        if(x < 0 || x >= SNOWCONTENTS_X_SIZE ||
           y < 0 || y >= SNOWCONTENTS_Y_SIZE ||
           z < 0 || z >= SNOWCONTENTS_Z_SIZE
        ){
            item->pos[0] = flrand(newMins[0], newMaxs[0]);
            item->pos[1] = flrand(newMins[1], newMaxs[1]);
            item->pos[2] = newMaxs[2] - (newMins[2] - item->pos[2]);
            if(item->pos[2] < newMins[2] || item->pos[2] > newMaxs[2]){
                // Way out of range.
                item->pos[2] = flrand(newMins[2], newMaxs[2]);
            }

            item->velocity[0] = 0.0f;
            item->velocity[1] = 0.0f;
            item->velocity[2] = flrand(snowSystem->maxVelocity[2], snowSystem->minVelocity[2]);
            item->flags &= ~PARTICLE_FLAG_RENDER;
        }else if(snowSystem->contents[z][y][x] & CONTENTS_OUTSIDE){
            item->flags |= PARTICLE_FLAG_RENDER;
        }else{
            item->flags &= ~PARTICLE_FLAG_RENDER;
        }

        item++;
    }
}

/*
==================
RB_SnowSystemRender

Renders all snow particles
currently marked to be
rendered.
==================
*/

void RB_SnowSystemRender(worldEffectSystem_t *weSystem)
{
    snowSystem_t            *snowSystem;
    worldEffectParticle_t   *item;
    int                     i;
    static float            snowAttenuation[3] = {
        1.0f, 0.0f, 0.0004f
    };

    snowSystem = (snowSystem_t *)weSystem;

    // Only render if any particle is currently outside.
    if(!snowSystem->isRendering){
        return;
    }

    R_RenderWorldEffects(weSystem);

    qglColor4f(0.8f, 0.8f, 0.8f, snowSystem->alpha);

    GL_State(GLS_ALPHA);
    qglDisable(GL_TEXTURE_2D);

    if(qglPointParameterfEXT){
        qglPointSize(10.0f);
        qglPointParameterfEXT(GL_POINT_SIZE_MIN_EXT, 1.0f);
        qglPointParameterfEXT(GL_POINT_SIZE_MAX_EXT, 4.0f);
        qglPointParameterfvEXT(GL_DISTANCE_ATTENUATION_EXT, snowAttenuation);
    }else{
        qglPointSize(2.0f);
    }

    item = weSystem->particleList;
    qglBegin(GL_POINTS);

    for(i = 0; i < weSystem->numParticles; i++){
        if(item->flags & PARTICLE_FLAG_RENDER){
            qglVertex3fv(item->pos);
        }

        item++;
    }

    qglEnd();
    qglEnable(GL_TEXTURE_2D);
}
