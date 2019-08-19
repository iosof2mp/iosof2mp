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
// tr_we_mistyfog.c - Implementation of the misty fog world effect.

#include "tr_common.h"
#include "tr_we.h"

/*
==================
R_UpdateMistyFogTextures

Updates the misty fog colors
to render with data from all
misty fog images present.
==================
*/

void R_UpdateMistyFogTextures(mistyFogEffect_t *mistyFogEffect)
{
    int             x, y, tx, ty;
    float           xSize, ySize;
    float           xStep, yStep;
    float           xPos, yPos;
    int             andWidth, andHeight;
    float           alpha;
    float           *color;
    int             i, j;
    mistyFogImage_t *fogImage;

    for(i = 0; i < MISTYFOG_NUM_IMAGES; i++){
        for(j = 0; j < MISTYFOG_NUM_PAIRED_IMAGES; j++){
            fogImage = &mistyFogEffect->images[i][j];

            if(!fogImage->isRendering){
                continue;
            }

            alpha       = fogImage->alpha * mistyFogEffect->alpha * (1.0f / 255.0f) * mistyFogEffect->fadeAlpha;
            andWidth    = fogImage->width - 1;
            andHeight   = fogImage->height - 1;
            xSize       = fogImage->textureCoords[1][0] - fogImage->textureCoords[0][0];
            ySize       = fogImage->textureCoords[1][1] - fogImage->textureCoords[0][1];
            xStep       = xSize / MISTYFOG_WIDTH;
            yStep       = ySize / MISTYFOG_HEIGHT;

            color = &mistyFogEffect->colors[0][0][3];
            for(y = 0, yPos = fogImage->textureCoords[0][1]; y < MISTYFOG_HEIGHT; y++, yPos += yStep){
                for(x = 0, xPos = fogImage->textureCoords[0][0]; x < MISTYFOG_WIDTH; x++, xPos += xStep){
                    tx = xPos * fogImage->width;
                    tx &= andWidth;

                    ty = yPos * fogImage->height;
                    ty &= andHeight;

                    (*color) += fogImage->data[(ty * fogImage->width + tx) * 4] * alpha;
                    color += sizeof(*color);
                }
            }
        }
    }
}

/*
==================
R_UpdateMistyFogWindDirection

Updates all present misty fog
images with the specified
wind direction.
==================
*/

void R_UpdateMistyFogWindDirection(worldEffectSystem_t *weSystem, vec3_t windDirection)
{
    worldEffect_t       *effect;
    mistyFogEffect_t    *mistyFogEffect;
    int                 i, j;

    effect = R_GetWorldEffect(weSystem, "fog");
    while(effect != NULL){
        mistyFogEffect = (mistyFogEffect_t *)effect;

        // Update wind direction for all effect images.
        for(i = 0; i < MISTYFOG_NUM_IMAGES; i++){
            for(j = 0; j < MISTYFOG_NUM_PAIRED_IMAGES; j++){
                VectorCopy(windDirection, mistyFogEffect->images[i][j].windTransform);
            }
        }

        effect = R_GetNextWorldEffect(effect);
    }
}

//==============================================

/*
==================
R_MistyFogEffectCommand

Command handler of the misty
fog world effect.
==================
*/

static qboolean R_MistyFogEffectCommand(worldEffect_t *effect, char *command)
{
    mistyFogEffect_t    *mistyFogEffect;
    char                *token;

    mistyFogEffect  = (mistyFogEffect_t *)effect;
    token           = COM_ParseExt(&command, qfalse);

    // fog
    if(Q_stricmp(token, "fog") == 0){
        token = COM_ParseExt(&command, qfalse);

        // fog density
        if(Q_stricmp(token, "density") == 0){
            float   density;

            token = COM_ParseExt(&command, qfalse);
            density = atof(token);
            if(!density){
                ri.Printf(PRINT_ALL, "Usage: fog density <density>\n");
                ri.Printf(PRINT_ALL, "Default: 0.30\n");
            }else{
                mistyFogEffect->alpha = density;
            }

            return qtrue;
        }
    }

    return qfalse;
}

/*
==================
R_AddMistyFogEffect

Adds a new misty fog effect
instance to the specified
world effect system.
==================
*/

mistyFogEffect_t *R_AddMistyFogEffect(worldEffectSystem_t *weSystem, int fogFileIndex)
{
    mistyFogEffect_t    *mistyFogEffect;
    mistyFogImage_t     *mainImage, *buddyImage;
    char                fileName[MAX_QPATH];
    float               xStep, yStep;
    int                 i, x, y;

    //
    // Initialize new misty fog effect instance.
    //
    mistyFogEffect = ri.Malloc(sizeof(mistyFogEffect_t));
    Com_Memset(mistyFogEffect, 0, sizeof(mistyFogEffect_t));

    mistyFogEffect->base.name = "fog";

    mistyFogEffect->base.Update = RB_MistyFogEffectUpdate;
    mistyFogEffect->base.Render = RB_MistyFogEffectRender;
    mistyFogEffect->base.Command = R_MistyFogEffectCommand;

    //
    // Set misty fog effect information.
    //
    mistyFogEffect->alpha = 0.3f;

    // Initialize fog images.
    Com_sprintf(fileName, sizeof(fileName), "gfx/world/fog%d.png", fogFileIndex);
    for(i = 0; i < MISTYFOG_NUM_IMAGES; i++){
        mainImage   = &mistyFogEffect->images[i][0];
        buddyImage  = &mistyFogEffect->images[i][1];

        if(!RB_LoadMistyFogImage(mainImage, fileName)){
            ri.Error(ERR_DROP, "R_AddMistyFogEffect: Could not load fog image \"%s\".\n", fileName);
            return NULL;
        }

        mainImage->size         = 0.05f * 2.0f;
        mainImage->minSize      = 0.05f * 3.0f;
        mainImage->maxSize      = 0.15f * 2.0f;

        mainImage->alpha        = 1.0f;

        Com_Memcpy(buddyImage, mainImage, sizeof(mistyFogImage_t));

        mainImage->isRendering  = qtrue;

        RB_CreateMistyFogTextureCoords(mainImage);
        RB_CreateMistyFogTextureCoords(buddyImage);
    }

    // Initialize fog effect.
    xStep = 20.0f / (MISTYFOG_WIDTH - 1);
    yStep = 20.0f / (MISTYFOG_HEIGHT - 1);

    for(y = 0; y < MISTYFOG_HEIGHT; y++){
        for(x = 0; x < MISTYFOG_WIDTH; x++){
            mistyFogEffect->verts[y][x][0] = -10.0f + (x * xStep) + flrand(-xStep / 16.0f, xStep / 16.0f);
            mistyFogEffect->verts[y][x][1] = 10.0f - (y * yStep) + flrand(-xStep / 16.0f, xStep / 16.0f);
            mistyFogEffect->verts[y][x][2] = -10.0f;

            VectorSet(mistyFogEffect->colors[y][x], 1.0f, 1.0f, 1.0f);

            if(y < MISTYFOG_HEIGHT - 1 && x < MISTYFOG_WIDTH - 1){
                mistyFogEffect->indexes[y][x][0] = y * MISTYFOG_WIDTH + x;
                mistyFogEffect->indexes[y][x][1] = y * MISTYFOG_WIDTH + x + 1;
                mistyFogEffect->indexes[y][x][2] = (y + 1) * MISTYFOG_WIDTH + x + 1;
                mistyFogEffect->indexes[y][x][3] = (y + 1) * MISTYFOG_WIDTH + x;
            }
        }
    }

    //
    // Add the new instance to the world effect list
    // of the parent world effect system.
    //
    R_AddWorldEffect(weSystem, (worldEffect_t *)mistyFogEffect);

    return mistyFogEffect;
}
