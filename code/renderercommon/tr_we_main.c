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
// tr_we_main.c - Initialization and main functions of the world effect system.

#include "tr_common.h"
#include "tr_we.h"

// Local variable definitions.
static  worldEffectSystem_t     *worldEffectSystemList  = NULL;

//==============================================

/*
==================
R_ParseVectorArgument

Functionality-wise the same as the
shader parsing function ParseVector,
but with different warning messages.
==================
*/

qboolean R_ParseVectorArgument(char **text, int count, float *v, char *argDesc)
{
    char    *token;
    int     i;

    // FIXME: spaces are currently required after parens, should change parseext...
    token = COM_ParseExt(text, qfalse);
    if(strcmp(token, "(")){
        ri.Printf(PRINT_WARNING, "WARNING: missing '(' parenthesis in argument %s\n", argDesc);
        return qfalse;
    }

    for(i = 0; i < count; i++){
        token = COM_ParseExt(text, qfalse);
        if(!token[0]){
            ri.Printf(PRINT_WARNING, "WARNING: missing vector element in argument %s\n", argDesc);
            return qfalse;
        }
        v[i] = atof(token);
    }

    token = COM_ParseExt(text, qfalse);
    if(strcmp(token, ")")){
        ri.Printf(PRINT_WARNING, "WARNING: missing ')' parenthesis in argument %s\n", argDesc);
        return qfalse;
    }

    return qtrue;
}

//==============================================

/*
==================
R_IsWorldEffectSystemInitialized

Checks if a world effect system is already
initialized, based on name.

Returns the world effect system if a match
is found, NULL otherwise.
==================
*/

static worldEffectSystem_t *R_IsWorldEffectSystemInitialized(const char *name)
{
    worldEffectSystem_t *current;

    // Check if the world effect system list is initialized.
    if(!worldEffectSystemList){
        return NULL;
    }

    // Iterate through world effect systems, check if there is a match.
    current = worldEffectSystemList;
    while(current != NULL){
        if(Q_stricmp(current->name, name) == 0){
            return current;
        }

        current = current->nextSystem;
    }

    return NULL;
}

/*
==================
R_AddWorldEffectSystem

Adds a new world effect system to the end of
an existing list, or sets the list base to
this world effect system.
==================
*/

void R_AddWorldEffectSystem(worldEffectSystem_t *weSystem)
{
    worldEffectSystem_t *lastSystem;

    // If there is no world effect system initialized yet,
    // use this sytem as list base.
    if(!worldEffectSystemList){
        worldEffectSystemList = weSystem;
        return;
    }

    // Find the end of the list.
    lastSystem = worldEffectSystemList;
    while(lastSystem->nextSystem != NULL){
        lastSystem = lastSystem->nextSystem;
    }

    // Add the new world effect system to the end of the list.
    lastSystem->nextSystem = weSystem;
}

/*
==================
R_RemoveWorldEffectSystem

Fully remove a world effect system and free
all associated memory.
==================
*/

void R_RemoveWorldEffectSystem(worldEffectSystem_t *weSystem)
{
    worldEffectSystem_t *currentSystem;

    // Free allocated memory.
    // FIXME BOE

    // Update list indexes.
    if(worldEffectSystemList == weSystem){
        // The current system is the current list base.
        if(weSystem->nextSystem != NULL){
            worldEffectSystemList = weSystem->nextSystem;
        }else{
            // The single remaining system in the list is removed.
            // The list is now empty.
            worldEffectSystemList = NULL;
        }
    }else{
        // Iterate through the world effect system list.
        currentSystem = worldEffectSystemList;
        while(currentSystem != NULL){
            // Check if the next system in the list is the system
            // that is removed.
            if(currentSystem->nextSystem == weSystem){
                if(weSystem->nextSystem != NULL){
                    currentSystem->nextSystem = weSystem->nextSystem;
                }else{
                    // Last system in the list is removed.
                    currentSystem->nextSystem = NULL;
                }

                break;
            }

            currentSystem = currentSystem->nextSystem;
        }
    }

    // Free the world effect system itself.
    ri.Free(weSystem);
}

//==============================================

/*
==================
R_WorldEffectCommand

Any system defined should have a
command handler set in this
function.

If the name matches a defined world
effect system, the command is then
forwarded to the command handler of
that world effect system.
==================
*/

static void R_WorldEffectCommand(char *command)
{
    char                *systemName;
    worldEffectSystem_t *weSystem;

    // The first token is always the name
    // of the world effect system.
    systemName = COM_ParseExt(&command, qfalse);

    // All known world effect systems are checked here
    // and passed along to the command handler of that
    // system.
    weSystem = R_IsWorldEffectSystemInitialized(systemName);

    if(Q_stricmp(systemName, "") == 0){
        // FIXME BOE
    }else{
        ri.Printf(PRINT_ERROR, "ERROR: Unknown world effect system '%s'.\n", systemName);
    }
}

/*
==================
R_WorldEffect_f

Gets the console arguments and passes
it along to the generic world effect
system command handler function.
==================
*/

void R_WorldEffect_f(void)
{
    char cmd[2048];

    // Grab the console arguments.
    ri.Cmd_ArgsBuffer(cmd, sizeof(cmd));

    // Execute the command.
    R_WorldEffectCommand(cmd);
}
