/*
 ==============================================================================

 This file is part of the iPlug 2 library. Copyright (C) the iPlug 2 developers.

 See LICENSE.txt for  more info.

 ==============================================================================
*/

#pragma once

#include "IPlugPlatform.h"
#include "OOscillator.h"

BEGIN_IPLUG_NAMESPACE

class OsciSawOscillator : public OOscillator
{
  public:
    OsciSawOscillator(double startPhase = 0., double startFreq = 1.)
        : OOscillator(startPhase, startFreq)
    {
    }

    sample Process(double freqHz)
    {
        SetFreqCPS(freqHz);
        mPhase = std::fmod(mPhase + mPhaseIncr, 1);
        return calculate(mPhase);
    }

    sample Process()
    {
        mPhase = std::fmod(mPhase + mPhaseIncr, 1);
        return calculate(mPhase);
    }

    sample calculate(double phase) { return phase * 2 - 1; }
};

class OsciDrumOscillator : public OOscillator
{
  public:
    OsciDrumOscillator(double startPhase = 0., double startFreq = 1.)
        : OOscillator(startPhase, startFreq)
    {
    }

    sample Process(double freqHz)
    {
        SetFreqCPS(freqHz);
        mPhase = std::fmod(mPhase + mPhaseIncr, 1);
        return calculate(mPhase);
    }

    sample Process()
    {
        mPhase = std::fmod(mPhase + mPhaseIncr, 1);
        return calculate(mPhase);
    }

    sample calculate(double phase) { return std::sin(phase * PI * 2); }
};

END_IPLUG_NAMESPACE