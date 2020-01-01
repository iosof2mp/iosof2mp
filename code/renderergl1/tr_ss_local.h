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
// tr_ss_local.h

#ifndef __TR_SS_LOCAL_H
#define __TR_SS_LOCAL_H

//=============================================
//
// Global surface sprite system definitions.
//

#define     WIND_DAMP_INTERVAL      50.0f
#define     WIND_GUST_TIME          2500.0f
#define     WIND_GUST_DECAY         (1.0f / WIND_GUST_TIME)

#define     WINDPOINT_RADIUS        750.0f
#define     FADE_RANGE              250.0f

//=============================================
//
// Surface sprite system structures.
//

typedef struct {
    float       curWindSpeed;
    float       curWeatherAmount;

    float       rangeScaleFactor;
    float       rangeScaleDistance;

    vec3_t      curWindBlowVect;
    vec3_t      curWindGrassDir;

    qboolean    curWindPointActive;
    float       curWindPointForce;
    vec3_t      curWindPoint;

    vec3_t      rightVectors[4];
    int         rightVectorCount;
    vec3_t      fwdVector;

    qboolean    additiveTransparency;
    qboolean    usingFog;

    int         numSurfaceSprites;
} surfaceSpriteSystem_t;

#endif // __TR_SS_LOCAL_H
