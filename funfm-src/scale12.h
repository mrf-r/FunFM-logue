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

#ifndef _SCALE12_H
#define _SCALE12_H

#include <math.h>
// #include "dusty_config.h"
#define ASSERT(...)

#ifdef __cplusplus
extern "C" {
#endif

#define OCTAVE_OFFSET 12

static inline float scale12EdoGetFreqHz(float semitone)
{
    static const float table2[25] = {
        8.175798915643707, 8.415368110219507, 8.661957218027252, 8.915771938225692,
        9.177023997418988, 9.445931326274362, 9.722718241315029, 10.007615632040041,
        10.300861153527183, 10.602699424679592, 10.913382232281373, 11.233168741032559,
        11.562325709738575, 11.90112771383447, 12.249857374429663, 12.60880559406423,
        12.978271799373287, 13.358564190862078, 13.75, 14.15290575384802, 14.567617547440307,
        14.994481324147293, 15.433853164253883, 15.886099581993749, 16.351597831287414
    };
    // octave contain offset!!!
    unsigned octave = (unsigned)((semitone + 12.0f * (float)OCTAVE_OFFSET) / 12.0f);
    float mul = (float)(1 << octave) * (1.0f / (float)(1 << OCTAVE_OFFSET));

    float fpos = semitone - ((float)octave - (float)OCTAVE_OFFSET) * 12.0f;
    fpos = fpos * 2.0f; // table size - 24 / octave - 12
    unsigned pos = (unsigned)fpos;
    float spos = fpos - (float)pos;

    // interpolate freq
    float s1 = table2[pos];
    float s2 = table2[pos + 1];
    float freq = s1 + (s2 - s1) * spos;
    freq *= mul;
    return freq;
}

#ifdef __cplusplus
}
#endif

#endif // _SCALE12_H