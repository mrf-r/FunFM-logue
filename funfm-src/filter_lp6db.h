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
// #include <math.h>
// #ifndef M_PI
// #define M_PI 3.14159265358979323846
// #endif

static inline float fltLP6GetW(const float freq, const float sr_recp)
{
    float f_lim = freq < 1.f ? 1.f : freq;
    float ret = sr_recp * f_lim;
    if (ret > 0.48f)
        ret = 0.48f;
    ret = osc_tanpif(ret); // tanf(M_PI * ret)
    return ret;
}

static inline float fltLP6GetM(const float w)
{
    return (w - 1.f) / w;
}

static inline float fltLP6GetR(const float w)
{
    return w / (1.f + w);
}

static inline float fltLP6CalcSmpl(float* const state, const float in, const float m, const float r)
{
    float s = *state;
    float z = (in - s * m) * r;
    float out = z + s;
    *state = z;
    return out;
}

#endif // _FILTER_LP6DB_H