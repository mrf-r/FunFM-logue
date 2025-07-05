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

#ifndef _FILTER_LP6DB_H
#define _FILTER_LP6DB_H

#include "osc_api.h"
// #define k_samplerate 48000
// #define k_samplerate_recipf (1.f/48000.f)

typedef struct
{
    float a;
    float b;
    float zi;
    float state;
} Filter6db_state;

static inline void fltLP6Precalc(Filter6db_state *const state, float freq)
{
    if (freq > (float)(k_samplerate / 2))
    {
        freq = (float)(k_samplerate / 2);
    }
    float c = k_samplerate_recipf * freq / 2.f;
    state->a = (1 - c) / (1 + c);
    state->b = c / (1 + c);
}

static inline void fltLP6PInit(Filter6db_state *const state) {
    state->state = 0.f;
    state->zi = 0.f;
    fltLP6Precalc(state, k_samplerate / 2);
}

static inline float fltLP6CalcSmpl(Filter6db_state *const state, const float in)
{
    float s = state->a * state->state + state->b * (in + state->zi);
    state->zi = in;
    state->state = s;
    return s;
}

#endif // _FILTER_LP6DB_H