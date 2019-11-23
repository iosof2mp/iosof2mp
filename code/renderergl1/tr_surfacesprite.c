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
// tr_we_surfacesprite.c - Implementation of the surface sprite system.

#include "tr_local.h"

#define WIND_DAMP_INTERVAL  50.0f
#define WIND_GUST_TIME      2500.0f
#define WIND_GUST_DECAY     (1.0f / WIND_GUST_TIME)

static int              lastUpdateTime;
static trRefEntity_t    *lastEntityDrawn;

static float            curWindSpeed;
static float            curWeatherAmount        = 1.0f;

static float            rangeScaleFactor        = 1.0f;

static vec3_t           curWindBlowVect, curWindGrassDir;

static qboolean         curWindPointActive;
static float            curWindPointForce;
static vec3_t           curWindPoint;

static vec3_t           rightVectors[4];
static vec3_t           fwdVector;

static vec3_t           viewOrigin, viewRight, viewUp, viewForward;
static vec3_t           up, down;
static vec3_t           *rightCoords, *upCoords, *forwardCoords;

static qboolean         additiveTransparency, usingFog;

static int              numSurfaceSprites, numSurfaces;

//==============================================

/*
==================
R_CreateRightVectors

Creates a set of four right vectors
so that surface sprites aren't
always facing the same way.
==================
*/

static void R_CreateRightVectors(qboolean drawingModel)
{
    static vec3_t   modelRight, modelUp, modelForward;

    if(drawingModel){
        VectorSet(modelRight,
            backEnd.or.modelMatrix[0],
            backEnd.or.modelMatrix[4],
            backEnd.or.modelMatrix[8]);

        VectorSet(modelUp,
            backEnd.or.modelMatrix[1],
            backEnd.or.modelMatrix[5],
            backEnd.or.modelMatrix[9]);

        VectorSet(modelForward,
            backEnd.or.modelMatrix[2],
            backEnd.or.modelMatrix[6],
            backEnd.or.modelMatrix[10]);

        VectorSet(up,
            backEnd.or.preTransEntMatrix[2],
            backEnd.or.preTransEntMatrix[6],
            backEnd.or.preTransEntMatrix[10]);

        VectorSet(down,
            -backEnd.or.preTransEntMatrix[2],
            -backEnd.or.preTransEntMatrix[6],
            -backEnd.or.preTransEntMatrix[10]);

        rightCoords     = &modelRight;
        upCoords        = &modelUp;
        forwardCoords   = &modelForward;
    }else{
        VectorSet(up, 0.0f, 0.0f, 1.0f);
        VectorSet(down, 0.0f, 0.0f, -1.0f);

        rightCoords     = &viewRight;
        upCoords        = &viewUp;
        forwardCoords   = &viewForward;
    }

    // First generate a horizontal forward vector.
    CrossProduct(*rightCoords, up, fwdVector);

    // Nudge forward.
    VectorScale(*rightCoords, 0.993f, rightVectors[0]);
    VectorMA(rightVectors[0], 0.115f, fwdVector, rightVectors[0]);

    // Big nudge back.
    VectorScale(*rightCoords, 0.94f, rightVectors[1]);
    VectorMA(rightVectors[1], -0.34f, fwdVector, rightVectors[1]);

    // Big nudge forward.
    VectorScale(*rightCoords, 0.94f, rightVectors[2]);
    VectorMA(rightVectors[2], 0.34f, fwdVector, rightVectors[2]);

    // Nudge back.
    VectorScale(*rightCoords, 0.993f, rightVectors[3]);
    VectorMA(rightVectors[3], -0.115f, fwdVector, rightVectors[3]);
}

/*
==================
R_SurfaceSpriteFrameUpdate

Updates surface sprite system internals
based on the current frame being
rendered.
==================
*/

static void R_SurfaceSpriteFrameUpdate(void)
{
    float           targetSpeed, dampFactor, dTime, ratio;
    vec3_t          targetWindGrassDir, targetWindBlowVect;
    vec3_t          retWindVec, ang;
    vec3_t          diff;

    static float    curWindGust             = 5.0f;
    static float    curWindGustLeft;
    static int      nextWindGustTime;

    static float    standardFovX            = 90.0f;
    static float    standardScaleX          = 1.0f;
    static qboolean standardFovInitialized;

    //
    // Check if there is anything to update.
    //
    if(backEnd.refdef.time == lastUpdateTime){
        return;
    }else if(backEnd.refdef.time < lastUpdateTime){
        // The current time is before the last update time,
        // reset everything.
        curWindGust         = 5.0f;
        curWindGustLeft     = 0.0f;
        curWindSpeed        = r_windSpeed->value;
        nextWindGustTime    = 0;

        VectorClear(curWindGrassDir);
    }

    // Reset the last entity drawn, since this is a new frame.
    lastEntityDrawn = NULL;

    //
    // Adjust scale using FOV.
    //
    if(!standardFovInitialized){
        // Not initialized yet.
        if(backEnd.refdef.fov_x > 50.0f && backEnd.refdef.fov_y < 135.0f){
            standardFovX = backEnd.refdef.fov_x;
            standardFovInitialized = qtrue;
        }else{
            standardFovX = 90.0f;
        }

        standardScaleX = tan(standardFovX * 0.5f * (M_PI / 180.0f));

        // Don't multiply the shader range by anything.
        rangeScaleFactor = 1.0f;
    }else if(standardFovX == backEnd.refdef.fov_x){
        // This is the standard FOV or higher,
        // don't multiply the shader range.
        rangeScaleFactor = 1.0f;
    }else{
        // Using a non-standard FOV. Multiply the range
        // of the shader by the scale factor.
        if(backEnd.refdef.fov_x > 135){
            rangeScaleFactor = standardScaleX / tan(135.0f * 0.5f * (M_PI / 180.0f));
        }else{
            rangeScaleFactor = standardScaleX / tan(backEnd.refdef.fov_x * 0.5f * (M_PI / 180.0f));
        }
    }

    //
    // Create the set of four right vectors.
    //
    R_CreateRightVectors(qfalse);

    //
    // If any world effect system is rendering, always
    // render surface weather.
    //
    if(R_IsAnyWorldEffectSystemRendering()){
        curWeatherAmount = 1.0f;
    }else{
        curWeatherAmount = r_surfaceWeather->value;
    }

    //
    // Update the wind.
    //

    // Try to get the wind speed from the rain system
    // rather than the CVAR.
    if(R_GetRainWindSpeed(&targetSpeed)){
        // Successfully got a speed from the rain system.
        targetSpeed *= 0.3f;

        if(targetSpeed >= 1.0f){
            curWindGust = 300.0f / targetSpeed;
        }else{
            curWindGust = 0.0f;
        }
    }else{
        // Use the CVAR value.

        // The minimum gust delay, in seconds.
        targetSpeed = r_windSpeed->value;
        curWindGust = r_windGust->value;
    }

    if(targetSpeed > 0.0f && curWindGust != 0.0f){
        if(curWindGustLeft > 0.0f){
            // Currently gusting.
            // Add an amount to the target wind speed.
            targetSpeed     *= 1.0f + curWindGustLeft;
            curWindGustLeft -= (float)(backEnd.refdef.time - lastUpdateTime) * WIND_GUST_DECAY;

            if(curWindGustLeft <= 0.0f){
                nextWindGustTime = backEnd.refdef.time + (curWindGust * 1000.0f) * flrand(1.0f, 4.0f);
            }
        }else if(backEnd.refdef.time >= nextWindGustTime){
            // See if there is another gust right now.
            curWindGustLeft = flrand(0.75f, 1.5f);
        }
    }

    // Check if there is a weather system available
    // for a current wind direction.
    if(R_GetWindDirection(retWindVec)){
        // Successfully got a wind direction.
        retWindVec[2] = 0.0f;
        VectorScale(retWindVec, -1.0f, retWindVec);
        vectoangles(retWindVec, ang);
    }else{
        // Calculate the target wind vector from the CVAR value.
        ang[YAW] = r_windAngle->value;
    }

    ang[PITCH] = -90.0f + targetSpeed;
    if(ang[PITCH] > -45.0f){
        ang[PITCH] = -45.0f;
    }
    ang[ROLL] = 0.0f;

    // Get the grass wind vector.
    AngleVectors(ang, targetWindGrassDir, NULL, NULL);
    targetWindGrassDir[2] -= 1.0f;

    // Get the general wind vector (no pitch).
    ang[PITCH] = 0.0f;
    AngleVectors(ang, targetWindBlowVect, NULL, NULL);

    // Start calculating a smoothing factor so wind doesn't
    // change abruptly between speeds.
    dampFactor = 1.0f - r_windDampFactor->value;

    // Figure out the time since the last update and set the ratio.
    dTime = (float)(backEnd.refdef.time - lastUpdateTime) * (1.0f / WIND_DAMP_INTERVAL);
    ratio = powf(dampFactor, dTime);

    // Apply this ratio to the windspeed.
    curWindSpeed = targetSpeed - (ratio * (targetSpeed - curWindSpeed));

    // Calculate the final target wind vector (with speed).
    VectorScale(targetWindBlowVect, curWindSpeed, targetWindBlowVect);
    VectorSubtract(targetWindBlowVect, curWindBlowVect, diff);
    VectorMA(targetWindBlowVect, -ratio, diff, curWindBlowVect);

    // Update the grass vector now.
    VectorSubtract(targetWindGrassDir, curWindGrassDir, diff);
    VectorMA(targetWindGrassDir, -ratio, diff, curWindGrassDir);

    curWindPointForce = r_windPointForce->value - (ratio * (r_windPointForce->value - curWindPointForce));
    if(curWindPointForce < 0.01f){
        curWindPointActive = qfalse;
    }else{
        VectorSet(curWindPoint,
            r_windPointX->value,
            r_windPointY->value,
            0);

        curWindPointActive = qtrue;
    }

    if(r_surfaceSprites->integer == 2){
        ri.Printf(PRINT_ALL, "Surfacesprites drawn: %d on %d surfaces.\n", numSurfaceSprites, numSurfaces);
    }

    lastUpdateTime      = backEnd.refdef.time;
    numSurfaceSprites   = 0;
    numSurfaces         = 0;
}

/*
==================
RB_DrawSurfaceSprites

Draw any surface sprites present
in the current shader stage.
==================
*/

void RB_DrawSurfaceSprites(shaderStage_t *stage, shaderCommands_t *input)
{
    unsigned long   glBits;
    fog_t           *fog;

    //
    // Update with the current frame.
    //
    R_SurfaceSpriteFrameUpdate();

    //
    // Check fog.
    //

    // Special provision in case the transparency is additive.
    glBits = stage->stateBits;

    if((glBits & (GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS)) == (GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE)){
        // Additive transparency, scale light value.
        additiveTransparency = qtrue;
    }else{
        additiveTransparency = qfalse;
    }

    if(tess.fogNum && tess.shader->fogPass && !additiveTransparency){
        fog = tr.world->fogs + tess.fogNum;

        RB_StartQuickSpriteRendering(&stage->bundle[0], glBits, fog->colorInt);
        usingFog = qtrue; // FIXME BOE: Apparently there is no using fog variable in SoF2MP?
    }else{
        RB_StartQuickSpriteRendering(&stage->bundle[0], glBits, 0);
        usingFog = qfalse;
    }

    //
    // Check if this a new entity transformation (incl. world entity),
    // and update the appropriate vectors if so.
    //
    if(backEnd.currentEntity != lastEntityDrawn){
        if(backEnd.currentEntity == &tr.worldEntity){
            // Drawing the world.
            VectorCopy(backEnd.viewParms.or.origin, viewOrigin);
            VectorCopy(backEnd.viewParms.or.axis[1], viewRight);
            VectorCopy(backEnd.viewParms.or.axis[2], viewUp);
        }else{
            // Drawing an entity. Transform the viewparms to the coordinate
            // system of the model.
            R_WorldNormalToEntity(backEnd.viewParms.or.axis[0], viewForward);
            R_WorldNormalToEntity(backEnd.viewParms.or.axis[1], viewRight);
            R_WorldNormalToEntity(backEnd.viewParms.or.axis[2], viewUp);
            VectorCopy(backEnd.or.viewOrigin, viewOrigin);
        }

        R_CreateRightVectors(/*stage->ss->??*/qfalse); // FIXME BOE
        lastEntityDrawn = backEnd.currentEntity;
    }

    switch(stage->ss->surfaceSpriteType){
        default:
            break;
    }

    RB_EndQuickSpriteRendering();
    numSurfaces++;
}
