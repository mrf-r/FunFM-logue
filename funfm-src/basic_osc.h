// TEST WAVE
// Copyright (C) Eugene Chernyh man125403@gmail.com

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _BASIC_OSC_H
#define _BASIC_OSC_H

#include <stdint.h>

// low accuracy for testing only
static inline int32_t boscInc(const int32_t freq, const uint32_t sample_rate)
{
    return (0x100000000LL / sample_rate) * freq;
}

static inline int32_t boscSaw(int32_t* const acc, const int32_t inc)
{
    *acc += inc;
    return *acc;
}

static inline int32_t boscTriangle(const int32_t saw)
{
    int32_t tri = saw;
    if (tri < 0)
        tri = ~tri;
    tri = tri * 2;
    tri -= 0x80000000;
    return tri;
}

static inline int32_t boscParabolicSine(const int32_t tri)
{
    volatile int32_t ps = tri;
    if (ps < 0) {
        ps = ps + 0x80000000;
        ps = (ps / 65536) * (ps / 32768);
        ps = ps - 0x80000000;
    } else {
        ps = 0x7FFFFFFF - ps;
        ps = (ps / 65536) * (ps / 32768);
        ps = 0x7FFFFFFF - ps;
    }
    return ps;
}

#endif // _BASIC_OSC_H