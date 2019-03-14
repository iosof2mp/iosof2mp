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

/*
==================
R_InitQuickSprite

Allocates and initializes a new quick sprite
and returns the new instance.
==================
*/

quickSprite_t *R_InitQuickSprite(void)
{
    quickSprite_t   *qs;
    int             i;

    // Allocate memory for this new quick sprite system instance.
    qs = Hunk_Alloc(sizeof(quickSprite_t), h_high);

    // Set initial texture coordinates.
    for(i = 0; i < SHADER_MAX_VERTEXES; i += 4){
        // Bottom right.
        qs->mTextureCoords[i + 0][0] = 1.0;
        qs->mTextureCoords[i + 0][1] = 1.0;

        // Top right.
        qs->mTextureCoords[i + 1][0] = 1.0;
        qs->mTextureCoords[i + 1][1] = 0.0;

        // Top left.
        qs->mTextureCoords[i + 2][0] = 0.0;
        qs->mTextureCoords[i + 2][1] = 0.0;

        // Bottom left.
        qs->mTextureCoords[i + 3][0] = 0.0;
        qs->mTextureCoords[i + 3][1] = 1.0;
    }

    return qs;
}

/*
==================
RB_RenderQuickSprite

Renders all sprites available in
this quick sprite instance.

Also flushes all buffers.
==================
*/

static void RB_RenderQuickSprite(quickSprite_t *qs)
{
    //fog_t *fog;

    if(qs->mNextVert == 0){
        return;
    }

    //
    // Render the main pass.
    //
    R_BindAnimatedImage(qs->mTexBundle);
    GL_State(qs->mGLStateBits);

    //
    // Render the fog pass.
    //
    if(qs->mUseFog){
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
    qglTexCoordPointer(2, GL_FLOAT, 0, qs->mTextureCoords);
    qglEnableClientState(GL_TEXTURE_COORD_ARRAY);

    qglColorPointer(4, GL_UNSIGNED_BYTE, 0, qs->mColors);
    qglEnableClientState(GL_COLOR_ARRAY);

    qglVertexPointer(3, GL_FLOAT, 16, qs->mVerts);

    if(qglLockArraysEXT){
        qglLockArraysEXT(0, qs->mNextVert);
        GLimp_LogComment("glLockArraysEXT\n");
    }
    qglDrawArrays(GL_QUADS, 0, qs->mNextVert);

    //
    // Update backend counters.
    //
    backEnd.pc.c_vertexes       += qs->mNextVert;
    backEnd.pc.c_indexes        += qs->mNextVert;
    backEnd.pc.c_totalIndexes   += qs->mNextVert;

    //
    // Disable fog after drawing.
    //
    if(qs->mUseFog){
        qglDisable(GL_FOG);
    }

    //
    // Unlock arrays.
    //
    if(qglUnlockArraysEXT){
        qglUnlockArraysEXT();
        GLimp_LogComment("glUnlockArraysEXT\n");
    }

    qs->mNextVert = 0;
}

/*
==================
RB_AddSprite

Adds a sprite to the render list of the
specified quick sprite instance.
==================
*/

void RB_AddSprite(quickSprite_t *qs, float *pointData, color4ub_t *color)
{
    unsigned long   *currColor;
    float           *currCoord;
    int             i;

    // Render all sprites in this quick sprite and flush
    // the buffers right away if there is no room left.
    if(qs->mNextVert > (SHADER_MAX_VERTEXES - 4)){
        RB_RenderQuickSprite(qs);
    }

    // Store point data.
    currCoord = qs->mVerts[qs->mNextVert];
    Com_Memcpy(currCoord, pointData, sizeof(vec4_t) * 4);

    // Setup the color.
    currColor = &qs->mColors[qs->mNextVert];
    for(i = 0; i < 4; i++){
        *currColor++ = *(unsigned long *)color;
    }

    qs->mNextVert += 4;
}

/*
==================
RB_StartQuickSpriteRendering

Signal the quick sprite system to
start rendering sprites.
==================
*/

void RB_StartQuickSpriteRendering(quickSprite_t *qs, textureBundle_t *bundle, unsigned long stateBits, unsigned long fogColor)
{
    int cullingOn;

    // Store common information.
    qs->mTexBundle = bundle;
    qs->mGLStateBits = stateBits;
    qs->mNextVert = 0;

    // Check if we use fog.
    if(fogColor){
        qs->mFogColor = fogColor; // FIXME BOE: This doesn't appear to be used anywhere.
        qs->mUseFog = qtrue;
    }else{
        qs->mUseFog = qfalse;
    }

    // Check if we have to turn culling back on after
    // finishing up rendering the sprites.
    qglGetIntegerv(GL_CULL_FACE, &cullingOn);

    if(cullingOn){
        qs->mTurnCullBackOn = qtrue;
    }else{
        qs->mTurnCullBackOn = qfalse;
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

void RB_EndQuickSpriteRendering(quickSprite_t *qs)
{
    // Render all sprites.
    RB_RenderQuickSprite(qs);

    // Finish this render pass.
    qglColor4ub(255, 255, 255, 255);
    if(qs->mTurnCullBackOn){
        qglEnable(GL_CULL_FACE);
    }
}
