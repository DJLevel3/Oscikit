#pragma once

#include "ADSREnvelope.h"
#include "OSynth.h"
#include <vector>

static constexpr int kNumDrums = 4;
static constexpr double drum0Freq = 25.;        // Hz
static constexpr double drum1Freq = 50.;        // Hz
static constexpr double drum2Freq = 75.;        // Hz
static constexpr double drum3Freq = 100.;       // Hz
static constexpr double kPitchEnvRange = 500.;  // Hz
static constexpr double kAmpDecayTime = 300;    // Ms
static constexpr double kPitchDecayTime = 100.; // Ms

using namespace iplug;
class DrumVoice
{
  protected:
    ADSREnvelope<sample> mPitchEnv{"pitch", nullptr, false};
    ADSREnvelope<sample> mAmpEnv{"amp", nullptr, false};

  public:
    virtual inline sample Process() = 0;

    virtual void Trigger(double amp, double sRate) = 0;

    inline bool IsActive() const { return mAmpEnv.GetBusy(); }
};

class SawDrum : public DrumVoice
{
  protected:
    OsciSawOscillator mOsc;
    double mBaseFreq;
    double mBasePhase;

  public:
    SawDrum(double baseFreq, double phase)
        : mBaseFreq(baseFreq)
        , mBasePhase(phase)
    {
    }

    inline sample Process() override { return mOsc.Process(mBaseFreq + mPitchEnv.Process()) * mAmpEnv.Process(); }

    void Trigger(double amp, double sRate) override
    {
        mAmpEnv.SetSampleRate(sRate);
        mAmpEnv.SetStageTime(ADSREnvelope<sample>::kAttack, 0.);
        mAmpEnv.SetStageTime(ADSREnvelope<sample>::kDecay, kAmpDecayTime);

        mPitchEnv.SetSampleRate(sRate);
        mPitchEnv.SetStageTime(ADSREnvelope<sample>::kAttack, 0.);
        mPitchEnv.SetStageTime(ADSREnvelope<sample>::kDecay, kPitchDecayTime);

        mOsc.SetSampleRate(sRate);
        mOsc.Reset();
        mOsc.SetPhase(mBasePhase);

        mPitchEnv.Start(amp * kPitchEnvRange);
        mAmpEnv.Start(amp);
    }
};

class SineDrum : public DrumVoice
{
  protected:
    OsciDrumOscillator mOsc;
    double mBaseFreq;
    double mBasePhase;

  public:
    SineDrum(double baseFreq, double phase)
        : mBaseFreq(baseFreq)
        , mBasePhase(phase)
    {
    }

    inline sample Process() override { return mOsc.Process(mBaseFreq + mPitchEnv.Process()) * mAmpEnv.Process(); }

    void Trigger(double amp, double sRate) override
    {
        mAmpEnv.SetSampleRate(sRate);
        mAmpEnv.SetStageTime(ADSREnvelope<sample>::kAttack, 0.);
        mAmpEnv.SetStageTime(ADSREnvelope<sample>::kDecay, kAmpDecayTime);

        mPitchEnv.SetSampleRate(sRate);
        mPitchEnv.SetStageTime(ADSREnvelope<sample>::kAttack, 0.);
        mPitchEnv.SetStageTime(ADSREnvelope<sample>::kDecay, kPitchDecayTime);

        mOsc.SetSampleRate(sRate);
        mOsc.Reset();
        mOsc.SetPhase(mBasePhase);
       
        mPitchEnv.Retrigger(amp * kPitchEnvRange);
        mAmpEnv.Retrigger(amp);
    }
};

class DrumSynthDSP
{
  public:

    DrumSynthDSP()
    //  : mMidiQueue(IMidiMsg::QueueSize(DEFAULT_BLOCK_SIZE, DEFAULT_SAMPLE_RATE))
    {
        std::vector<DrumVoice*> drum0;
        drum0.push_back(new SawDrum(drum0Freq, 0));
        drum0.push_back(new SawDrum(drum0Freq, 0.5));
        mDrums.push_back(drum0);

        std::vector<DrumVoice*> drum1;
        drum1.push_back(new SawDrum(drum1Freq, 0));
        drum1.push_back(new SawDrum(drum1Freq, 0.5));
        mDrums.push_back(drum1);

        std::vector<DrumVoice*> drum2;
        drum2.push_back(new SineDrum(drum0Freq, 0));
        drum2.push_back(new SineDrum(drum0Freq, 0.25));
        mDrums.push_back(drum2);

        std::vector<DrumVoice*> drum3;
        drum3.push_back(new SineDrum(drum1Freq, 0));
        drum3.push_back(new SineDrum(drum1Freq, 0.25));
        mDrums.push_back(drum3);

    }

    void Reset(double sampleRate, int blockSize)
    {
        mMidiQueue.Resize(blockSize);
        //    mMidiQueue.Resize(IMidiMsg::QueueSize(blockSize, sampleRate));
    }

    void ProcessBlock(sample** outputs, int nFrames, double sRate)
    {
        for (int s = 0; s < nFrames; s++)
        {
            while (!mMidiQueue.Empty())
            {
                IMidiMsg& msg = mMidiQueue.Peek();
                if (msg.mOffset > s)
                    break;

                if (msg.StatusMsg() == IMidiMsg::kNoteOn && msg.Velocity())
                {
                    int pitchClass = msg.NoteNumber() % 12;

                    if (pitchClass < kNumDrums)
                    {
                        (*mDrums[pitchClass][0]).Trigger(msg.Velocity() / 127.f, sRate);
                        (*mDrums[pitchClass][1]).Trigger(msg.Velocity() / 127.f, sRate);
                    }
                }

                mMidiQueue.Remove();
            }

            if (mMultiOut)
            {
                int channel = 0;
                for (int d = 0; d < kNumDrums; d++)
                {
                    outputs[channel + 0][s] = 0.;
                    outputs[channel + 1][s] = 0.;

                    if ((*mDrums[d][0]).IsActive())
                    {
                        outputs[channel + 0][s] = (*mDrums[d][0]).Process();
                        outputs[channel + 1][s] = (*mDrums[d][1]).Process();
                    }

                    channel += 2;
                }
            }
            else
            {
                outputs[0][s] = 0.;
                outputs[1][s] = 0.;

                for (int d = 0; d < kNumDrums; d++)
                {
                    if ((*mDrums[d][0]).IsActive())
                    {
                        outputs[0][s] += (*mDrums[d][0]).Process();
                        outputs[1][s] += (*mDrums[d][1]).Process();
                    }
                }
            }
        }
        mMidiQueue.Flush(nFrames);
    }

    void ProcessMidiMsg(const IMidiMsg& msg) { mMidiQueue.Add(msg); }

    void SetMultiOut(bool multiOut) { mMultiOut = multiOut; }

  private:
    bool mMultiOut = false;
    std::vector<std::vector<DrumVoice*>> mDrums;
    IMidiQueue mMidiQueue;
};
