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
// tr_we_rain.c - Implementation of the rain world effect system.

#include "tr_common.h"
#include "tr_we.h"

/*
==================
R_RainSystemInitialize

Initializes a new instance of
the rain world effect system.
==================
*/

static worldEffectSystem_t *R_RainSystemInitialize(int maxRaindrops)
{
    rainSystem_t            *rainSystem;
    worldEffectParticle_t   *item;
    int                     i, j;

    //
    // Allocate memory for the new rain system instance.
    //
    rainSystem = ri.Malloc(sizeof(rainSystem_t));
    Com_Memset(rainSystem, 0, sizeof(rainSystem_t));

    //
    // Set base world effect system information.
    //
    rainSystem->base.name           = "rain";

    rainSystem->base.particleList   = ri.Malloc(sizeof(worldEffectParticle_t) * maxRaindrops);
    rainSystem->base.numParticles   = maxRaindrops;

    // TODO: Set proper functions.
    rainSystem->base.Update         = NULL;
    rainSystem->base.Render         = NULL;

    //
    // Set snow system information.
    //
    rainSystem->rainHeight = 5.0f;
    rainSystem->alpha = 0.1f;
    rainSystem->windAngle = 1.0f;

    VectorSet(rainSystem->spread, M_PI * 2.0f, 20.0f, 20.0f);
    VectorSet(rainSystem->minVelocity, 0.1f, 0.1f, -60.0f);
    VectorSet(rainSystem->maxVelocity, -0.1f, -0.1f, -50.0f);

    // TODO: Bind texture in backend.

    //
    // Set raindrop particle defaults.
    //
    Com_Memset(rainSystem->base.particleList, 0, sizeof(worldEffectParticle_t) * maxRaindrops);

    for(i = 0; i < maxRaindrops; i++){
        item = &rainSystem->base.particleList[i];

        item->pos[0] = flrand(0.0f, rainSystem->spread[0]);
        item->pos[1] = flrand(0.0f, rainSystem->spread[1]);
        item->pos[2] = flrand(-rainSystem->spread[2], 40.0f);

        for(j = 0; j < 2; j++){
            item->velocity[j] = flrand(rainSystem->minVelocity[j], rainSystem->maxVelocity[j]);
        }
    }

    return (worldEffectSystem_t *)rainSystem;
}

//==============================================

/*
==================
R_RainSystemCommand

Command handler of the rain
world effect system.
==================
*/

void R_RainSystemCommand(worldEffectSystem_t *weSystem, char *command)
{
    char            *originalCommand;
    char            *token;
    int             maxRaindrops;
    rainSystem_t    *rainSystem;

    originalCommand = command;
    token           = COM_ParseExt(&command, qfalse);

    //
    // Rain system initialization.
    //

    // rain init
    if(Q_stricmp(token, "init") == 0){
        // Check if particle count is valid.
        token = COM_ParseExt(&command, qfalse);
        maxRaindrops = atoi(token);

        if(!maxRaindrops){
            ri.Printf(PRINT_ERROR, "ERROR: Invalid raindrop count, not (re-)initializing rain system.\n");
            ri.Printf(PRINT_ALL, "Usage: rain init <raindrops>\n");
            return;
        }

        // Remove a previously initialized rain system, if present.
        if(weSystem != NULL){
            R_RemoveWorldEffectSystem(weSystem);

            weSystem = R_RainSystemInitialize(maxRaindrops);
            ri.Printf(PRINT_ALL, "Rain system re-initialized.\n");
        }else{
            weSystem = R_RainSystemInitialize(maxRaindrops);
            ri.Printf(PRINT_ALL, "Rain system initialized.\n");
        }

        // Add the new instance to the world effect system list.
        R_AddWorldEffectSystem(weSystem);

        return;
    }

    //
    // Rain system must be initialized for any other command.
    //
    if(weSystem == NULL){
        ri.Printf(PRINT_ERROR, "ERROR: Rain system not initialized yet. Not executing command.\n");
        return;
    }

    //
    // Check if this command belongs to any world effect.
    //
    if(R_WorldEffectCommand(weSystem, originalCommand)){
        return;
    }

    //
    // Remaining commands.
    //
    rainSystem  = (rainSystem_t *)weSystem;
    command     = originalCommand;
    token       = COM_ParseExt(&command, qfalse);

    // rain remove
    if(Q_stricmp(token, "remove") == 0){
        R_RemoveWorldEffectSystem(weSystem);

        ri.Printf(PRINT_ALL, "Rain system removed.\n");
    }
    // rain fog
    else if(Q_stricmp(token, "fog") == 0){
        R_AddMistyFogEffect(weSystem, 2);
        rainSystem->windChange = 0;
    }
    // rain fall
    else if(Q_stricmp(token, "fall") == 0){
        vec2_t  velocity;

        if(!R_ParseVectorArgument(&command, 2, velocity, "fall")){
            ri.Printf(PRINT_ALL, "Usage: rain fall ( minVelocity maxVelocity )\n");
            ri.Printf(PRINT_ALL, "Defaults: ( -60.0 -50.0 )\n");
            return;
        }

        rainSystem->minVelocity[2] = velocity[0];
        rainSystem->maxVelocity[2] = velocity[1];
    }
    // rain spread
    else if(Q_stricmp(token, "spread") == 0){
        vec2_t  spread;

        if(!R_ParseVectorArgument(&command, 2, spread, "spread")){
            ri.Printf(PRINT_ALL, "Usage: rain spread ( radius height )\n");
            ri.Printf(PRINT_ALL, "Defaults: ( 20.0 20.0 )\n");
            return;
        }

        rainSystem->spread[1] = spread[0];
        rainSystem->spread[2] = spread[1];
    }
    // rain alpha
    else if(Q_stricmp(token, "alpha") == 0){
        float   alpha;

        token = COM_ParseExt(&command, qfalse);
        alpha = atof(token);
        if(!alpha){
            ri.Printf(PRINT_ALL, "Usage: rain alpha <alpha>\n");
            ri.Printf(PRINT_ALL, "Default: 0.10\n");
            return;
        }

        rainSystem->alpha = alpha;
    }
    // rain height
    else if(Q_stricmp(token, "height") == 0){
        float   height;

        token = COM_ParseExt(&command, qfalse);
        height = atof(token);
        if(!height){
            ri.Printf(PRINT_ALL, "Usage: rain height <height>\n");
            ri.Printf(PRINT_ALL, "Default: 5.0\n");
            return;
        }

        rainSystem->rainHeight = height;
    }
    // rain angle
    else if(Q_stricmp(token, "angle") == 0){
        float   angle;

        token = COM_ParseExt(&command, qfalse);
        angle = atof(token);
        if(!angle){
            ri.Printf(PRINT_ALL, "Usage: rain angle <angle>\n");
            ri.Printf(PRINT_ALL, "Default: 1.0\n");
            return;
        }

        rainSystem->windAngle = angle;
    }else{
        ri.Printf(PRINT_ERROR, "ERROR: Unknown rain system command.\n");
    }
}
