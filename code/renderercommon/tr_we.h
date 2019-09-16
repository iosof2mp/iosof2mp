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

#define     MISTYFOG_WIDTH                  30
#define     MISTYFOG_HEIGHT                 30
#define     MISTYFOG_NUM_IMAGES             2
#define     MISTYFOG_NUM_PAIRED_IMAGES      2

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

typedef struct {
    worldEffect_t           base;

    vec4_t                  planes[3];
    float                   maxDistance[3];
    vec3_t                  velocity;
    int                     numPlanes;

    int                     affectedDuration;
    int                     *affectedParticles;
    vec3_t                  point, size;

    qboolean                isGlobal;
    qboolean                isEnabled;
} windEffect_t;

typedef struct {
    float                   textureCoords[2][2];

    float                   alpha, alphaDirection;
    qboolean                alphaFade;

    qboolean                isRendering;
    float                   speed;

    float                   currentSize;
    float                   minSize, maxSize;
    vec3_t                  windTransform;

    // Image data and properties.
    byte                    *data;
    int                     width, height;

    float                   size;
} mistyFogImage_t;

typedef struct {
    worldEffect_t           base;

    mistyFogImage_t         images[MISTYFOG_NUM_IMAGES][MISTYFOG_NUM_PAIRED_IMAGES];

    vec4_t                  colors[MISTYFOG_HEIGHT][MISTYFOG_WIDTH];
    vec3_t                  verts[MISTYFOG_HEIGHT][MISTYFOG_WIDTH];
    unsigned int            indexes[MISTYFOG_HEIGHT-1][MISTYFOG_WIDTH-1][4];

    float                   alpha;
    float                   fadeAlpha;
} mistyFogEffect_t;

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

    qboolean                isRendering;

    void                    (*Update)           ( worldEffectSystem_t *weSystem, float elapsedTime );
    void                    (*Render)           ( worldEffectSystem_t *weSystem );
};

typedef struct {
    worldEffectSystem_t     base;

    float                   rainHeight;
    float                   alpha;
    float                   windAngle;

    image_t                 *image;
    vec3_t                  spread;
    vec3_t                  minVelocity, maxVelocity;

    vec3_t                  windDirection;
    vec3_t                  windNewDirection;
    int                     windChange;

    float                   fadeAlpha;
} rainSystem_t;

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
    float                   velocityStabilize;
} snowSystem_t;

//=============================================

//
// tr_we backend.c
//

qboolean            RB_LoadMistyFogImage            ( mistyFogImage_t *fogImage, char *fileName );
void                RB_CreateMistyFogTextureCoords  ( mistyFogImage_t *fogImage );
void                RB_MistyFogEffectUpdate         ( worldEffectSystem_t *weSystem, worldEffect_t *effect, float elapsedTime );
void                RB_MistyFogEffectRender         ( worldEffectSystem_t *weSystem, worldEffect_t *effect );

void                RB_WindEffectUpdate             ( worldEffectSystem_t *weSystem, worldEffect_t *effect, float elapsedTime );

void                RB_LoadRainImage                ( rainSystem_t *rainSystem, const char *fileName );
void                RB_RainSystemUpdate             ( worldEffectSystem_t *weSystem, float elapsedTime );
void                RB_RainSystemRender             ( worldEffectSystem_t *weSystem );

void                RB_SnowSystemUpdate             ( worldEffectSystem_t *weSystem, float elapsedTime );
void                RB_SnowSystemRender             ( worldEffectSystem_t *weSystem );

//
// tr_we_main.c
//

qboolean            R_ParseVectorArgument           ( char **text, int count, float *v, char *argDesc );

worldEffect_t       *R_GetWorldEffect               ( worldEffectSystem_t *weSystem, const char *name );
worldEffect_t       *R_GetNextWorldEffect           ( worldEffect_t *effect );
void                R_AddWorldEffect                ( worldEffectSystem_t *weSystem, worldEffect_t *effect );
void                R_RemoveWorldEffect             ( worldEffectSystem_t *weSystem, worldEffect_t *effect );
void                R_UpdateWorldEffects            ( worldEffectSystem_t *weSystem, float elapsedTime );
void                R_RenderWorldEffects            ( worldEffectSystem_t *weSystem );
qboolean            R_WorldEffectCommand            ( worldEffectSystem_t *weSystem, char *command );

void                R_AddWorldEffectSystem          ( worldEffectSystem_t *weSystem );
void                R_RemoveWorldEffectSystem       ( worldEffectSystem_t *weSystem );
void                R_RenderWorldEffectSystems      ( float elapsedTime );

void                R_WorldEffect_f                 ( void );

//
// tr_we_mistyfog.c
//

void                R_UpdateMistyFogTextures        ( mistyFogEffect_t *mistyFogEffect );
void                R_UpdateMistyFogWindDirection   ( worldEffectSystem_t *weSystem, vec3_t windDirection );

mistyFogEffect_t    *R_AddMistyFogEffect            ( worldEffectSystem_t *weSystem, int fogFileIndex );

//
// tr_we_rain.c
//

void                R_RainSystemCommand             ( worldEffectSystem_t *weSystem, char *command );

//
// tr_we_snow.c
//

void                R_SnowSystemCommand             ( worldEffectSystem_t *weSystem, char *command );

//
// tr_we_wind.c
//

void                R_UpdateWindParams              ( windEffect_t *windEffect, vec3_t point, vec3_t velocity, vec3_t size );

windEffect_t        *R_AddWindEffect                ( worldEffectSystem_t *weSystem, qboolean isGlobalEffect );

#endif // __TR_WE__H
