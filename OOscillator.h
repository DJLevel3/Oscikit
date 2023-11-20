/*
 ==============================================================================
 
 This file is part of the iPlug 2 library. Copyright (C) the iPlug 2 developers. 
 
 See LICENSE.txt for  more info.
 
 ==============================================================================
*/

#pragma once

#include "IPlugPlatform.h"

BEGIN_IPLUG_NAMESPACE

class OOscillator
{
  public:
    OOscillator(double startPhase = 0., double startFreq = 1.)
        : mStartPhase(startPhase)
    {
        SetFreqCPS(startFreq);
    }

    sample Process(double freqHz)
    {
        SetFreqCPS(freqHz);
        mPhase = std::fmod(mPhase + mPhaseIncr, 1);
        return 0;
    }

    inline void SetFreqCPS(double freqHz) { mPhaseIncr = (1. / mSampleRate) * freqHz; }

    void SetSampleRate(double sampleRate) { mSampleRate = sampleRate; }

    void Reset() { mPhase = mStartPhase; }

    void SetPhase(double phase) { mPhase = phase; }

  protected:
    double mPhase = 0.;     // float phase (goes between 0. and 1.)
    double mPhaseIncr = 0.; // how much to add to the phase on each T
    double mSampleRate = 44100.;
    double mStartPhase;
};

template <typename T>
class OSinOscillator : public OOscillator
{
public:
  OSinOscillator(double startPhase = 0., double startFreq = 1.)
  : OOscillator(startPhase, startFreq)
  {
  }
  
  inline sample Process()
  {
    OOscillator::mPhase = OOscillator::mPhase + OOscillator::mPhaseIncr;
    return std::sin(OOscillator::mPhase * PI * 2.);
  }

  inline sample Process(double freqHz)
  {
    OOscillator::SetFreqCPS(freqHz);
    OOscillator::mPhase = OOscillator::mPhase + OOscillator::mPhaseIncr;
    return std::sin(OOscillator::mPhase * PI * 2.);
  }
};

END_IPLUG_NAMESPACE
