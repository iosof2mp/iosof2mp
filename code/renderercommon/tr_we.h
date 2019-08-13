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
// tr_we.h

#ifndef __TR_WE_H
#define __TR_WE_H

//=============================================
//
// Global world effect system definitions.
//

#define     SNOWCONTENTS_X_SIZE             16
#define     SNOWCONTENTS_Y_SIZE             16
#define     SNOWCONTENTS_Z_SIZE             8

typedef     struct      worldEffect_s           worldEffect_t;
typedef     struct      worldEffectSystem_s     worldEffectSystem_t;

//=============================================
//
// World effect particle definitions.
//

typedef struct {
    vec3_t                  pos;
    vec3_t                  velocity;
    unsigned int            flags;
} worldEffectParticle_t;

typedef enum {
    PARTICLE_FLAG_RENDER    = (1 << 0)
} worldEffectParticleFlags_t;

//=============================================
//
// World effect structures.
//

struct worldEffect_s {
    const char              *name;
    worldEffect_t           *nextEffect;

    void                    (*Update)           ( worldEffectSystem_t *weSystem, worldEffect_t *effect, float elapsedTime );
    void                    (*Render)           ( worldEffectSystem_t *weSystem, worldEffect_t *effect );

    qboolean                (*Command)          ( worldEffect_t *effect, char *command );
};

//=============================================
//
// World effect system structures.
//

struct worldEffectSystem_s {
    const char              *name;
    worldEffectSystem_t     *nextSystem;

    worldEffect_t           *worldEffectList;

    worldEffectParticle_t   *particleList;
    int                     numParticles;

    void                    (*Update)           ( worldEffectSystem_t *weSystem, float elapsedTime );
    void                    (*Render)           ( worldEffectSystem_t *weSystem );
};

typedef struct {
    worldEffectSystem_t     base;

    float                   alpha;
    vec3_t                  minSpread, maxSpread;
    vec3_t                  minVelocity, maxVelocity;
    float                   windDuration, windLow;
    float                   windMin, windMax;
    vec3_t                  windSize;

    vec3_t                  mins, maxs;
    float                   nextWindGust, windLowSize;

    vec3_t                  windDirection, windSpeed;
    int                     windChange;

    int                     contents[SNOWCONTENTS_Z_SIZE][SNOWCONTENTS_Y_SIZE][SNOWCONTENTS_X_SIZE];
    vec3_t                  contentsSize, contentsStart;

    int                     overallContents;
    qboolean                isRendering;

    float                   velocityStabilize;
} snowSystem_t;

//=============================================

//
// tr_we backend.c
//

void            RB_SnowSystemUpdate             ( worldEffectSystem_t *weSystem, float elapsedTime );
void            RB_SnowSystemRender             ( worldEffectSystem_t *weSystem );

//
// tr_we_main.c
//

qboolean        R_ParseVectorArgument           ( char **text, int count, float *v, char *argDesc );

worldEffect_t   *R_GetWorldEffect               ( worldEffectSystem_t *weSystem, const char *name );
worldEffect_t   *R_GetNextWorldEffect           ( worldEffect_t *effect );
void            R_AddWorldEffect                ( worldEffectSystem_t *weSystem, worldEffect_t *effect );
void            R_RemoveWorldEffect             ( worldEffectSystem_t *weSystem, worldEffect_t *effect );
void            R_UpdateWorldEffects            ( worldEffectSystem_t *weSystem, float elapsedTime );
void            R_RenderWorldEffects            ( worldEffectSystem_t *weSystem );
qboolean        R_WorldEffectCommand            ( worldEffectSystem_t *weSystem, char *command );

void            R_AddWorldEffectSystem          ( worldEffectSystem_t *weSystem );
void            R_RemoveWorldEffectSystem       ( worldEffectSystem_t *weSystem );
void            R_RenderWorldEffectSystems      ( float elapsedTime );

void            R_WorldEffect_f                 ( void );

//
// tr_we_snow.c
//

void            R_SnowSystemCommand             ( worldEffectSystem_t *weSystem, char *command );

#endif // __TR_WE__H
