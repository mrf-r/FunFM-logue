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

#ifndef _OPERATOR_H
#define _OPERATOR_H

#include <stdint.h>
#include <math.h> // fabsf
#define ASSERT(...)

static inline float bwAmpComp(const float bw)
{
    // bw is 0..1, where 0-sine 1-saw
    const float a = bw + 1.f;
    return 4.f / (a * a);
}

// saw alias-supressed float
static inline float sawAsF(const float saw, const float thrs)
{
    float result = saw;
    if (saw > thrs) {
        float k = saw - thrs;
        k = k / (1.f - thrs);
        k = k * k;
        result -= k;
    } else if (saw < -thrs) {
        float k = saw + thrs;
        k = k / (1.f - thrs);
        k = k * k;
        result += k;
    }
    return result;
}

// i hate integer dsp, but i hate wrapping in floats even more)
static inline float operatorSmpl(
    int32_t* const acc, int32_t* const pm_prev, const float phasemod,
    const int32_t inc,
    const float speed, // unlimited
    const float bw, // 0..1
    const float amp_bw_comp // 4..1, depending on bw
)
{
    int32_t phase_old = *acc;
    int32_t phase = phase_old + inc;
    *acc = phase;
    // pm
    int32_t pm = (int32_t)(phasemod * 32768.f);
    int32_t pm_speed = pm - *pm_prev;
    *pm_prev = pm;
    pm_speed = (int16_t)pm;
    phase += pm * 65536;
    // speed
    float s = speed;
    s += (float)pm_speed * (1.f / 32768.f);
    s = fabsf(speed);
    float thrs = 1.f - s;
    if (s > 1.f) {
        s = 1.f;
        thrs = 0.f;
    }
    float amp = thrs * amp_bw_comp;
    thrs *= bw;
    float saw = (float)phase * (1.f / 32768.f / 65536.f);
    saw = sawAsF(saw, thrs);
    saw *= amp;
    return saw;
}

static inline float operatorDoubleSmpl(
    int32_t* const acc, // core accumulator
    int32_t* const pm_prev, // needs for artifacts suppression of audio in
    const float phasemod, // audio input. prone to aliasing on high gain
    const int32_t inc, // core increment
    const float speed, // same as inc, used for alias supression
    const float bw, // bandwidth 0..1 - same as speed, but for fun)
    const int32_t phasediff, // -P-W-M-
    const float amp_1, // bw compensation and pwm mixing
    const float amp_2 // from 4..1 to 2..0.5 when mixed
)
{
    // core phase calculation
    int32_t phase_old = *acc;
    int32_t phase = phase_old + inc;
    *acc = phase;
    // phase mod (with phase mod "speed")
    int32_t pm = (int32_t)(phasemod * 32768.f);
    int32_t pm_speed = pm - *pm_prev; // it's probably better to use unwrapped value?
    *pm_prev = pm;
    pm_speed = (int16_t)pm;
    phase += pm * 65536;
    // total "speed"
    float s = speed;
    s += (float)pm_speed * (1.f / 32768.f);
    s = fabsf(speed);
    float thrs = 1.f - s;
    if (s > 1.f) {
        s = 1.f;
        thrs = 0.f;
    }
    float amp = thrs; // anti-aliasing 0-amplification
    thrs *= bw;
    float saw1 = (float)phase * (1.f / 32768.f / 65536.f);
    saw1 = sawAsF(saw1, thrs) * amp_1;
    int32_t phase2 = phase + phasediff;
    float saw2 = (float)phase2 * (1.f / 32768.f / 65536.f);
    saw2 = sawAsF(saw2, thrs) * amp_2;
    return amp * (saw1 + saw2);
}

#endif // _OPERATOR_H