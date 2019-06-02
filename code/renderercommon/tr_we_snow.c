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
// tr_we_snow.c - Implementation of the snow world effect system.

#include "tr_common.h"
#include "tr_we.h"

/*
==================
R_SnowSystemInitialize

Initializes a new instance of
the snow world effect system.
==================
*/

static worldEffectSystem_t *R_SnowSystemInitialize(int maxSnowflakes)
{
    snowSystem_t            *snowSystem;
    worldEffectParticle_t   *item;
    int                     i;

    //
    // Allocate memory for the new snow system instance.
    //
    snowSystem = ri.Malloc(sizeof(snowSystem_t));
    Com_Memset(snowSystem, 0, sizeof(snowSystem_t));

    //
    // Set base world effect system information.
    //
    snowSystem->base.name           = "snow";

    snowSystem->base.particleList   = ri.Malloc(sizeof(worldEffectParticle_t) * maxSnowflakes);
    snowSystem->base.numParticles   = maxSnowflakes;

    // TODO: Set proper functions.
    snowSystem->base.Update         = NULL;
    snowSystem->base.Render         = NULL;

    //
    // Set snow system information.
    //
    snowSystem->alpha = 0.09f;

    VectorSet(snowSystem->minSpread, -600, -600, -200);
    VectorSet(snowSystem->maxSpread, 600, 600, 250);
    VectorSet(snowSystem->minVelocity, -15, -15, -20);
    VectorSet(snowSystem->maxVelocity, 15, 15, -70);

    snowSystem->windDuration = 2.0f;
    snowSystem->windLow = 3.0f;
    snowSystem->windMin = 30.0f;
    snowSystem->windMax = 70.0f;

    VectorSet(snowSystem->windSize, 1000, 300, 300);
    VectorSet(snowSystem->mins, 99999, 99999, 99999);
    VectorSet(snowSystem->maxs, 99999, 99999, 99999);

    snowSystem->contentsSize[0] = (snowSystem->maxSpread[0] - snowSystem->minSpread[0]) / SNOWCONTENTS_X_SIZE;
    snowSystem->contentsSize[1] = (snowSystem->maxSpread[1] - snowSystem->minSpread[1]) / SNOWCONTENTS_Y_SIZE;
    snowSystem->contentsSize[2] = (snowSystem->maxSpread[2] - snowSystem->minSpread[2]) / SNOWCONTENTS_Z_SIZE;

    snowSystem->velocityStabilize = 18.0f;

    //
    // Set snowflake particle defaults.
    //
    Com_Memset(snowSystem->base.particleList, 0, sizeof(worldEffectParticle_t) * maxSnowflakes);

    item = snowSystem->base.particleList;
    for(i = 0; i < maxSnowflakes; i++){
        VectorSet(item->pos, 99999, 99999, 99999);
        item++;
    }

    return (worldEffectSystem_t *)snowSystem;
}

//==============================================

/*
==================
R_SnowSystemCommand

Command handler of the snow
world effect system.
==================
*/

void R_SnowSystemCommand(worldEffectSystem_t *weSystem, char *command)
{
    char            *token;
    int             maxSnowflakes;
    snowSystem_t    *snowSystem;

    token = COM_ParseExt(&command, qfalse);

    //
    // Snow system initialization.
    //

    // snow init <particles>
    if(Q_stricmp(token, "init") == 0){
        // Check if particle count is valid.
        token = COM_ParseExt(&command, qfalse);
        maxSnowflakes = atoi(token);

        if(!maxSnowflakes){
            ri.Printf(PRINT_ERROR, "ERROR: Invalid snowflake count, not (re-)initializing snow system.\n");
            ri.Printf(PRINT_ALL, "Usage: snow init <snowflakes>\n");
            return;
        }

        // Remove a previously initialized snow system, if present.
        if(weSystem != NULL){
            R_RemoveWorldEffectSystem(weSystem);

            weSystem = R_SnowSystemInitialize(maxSnowflakes);
            ri.Printf(PRINT_ALL, "Snow system re-initialized.\n");
        }else{
            weSystem = R_SnowSystemInitialize(maxSnowflakes);
            ri.Printf(PRINT_ALL, "Snow system initialized.\n");
        }

        // Add the new instance to the world effect system list.
        R_AddWorldEffectSystem(weSystem);

        return;
    }

    //
    // Snow system must be initialized for any other command.
    //
    if(weSystem == NULL){
        ri.Printf(PRINT_ERROR, "ERROR: Snow system not initialized yet. Not executing command.\n");
        return;
    }

    //
    // Remaining commands.
    //
    snowSystem = (snowSystem_t *)weSystem;

    // snow remove
    if(Q_stricmp(token, "remove") == 0){
        R_RemoveWorldEffectSystem(weSystem);

        ri.Printf(PRINT_ALL, "Snow system removed.\n");
    }
    // snow wind
    else if(Q_stricmp(token, "wind") == 0){
        vec3_t  windOrigin;
        vec3_t  windVelocity;
        vec3_t  size;

        if(!R_ParseVectorArgument(&command, 3, windOrigin, "origin")
            || !R_ParseVectorArgument(&command, 3, windVelocity, "velocity")
            || !R_ParseVectorArgument(&command, 3, size, "size")
        ){
            ri.Printf(PRINT_ALL, "Usage: snow wind ( originX originY originZ ) ( velocityX velocityY velocityZ ) ( sizeX sizeY sizeZ )\n");
            ri.Printf(PRINT_ALL, "Defaults: N/A\n");
            return;
        }

        // TODO: Add wind effect.
    }
    // snow fog
    else if(Q_stricmp(token, "fog") == 0){
        // TODO: Add fog effect.
        snowSystem->windChange = 0;
    }
    // snow alpha
    else if(Q_stricmp(token, "alpha") == 0){
        float   alpha;

        token = COM_ParseExt(&command, qfalse);
        alpha = atof(token);
        if(!alpha){
            ri.Printf(PRINT_ALL, "Usage: snow alpha <alpha>\n");
            ri.Printf(PRINT_ALL, "Default: 0.09\n");
        }

        snowSystem->alpha = atof(token);
    }
    // snow spread
    else if(Q_stricmp(token, "spread") == 0){
        vec3_t  minSpread;
        vec3_t  maxSpread;

        if(!R_ParseVectorArgument(&command, 3, minSpread, "min")
            || !R_ParseVectorArgument(&command, 3, maxSpread, "max")
        ){
            ri.Printf(PRINT_ALL, "Usage: snow spread ( minX minY minZ ) ( maxX maxY maxZ )\n");
            ri.Printf(PRINT_ALL, "Defaults: ( -600.0 -600.0 -200.0 ) ( 600.0 600.0 250.0 )\n");
            return;
        }

        VectorCopy(minSpread, snowSystem->minSpread);
        VectorCopy(maxSpread, snowSystem->maxSpread);
    }
    // snow velocity
    else if(Q_stricmp(token, "velocity") == 0){
        vec3_t  minVelocity;
        vec3_t  maxVelocity;

        if(!R_ParseVectorArgument(&command, 3, minVelocity, "min")
            || !R_ParseVectorArgument(&command, 3, maxVelocity, "max")
        ){
            ri.Printf(PRINT_ALL, "Usage: snow velocity ( minX minY minZ ) ( maxX maxY maxZ )\n");
            ri.Printf(PRINT_ALL, "Defaults: ( -15.0 -15.0 -20.0 ) ( 15.0 15.0 -70.0 )\n");
            return;
        }

        VectorCopy(minVelocity, snowSystem->minVelocity);
        VectorCopy(maxVelocity, snowSystem->maxVelocity);
    }
    // snow blowing
    else if(Q_stricmp(token, "blowing") == 0){
        token = COM_ParseExt(&command, qfalse);

        // snow blowing duration
        if(Q_stricmp(token, "duration") == 0){
            long    duration;

            token = COM_ParseExt(&command, qfalse);
            duration = atol(token);
            if(!duration){
                ri.Printf(PRINT_ALL, "Usage: snow blowing duration <seconds>\n");
                ri.Printf(PRINT_ALL, "Default: 2\n");
            }

            snowSystem->windDuration = duration;
        }
        // snow blowing low
        else if(Q_stricmp(token, "low") == 0){
            long    low;

            token = COM_ParseExt(&command, qfalse);
            low = atol(token);
            if(!low){
                ri.Printf(PRINT_ALL, "Usage: snow blowing low <seconds>\n");
                ri.Printf(PRINT_ALL, "Default: 3\n");
            }

            snowSystem->windLow = low;
        }
        // snow blowing velocity
        else if(Q_stricmp(token, "velocity") == 0){
            vec2_t  windVelocity;

            if(!R_ParseVectorArgument(&command, 2, windVelocity, "windVelocity")){
                ri.Printf(PRINT_ALL, "Usage: snow blowing velocity ( min max )\n");
                ri.Printf(PRINT_ALL, "Defaults: ( 30.0 70.0 )\n");
                return;
            }

            snowSystem->windMin = windVelocity[0];
            snowSystem->windMax = windVelocity[1];
        }
        // snow blowing size
        else if(Q_stricmp(token, "size") == 0){
            vec3_t  windSize;

            if(!R_ParseVectorArgument(&command, 3, windSize, "size")){
                ri.Printf(PRINT_ALL, "Usage: snow blowing velocity ( sizeX sizeY sizeZ )\n");
                ri.Printf(PRINT_ALL, "Defaults: ( 1000.0 300.0 300.0 )\n");
                return;
            }

            VectorCopy(windSize, snowSystem->windSize);
        }
    }else{
        ri.Printf(PRINT_ERROR, "ERROR: Unknown snow system command.\n");
    }
}
