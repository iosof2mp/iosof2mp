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
R_GetWorldEffect

Checks if there is a world effect in the list
with the specified name.

Returns the world effect if a match is found,
NULL otherwise.
==================
*/

worldEffect_t *R_GetWorldEffect(worldEffectSystem_t *weSystem, const char *name)
{
    worldEffect_t   *current;

    // Must have a valid list to iterate through.
    if(weSystem->worldEffectList == NULL){
        return NULL;
    }

    // Iterate through the world effect list, check if there is a match.
    current = weSystem->worldEffectList;
    while(current != NULL){
        if(Q_stricmp(current->name, name) == 0){
            return current;
        }

        current = current->nextEffect;
    }

    return NULL;
}

/*
==================
R_GetNextEffect

Gets a next world effect in the list of
the same type.

Returns the next world effect if a match
is found, NULL otherwise.
==================
*/

worldEffect_t *R_GetNextEffect(worldEffect_t *effect)
{
    worldEffect_t   *current;

    // Must not be the end of the world effect list.
    if(effect->nextEffect == NULL){
        return NULL;
    }

    // Iterate through remaining effects, start at the next effect in the list.
    current = effect->nextEffect;
    while(current != NULL){
        if(Q_stricmp(current->name, effect->name) == 0){
            return current;
        }

        current = current->nextEffect;
    }

    return NULL;
}

/*
==================
R_AddWorldEffect

Adds a new world effect to the end of the
existing effect list, or sets this effect
as the effect list base of the specified
world effect system.
==================
*/

void R_AddWorldEffect(worldEffectSystem_t *weSystem, worldEffect_t *effect)
{
    worldEffect_t   *lastEffect;

    // If there is no world effect initialized yet,
    // use this effect as list base.
    if(!weSystem->worldEffectList){
        weSystem->worldEffectList = effect;
        return;
    }

    // Find the end of the list.
    lastEffect = weSystem->worldEffectList;
    while(lastEffect->nextEffect != NULL){
        lastEffect = lastEffect->nextEffect;
    }

    // Add the new world effect to the end of the list.
    lastEffect->nextEffect = effect;
}

/*
==================
R_RemoveWorldEffect

Fully remove a world effect from the specified
world effect system and free all associated
memory.
==================
*/

void R_RemoveWorldEffect(worldEffectSystem_t *weSystem, worldEffect_t *effect)
{
    worldEffect_t   *currentEffect;

    // Update list indexes.
    if(weSystem->worldEffectList == effect){
        // The current effect is the current list base.
        if(effect->nextEffect != NULL){
            weSystem->worldEffectList = effect->nextEffect;
        }else{
            // The single remaining effect in the list is removed.
            // The list is now empty.
            weSystem->worldEffectList = NULL;
        }
    }else{
        // Iterate through the world effect list.
        currentEffect = weSystem->worldEffectList;

        while(currentEffect != NULL){
            // Check if the next effect in the list is the effect
            // that is removed.
            if(currentEffect->nextEffect == effect){
                if(effect->nextEffect != NULL){
                    currentEffect->nextEffect = effect->nextEffect;
                }else{
                    // Last effect in the list is removed.
                    currentEffect->nextEffect = NULL;
                }

                break;
            }

            currentEffect = currentEffect->nextEffect;
        }
    }

    // Free the world effect.
    ri.Free(effect);
}

/*
==================
R_UpdateWorldEffects

Update any world effect currently
present in the specified world
effect system.
==================
*/

void R_UpdateWorldEffects(worldEffectSystem_t *weSystem, float elapsedTime)
{
    worldEffect_t   *current;

    current = weSystem->worldEffectList;
    while(current != NULL){
        if(current->Update != NULL){
            current->Update(weSystem, current, elapsedTime);
        }

        current = current->nextEffect;
    }
}

/*
==================
R_RenderWorldEffects

Renders any world effect currently
present in the specified world
effect system.
==================
*/

void R_RenderWorldEffects(worldEffectSystem_t *weSystem)
{
    worldEffect_t   *current;

    current = weSystem->worldEffectList;
    while(current != NULL){
        if(current->Render != NULL){
            current->Render(weSystem, current);
        }

        current = current->nextEffect;
    }
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

    // Free all effects.
    while(weSystem->worldEffectList != NULL){
        R_RemoveWorldEffect(weSystem, weSystem->worldEffectList);
    }

    // Free particle list.
    ri.Free(weSystem->particleList);

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

    if(Q_stricmp(systemName, "snow") == 0){
        R_SnowSystemCommand(weSystem, command);
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
