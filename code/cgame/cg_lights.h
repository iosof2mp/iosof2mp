// Copyright (C) 2001-2002 Raven Software.
//
// cg_lights.h

#ifndef __CG_LIGHTS_H
#define __CG_LIGHTS_H

typedef struct {
    int         length;
    color4ub_t  value;
    color4ub_t  map[MAX_QPATH];
} clightstyle_t;

void    CG_ClearLightStyles ( void );
void    CG_RunLightStyles   ( void );
void    CG_SetLightstyle    ( int i );

#endif // __CG_LIGHTS_H
