/*  FunFM oscillator port for KORG Logue platform
    Copyright (C) 2024 Eugene Chernyh man125403@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _FCTRL
#define _FCTRL

#include "osc_api.h"
// #define k_samplerate 48000.f

#define FCTL_TC_MS 88 // should be > 10

#define BLOCKSIZE 64
#define FCTRL_FC (1000.f / (float)FCTL_TC_MS) 
#define FCTRL_CONTROLRATE ((float)k_samplerate / (float)BLOCKSIZE)
#define FCTRLW (FCTRL_FC / FCTRL_CONTROLRATE)

static inline float fltCtrl(float* const state, const float in)
{
    float s = *state;
    float out = s + (in - s) * FCTRLW;
    *state = out;
    return out;
}

#if FCTL_TC_MS < 10
#error "too low for 64 block @ 48000 Hz"
#endif

#endif // _FCTRL