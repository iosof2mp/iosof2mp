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
// tr_quicksprite.c - Implementation of the quick sprite system.

#include "tr_local.h"

static textureBundle_t  *texBundle;
static unsigned long    glStateBits;

static unsigned long    fogColor;
static qboolean         useFog;

static vec4_t           verts[SHADER_MAX_VERTEXES];
static vec2_t           textureCoords[SHADER_MAX_VERTEXES];
static unsigned long    colors[SHADER_MAX_VERTEXES];

static int              nextVert;
static qboolean         turnCullBackOn;

//==============================================

/*
==================
RB_RenderQuickSprites

Renders all sprites available in
the quick sprite system.

Also flushes all buffers.
==================
*/

static void RB_RenderQuickSprites(void)
{
    //fog_t *fog;

    if(nextVert == 0){
        return;
    }

    //
    // Render the main pass.
    //
    R_BindAnimatedImage(texBundle);
    GL_State(glStateBits);

    //
    // Render the fog pass.
    //
    if(useFog){
        // FIXME BOE: Hardware fog implementation.
        /*
        fog = &tr.world->fogs[tess.fogNum];

        // Enable hardware fog when we draw this sprite.
        qglFogf(GL_FOG_MODE, GL_EXP2);
        qglFogf(GL_FOG_DENSITY, logtestExp2 / fog->parms.depthForOpaque);
        qglFogfv(GL_FOG_COLOR, fog->parms.color);
        qglEnable(GL_FOG);
        */
    }

    //
    // Set arrays and lock.
    //
    qglTexCoordPointer(2, GL_FLOAT, 0, textureCoords);
    qglEnableClientState(GL_TEXTURE_COORD_ARRAY);

    qglColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
    qglEnableClientState(GL_COLOR_ARRAY);

    qglVertexPointer(3, GL_FLOAT, 16, verts);

    if(qglLockArraysEXT){
        qglLockArraysEXT(0, nextVert);
        GLimp_LogComment("glLockArraysEXT\n");
    }
    qglDrawArrays(GL_QUADS, 0, nextVert);

    //
    // Update backend counters.
    //
    backEnd.pc.c_vertexes       += nextVert;
    backEnd.pc.c_indexes        += nextVert;
    backEnd.pc.c_totalIndexes   += nextVert;

    //
    // Disable fog after drawing.
    //
    if(useFog){
        qglDisable(GL_FOG);
    }

    //
    // Unlock arrays.
    //
    if(qglUnlockArraysEXT){
        qglUnlockArraysEXT();
        GLimp_LogComment("glUnlockArraysEXT\n");
    }

    nextVert = 0;
}

//==============================================

/*
==================
RB_AddQuickSprite

Adds a quick sprite to the render list
of the quick sprite system.
==================
*/

void RB_AddQuickSprite(float *pointData, color4ub_t *color)
{
    unsigned long   *currColor;
    float           *currCoord;
    int             i;

    // Render all sprites in this quick sprite and flush
    // the buffers right away if there is no room left.
    if(nextVert > (SHADER_MAX_VERTEXES - 4)){
        RB_RenderQuickSprites();
    }

    // Store point data.
    currCoord = verts[nextVert];
    Com_Memcpy(currCoord, pointData, sizeof(vec4_t) * 4);

    // Setup the color.
    currColor = &colors[nextVert];
    for(i = 0; i < 4; i++){
        *currColor++ = *(unsigned long *)color;
    }

    nextVert += 4;
}

/*
==================
RB_StartQuickSpriteRendering

Signal the quick sprite system to
start rendering sprites.
==================
*/

void RB_StartQuickSpriteRendering(textureBundle_t *bundle, unsigned long stateBits, unsigned long fogColor)
{
    int cullingOn;

    // Store common information.
    texBundle = bundle;
    glStateBits = stateBits;
    nextVert = 0;

    // Check if we use fog.
    if(fogColor){
        fogColor = fogColor; // FIXME BOE: This doesn't appear to be used anywhere.
        useFog = qtrue;
    }else{
        useFog = qfalse;
    }

    // Check if we have to turn culling back on after
    // finishing up rendering the sprites.
    qglGetIntegerv(GL_CULL_FACE, &cullingOn);

    if(cullingOn){
        turnCullBackOn = qtrue;
    }else{
        turnCullBackOn = qfalse;
    }

    qglDisable(GL_CULL_FACE);
}

/*
==================
RB_EndQuickSpriteRendering

Signal the quick sprite system to
finish and stop rendering sprites.
==================
*/

void RB_EndQuickSpriteRendering(void)
{
    // Render all sprites.
    RB_RenderQuickSprites();

    // Finish this render pass.
    qglColor4ub(255, 255, 255, 255);
    if(turnCullBackOn){
        qglEnable(GL_CULL_FACE);
    }
}

/*
==================
R_InitQuickSpriteSystem

Initializes the quick sprite system.
==================
*/

void R_InitQuickSpriteSystem(void)
{
    int             i;

    // Set initial texture coordinates.
    for(i = 0; i < SHADER_MAX_VERTEXES; i += 4){
        // Bottom right.
        textureCoords[i + 0][0] = 1.0f;
        textureCoords[i + 0][1] = 1.0f;

        // Top right.
        textureCoords[i + 1][0] = 1.0f;
        textureCoords[i + 1][1] = 0.0f;

        // Top left.
        textureCoords[i + 2][0] = 0.0f;
        textureCoords[i + 2][1] = 0.0f;

        // Bottom left.
        textureCoords[i + 3][0] = 0.0f;
        textureCoords[i + 3][1] = 1.0f;
    }
}
