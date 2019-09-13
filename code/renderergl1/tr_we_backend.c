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

Loads the specified misty fog image as
data image file.
==================
*/

qboolean RB_LoadMistyFogImage(mistyFogImage_t *fogImage, char *fileName)
{
    R_LoadDataImage(fileName, &fogImage->data, &fogImage->width, &fogImage->height);

    return fogImage->data != NULL;
}

/*
==================
RB_CreateMistyFogTextureCoords

Sets up the texture coordinates on the
misty fog image based on the wind.
==================
*/

void RB_CreateMistyFogTextureCoords(mistyFogImage_t *fogImage)
{
    int     xStart, yStart;
    float   forwardWind;

    fogImage->speed = flrand(200.0f, 700.0f);

    forwardWind = DotProduct(fogImage->windTransform, backEnd.viewParms.or.axis[0]);

    if(forwardWind > 0.5f){
        // Moving away, so make the size smaller.
        fogImage->currentSize = flrand(fogImage->minSize, fogImage->minSize * 1.01f);
    }else if(forwardWind < -0.5f){
        // Moving towards, so make the size bigger.
        fogImage->currentSize = flrand(fogImage->maxSize - fogImage->minSize, fogImage->maxSize);
    }else{
        // Normal range.
        fogImage->currentSize = flrand(fogImage->minSize * 1.5f, fogImage->minSize * 1.5f + fogImage->size);
    }

    fogImage->currentSize /= 2.0f;

    xStart = (1.0f - fogImage->currentSize - 0.40f) * flrand(0.0f, 1.0f) + 0.20f;
    yStart = (1.0f - fogImage->currentSize - 0.40f) * flrand(0.0f, 1.0f) + 0.20f;

    fogImage->textureCoords[0][0] = xStart - fogImage->currentSize;
    fogImage->textureCoords[0][1] = yStart - fogImage->currentSize;
    fogImage->textureCoords[1][0] = xStart + fogImage->currentSize;
    fogImage->textureCoords[1][1] = yStart + fogImage->currentSize;
}

/*
==================
RB_MistyFogEffectUpdate

Updates the misty fog effect images
by taking the wind speed and direction
into account, given we are allowed to
render in the current state
(e.g. outside).
==================
*/

void RB_MistyFogEffectUpdate(worldEffectSystem_t *weSystem, worldEffect_t *effect, float elapsedTime)
{
    mistyFogEffect_t    *mistyFogEffect;
    int                 originContents;
    int                 i, j;
    int                 x, y;

    mistyFogEffect = (mistyFogEffect_t *)effect;
    originContents = ri.CM_PointContents(backEnd.viewParms.or.origin, 0);

    if(originContents & CONTENTS_OUTSIDE && !(originContents & CONTENTS_WATER)){
        if(mistyFogEffect->fadeAlpha < 1.0f){
            // Player just went outside, fade in fog.
            mistyFogEffect->fadeAlpha += elapsedTime / 2.0f;
        }

        if(mistyFogEffect->fadeAlpha > 1.0f){
            mistyFogEffect->fadeAlpha = 1.0f;
        }
    }else{
        if(mistyFogEffect->fadeAlpha > 0.0f){
            // Player just went inside, fade out fog.
            mistyFogEffect->fadeAlpha -= elapsedTime / 2.0f;
        }

        if(mistyFogEffect->fadeAlpha <= 0.0f){
            // No need to continue here as the player is (still) inside
            // and the fog is already fully faded out.
            return;
        }
    }

    for(y = 0; y < MISTYFOG_HEIGHT; y++){
        for(x = 0; x < MISTYFOG_WIDTH; x++){
            mistyFogEffect->colors[y][x][3] = 0.0f;
        }
    }

    // Update all associated images.
    for(i = 0; i < MISTYFOG_NUM_IMAGES; i++){
        for(j = 0; j < MISTYFOG_NUM_PAIRED_IMAGES; j++){
            mistyFogImage_t     *mistyFogImage;
            mistyFogImage_t     *pairedImage;
            qboolean            removeImage;
            float               windSpeed;
            float               forwardWind, rightWind;

            mistyFogImage       = &mistyFogEffect->images[i][j];
            removeImage         = qfalse;

            // Translate.
            windSpeed           = 1.0f / mistyFogImage->speed;
            forwardWind         = DotProduct(mistyFogImage->windTransform, backEnd.viewParms.or.axis[0]);
            rightWind           = DotProduct(mistyFogImage->windTransform, backEnd.viewParms.or.axis[1]);

            mistyFogImage->textureCoords[0][0] += rightWind * windSpeed;
            mistyFogImage->textureCoords[1][0] += rightWind * windSpeed;

            mistyFogImage->textureCoords[0][0] -= forwardWind * windSpeed * 0.25f;
            mistyFogImage->textureCoords[0][1] -= forwardWind * windSpeed * 0.25f;
            mistyFogImage->textureCoords[1][0] -= forwardWind * windSpeed * 0.25f;
            mistyFogImage->textureCoords[1][1] -= forwardWind * windSpeed * 0.25f;

            if((fabs(mistyFogImage->textureCoords[0][0] - mistyFogImage->textureCoords[1][0]) < mistyFogImage->minSize) ||
               (fabs(mistyFogImage->textureCoords[0][1] - mistyFogImage->textureCoords[1][1]) < mistyFogImage->minSize)
            ){
                removeImage = qtrue;
            }else if((fabs(mistyFogImage->textureCoords[0][0] - mistyFogImage->textureCoords[1][0]) > mistyFogImage->maxSize) ||
                     (fabs(mistyFogImage->textureCoords[0][1] - mistyFogImage->textureCoords[1][1]) > mistyFogImage->maxSize)
            ){
                removeImage = qtrue;
            }

            if(removeImage && !mistyFogImage->alphaFade){
                mistyFogImage->alphaFade = qtrue;
                mistyFogImage->alphaDirection = -0.025f;

                // Update the image bound to the current one.
                pairedImage = &mistyFogEffect->images[i][j == 0];

                pairedImage->alpha = 0.0f;
                pairedImage->alphaDirection = 0.025f;
                pairedImage->alphaFade = qtrue;
                RB_CreateMistyFogTextureCoords(pairedImage);
                pairedImage->isRendering = qtrue;
            }else if(mistyFogImage->alphaFade){
                mistyFogImage->alpha += mistyFogImage->alphaDirection * 0.4f;

                if(mistyFogImage->alpha < 0.0f){
                    mistyFogImage->alpha = 0.0f;
                    mistyFogImage->isRendering = qfalse;
                }else if(mistyFogImage->alpha >= 1.0f){
                    mistyFogImage->alpha = 1.0f;
                    mistyFogImage->alphaFade = qfalse;
                }
            }
        }
    }

    // Determine and update texture colors for the current state.
    R_UpdateMistyFogTextures(mistyFogEffect);
}

/*
==================
RB_MistyFogEffectRender

Render all misty fog effect images
to the screen.
==================
*/

void RB_MistyFogEffectRender(worldEffectSystem_t *weSystem, worldEffect_t *effect)
{
    mistyFogEffect_t    *mistyFogEffect;
    double              gluPersMax;

    mistyFogEffect = (mistyFogEffect_t *)effect;

    // Only update if this effect is currently enabled,
    // i.e. it is not fully faded out.
    if(mistyFogEffect->fadeAlpha <= 0.0f){
        return;
    }

    qglMatrixMode(GL_PROJECTION);
    qglPushMatrix();
    qglLoadIdentity();

    // Define the view frustum.
    gluPersMax = 4.0 * tan(80.0 * M_PI / 360.0);
    qglFrustum(-gluPersMax, gluPersMax, -gluPersMax, gluPersMax, 4.0f, 2048.0f);

    qglMatrixMode(GL_MODELVIEW);
    qglPushMatrix();
    qglLoadIdentity();

    // Put Z going up.
    qglRotatef(-90, 1, 0, 0);
    qglRotatef(90, 0, 0, 1);

    qglRotatef(0, 1, 0, 0);

    qglRotatef(-90, 0, 1, 0);
    qglRotatef(-90, 0, 0, 1);

    qglDisable(GL_TEXTURE_2D);
    GL_State(GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE);
    qglShadeModel(GL_SMOOTH);

    qglEnableClientState(GL_COLOR_ARRAY);

    qglColorPointer(4, GL_FLOAT, 0, mistyFogEffect->colors);
    qglVertexPointer(3, GL_FLOAT, 0, mistyFogEffect->verts);

    if(qglLockArraysEXT){
        qglLockArraysEXT(0, MISTYFOG_HEIGHT * MISTYFOG_WIDTH);
    }

    qglDrawElements(GL_QUADS, (MISTYFOG_HEIGHT - 1) * (MISTYFOG_WIDTH - 1) * 4, GL_UNSIGNED_INT, mistyFogEffect->indexes);

    if(qglUnlockArraysEXT){
        qglUnlockArraysEXT();
    }

    qglDisableClientState(GL_COLOR_ARRAY);

    qglPopMatrix();
    qglMatrixMode(GL_PROJECTION);
    qglPopMatrix();
    qglMatrixMode(GL_MODELVIEW);

    qglEnable(GL_TEXTURE_2D);
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

Updates any particles of the parent
world effect system that are affected
by this wind effect.
==================
*/

void RB_WindEffectUpdate(worldEffectSystem_t *weSystem, worldEffect_t *effect, float elapsedTime)
{
    windEffect_t            *windEffect;
    worldEffectParticle_t   *item;
    vec3_t                  difference;
    float                   dist, calcDist;
    float                   scaleLength;
    int                     i, j;

    windEffect = (windEffect_t *)effect;

    // Only update if this effect is currently enabled in the parent world effect system.
    if(!windEffect->isEnabled){
        return;
    }

    // Calculate total distance between the wind effect origin
    // and the current player position.
    VectorSubtract(backEnd.viewParms.or.origin, windEffect->point, difference);
    if(VectorLength(difference) > 300.0f){
        // This effect instance is too far away to impact any
        // of the particles that are currently rendering.
        return;
    }

    calcDist = 0.0f;
    for(i = 0; i < weSystem->numParticles; i++){
        item = &weSystem->particleList[i];

        if(windEffect->affectedParticles[i]){
            windEffect->affectedParticles[i]--;
        }else{
            if(!windEffect->isGlobal){
                for(j = 0; j < windEffect->numPlanes; j++){
                    dist = DotProduct(item->pos, windEffect->planes[j]) - windEffect->planes[j][3];

                    if(dist < 0.01f || dist > windEffect->maxDistance[j]){
                        break;
                    }else if(j == 0){
                        calcDist = dist;
                    }
                }

                if(j != windEffect->numPlanes){
                    continue;
                }
            }

            scaleLength = 1.0f - (calcDist / windEffect->maxDistance[0]);

            windEffect->affectedParticles[i] = windEffect->affectedDuration * scaleLength;
            VectorMA(item->velocity, elapsedTime, windEffect->velocity, item->velocity);
        }
    }
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
    windEffect_t            *windGust;
    worldEffectParticle_t   *item;
    vec3_t                  origin, difference;
    vec3_t                  newMins, newMaxs;
    vec3_t                  start;
    qboolean                changePos;
    int                     i;
    int                     x, y, z;

    snowSystem  = (snowSystem_t *)weSystem;
    windGust    = (windEffect_t *)R_GetWorldEffect(weSystem, "wind");

    snowSystem->windChange--;
    if(snowSystem->windChange < 0){
        snowSystem->windDirection[0] = flrand(-1.0f, 1.0f);
        snowSystem->windDirection[1] = flrand(-1.0f, 1.0f);
        snowSystem->windDirection[2] = 0.0f;

        VectorNormalize(snowSystem->windDirection);
        VectorScale(snowSystem->windDirection, 0.025f, snowSystem->windSpeed);

        snowSystem->windChange = irand(200, 450);

        // Update wind direction for all misty fog effect images.
        R_UpdateMistyFogWindDirection(weSystem, snowSystem->windDirection);
    }

    // Update all effects if we are rendering.
    if(weSystem->isRendering){
        R_UpdateWorldEffects(weSystem, elapsedTime);
    }

    VectorCopy(backEnd.viewParms.or.origin, origin);

    // Check when the next wind gust should occur.
    snowSystem->nextWindGust -= elapsedTime;
    if(snowSystem->nextWindGust < 0){
        // There is currently no wind, disable the effect temporarily.
        windGust->isEnabled = qfalse;
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

        // Update the global wind effect with the new parameters.
        windGust->isEnabled = qtrue;
        R_UpdateWindParams(windGust, windPos, windDirection, snowSystem->windSize);

        snowSystem->nextWindGust = flrand(snowSystem->windDuration, snowSystem->windDuration * 2.0f);
        snowSystem->windLowSize = -flrand(snowSystem->windLow, snowSystem->windLow * 3.0f);
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
        weSystem->isRendering = qfalse;
        return;
    }

    weSystem->isRendering       = qtrue;
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
    if(!weSystem->isRendering){
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

    qglBegin(GL_POINTS);

    for(i = 0; i < weSystem->numParticles; i++){
        item = &weSystem->particleList[i];

        if(item->flags & PARTICLE_FLAG_RENDER){
            qglVertex3fv(item->pos);
        }
    }

    qglEnd();
    qglEnable(GL_TEXTURE_2D);
}

//==============================================

/*
==================
RB_RenderWorldEffectSystems

Renders all active world effect systems
along with their associated world
effects.
==================
*/

void RB_RenderWorldEffectSystems(void)
{
    // Only render world effect systems if there is a world
    // and it is being rendered.
    if(tr.refdef.rdflags & RDF_NOWORLDMODEL || !tr.world){
        return;
    }

    // Set model view matrix for the viewer.
    SetViewportAndScissor();
    qglMatrixMode(GL_MODELVIEW);
    qglLoadMatrixf(backEnd.viewParms.world.modelMatrix);

    // Let the world effect system handler update
    // and render all systems initialized.
    R_RenderWorldEffectSystems(tr.refdef.frameTime * 0.001);
}
