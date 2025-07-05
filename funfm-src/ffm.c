// FunFM oscillator port for KORG Logue platform
// Copyright (C) 2024 Eugene Chernyh man125403@gmail.com

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

#include "userosc.h"
#include "osc_api.h"
#include "operator.h"
#include "scale12.h"
#include "filter_lp6db.h"
#include "filter_ctrl.h"

#define RANGE_O1PITCH_HIGH 48.1f
#define RANGE_O1PITCH_LOW -47.4f
#define RANGE_O2PITCH_HIGH (39.3f)
#define RANGE_O2PITCH_LOW (-52.7f)
#define RANGE_BW 1.f
#define RANGE_O1PM 5.f
#define RANGE_O2PM 5.f
#define RANGE_MIX 1.f
#define RANGE_FILTER 125.f

typedef enum
{
    CONTROL_FILTER = 0,
    CONTROL_OP1PITCH,
    CONTROL_OP1BW,
    CONTROL_OP1PM,
    CONTROL_OP2PITCH,
    CONTROL_OP2BW,
    CONTROL_OP2PM,
    CONTROL_MIX,
    CONTROLS_TOTAL
} ControlsEn;

// instance
static int32_t in_o1acc;
static int32_t in_o2acc;
static int32_t in_o1pm_prev;
static int32_t in_o2pm_prev;
static float in_feedback;
static Filter6db_state filter_state;
static float control[CONTROLS_TOTAL];
static float control_flt_state[CONTROLS_TOTAL];

void OSC_INIT(uint32_t platform, uint32_t api)
{
    (void)platform;
    (void)api;
    // FPU->FPDSCR |= FPU_FPDSCR_FZ_Msk;
    // FPU->FPDSCR |= FPU_FPDSCR_DN_Msk;
    // FPU->FPDSCR &= ~(1UL << 12);
    // FPU->FPDSCR &= ~(1UL << 10);
    // FPU->FPDSCR &= ~(1UL << 8);
    fltLP6PInit(&filter_state);
    for (unsigned i = 0; i < CONTROLS_TOTAL; i++)
    {
        control_flt_state[i] = 0.f;
    }
}

// blocksize 64
void OSC_CYCLE(const user_osc_param_t *const params, int32_t *yn, const uint32_t frames)
{
    if (frames != BLOCKSIZE)
        return;
    float pitch = (float)(params->pitch) * (1.f / 256.f);
    float lfo = q31_to_f32(params->shape_lfo);
    float patch[CONTROLS_TOTAL];
    for (unsigned i = 0; i < CONTROLS_TOTAL; i++)
    {
        patch[i] = fltCtrl(&control_flt_state[i], control[i]);
    }
    float op1speed = scale12EdoGetFreqHz(pitch + patch[CONTROL_OP1PITCH]) * k_samplerate_recipf;
    float op2speed = scale12EdoGetFreqHz(pitch + patch[CONTROL_OP2PITCH]) * k_samplerate_recipf;
    float filter_f = scale12EdoGetFreqHz(patch[CONTROL_FILTER]);

    int32_t op1inc = (int32_t)(op1speed * 2147483648.f);
    int32_t op2inc = (int32_t)(op2speed * 2147483648.f);

    // feedback extension
    float op2pm = patch[CONTROL_OP2PM] * patch[CONTROL_OP2PM] * 4000.f / (filter_f + 3000.f);
    float op1pm = (patch[CONTROL_OP1PM] + RANGE_O1PM * lfo);
    op1pm = op1pm * op1pm;

    float op1bw;
    float op1amp_comp1;
    float op1amp_comp2;
    float op2bw;
    float op2amp_comp1;
    float op2amp_comp2;
    if (patch[CONTROL_OP1BW] < 0)
    {
        op1bw = -patch[CONTROL_OP1BW];
        op1amp_comp1 = bwAmpComp(op1bw) / 2;
        op1amp_comp2 = -op1amp_comp1;
    }
    else
    {
        op1bw = patch[CONTROL_OP1BW];
        op1amp_comp1 = bwAmpComp(op1bw);
        op1amp_comp2 = 0;
    }
    if (patch[CONTROL_OP2BW] < 0)
    {
        op2bw = -patch[CONTROL_OP2BW];
        op2amp_comp1 = bwAmpComp(op2bw) / 2;
        op2amp_comp2 = -op2amp_comp1;
    }
    else
    {
        op2bw = patch[CONTROL_OP2BW];
        op2amp_comp1 = bwAmpComp(op2bw);
        op2amp_comp2 = 0;
    }
    fltLP6Precalc(&filter_state, filter_f);

    float op1amp_mix = patch[CONTROL_MIX];
    float op2amp_mix = 1.f - op1amp_mix;

    float fb = in_feedback;

    q31_t *__restrict y = (q31_t *)yn;
    const q31_t *y_e = y + BLOCKSIZE;
    for (; y != y_e;)
    {
        // comparison on each sample is not required, but we still have cycles and let's keep this code simple.
        float o2pm = (patch[CONTROL_OP2PM] < 0 ? fabsf(fb) : fb) * op2pm;
        float o2 = operatorDoubleSmpl(&in_o2acc, &in_o2pm_prev, o2pm, op2inc, op2speed, op2bw, 0x80000000, op2amp_comp1, op2amp_comp2);
        float o1pm = (patch[CONTROL_OP1PM] < 0 ? fabsf(o2) : o2) * op1pm;
        float o1 = operatorDoubleSmpl(&in_o1acc, &in_o1pm_prev, o1pm, op1inc, op1speed, op1bw, 0x80000000, op1amp_comp1, op1amp_comp2);
        float out = o1 * op1amp_mix + o2 * op2amp_mix;
        float flt = fltLP6CalcSmpl(&filter_state, out);
        fb = flt;
        *(y++) = f32_to_q31(out);
    }

    in_feedback = fb;
}

void OSC_NOTEON(const user_osc_param_t *const params)
{
    (void)params;
}
void OSC_NOTEOFF(const user_osc_param_t *const params)
{
    (void)params;
}

void OSC_PARAM(uint16_t index, uint16_t value)
{
    switch (index)
    {
    case k_user_osc_param_shape:
        // o1pitch
        control[CONTROL_OP1PITCH] = (float)value * ((RANGE_O1PITCH_HIGH - RANGE_O1PITCH_LOW) / 1024.f) - RANGE_O1PITCH_HIGH;
        break;
    case k_user_osc_param_shiftshape:
        // o2pitch
        control[CONTROL_OP2PITCH] = (float)value * ((RANGE_O2PITCH_HIGH - RANGE_O2PITCH_LOW) / 1024.f) - RANGE_O2PITCH_HIGH;
        break;
    case k_user_osc_param_id1:
        // 1bw
        control[CONTROL_OP1BW] = (float)value * (RANGE_BW * 2.f / 200.f) - RANGE_BW;
        break;
    case k_user_osc_param_id2:
        // 1pm
        control[CONTROL_OP1PM] = (float)value * (RANGE_O1PM * 2.f / 200.f) - RANGE_O1PM;
        break;
    case k_user_osc_param_id3:
        // 2bw
        control[CONTROL_OP2BW] = (float)value * (RANGE_BW * 2.f / 200.f) - RANGE_BW;
        break;
    case k_user_osc_param_id4:
        // 2pm
        control[CONTROL_OP2PM] = (float)value * (RANGE_O2PM * 2.f / 200.f) - RANGE_O2PM;
        break;
    case k_user_osc_param_id5:
        // mix
        control[CONTROL_MIX] = (float)value * (RANGE_MIX / 200.f);
        break;
    case k_user_osc_param_id6:
        // feedback filter
        control[CONTROL_FILTER] = RANGE_FILTER - (float)value * (RANGE_FILTER / 200.f);
        break;
    default:
        break;
    }
    (void)value;
}
