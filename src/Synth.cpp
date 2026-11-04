#include "sylenth2/Synth.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace sylenth2 {
namespace {
constexpr double pi = 3.1415926535897932384626433832795;
constexpr float piF = 3.14159265358979323846f;
float coeff(float seconds, double sr) noexcept {
    if (seconds <= 0.0001f) return 1.0f;
    return 1.0f - std::exp(-6.908f / (std::max(seconds, 0.0001f) * static_cast<float>(sr)));
}
float midiHz(int note) noexcept { return 440.0f * std::exp2((note - 69) / 12.0f); }
float fract(float x) noexcept { return x - std::floor(x); }
}

float Synth::clampf(float v, float lo, float hi) noexcept { return std::max(lo, std::min(v, hi)); }

void Synth::Envelope::configure(float attack, float decay, float sustain, float release, double sr) noexcept {
    a = coeff(attack, sr); d = coeff(decay, sr); r = coeff(release, sr);
    s = std::clamp(sustain, 0.0f, 1.0f);
}
void Synth::Envelope::on() noexcept { stage = EnvStage::Attack; level = 0.0f; }
void Synth::Envelope::off() noexcept { if (active()) stage = EnvStage::Release; }
float Synth::Envelope::tick() noexcept {
    switch (stage) {
        case EnvStage::Off: return 0.0f;
        case EnvStage::Attack:
            level += (1.0f - level) * a;
            if (level >= 0.999f) { level = 1.0f; stage = EnvStage::Decay; }
            break;
        case EnvStage::Decay:
            level += (s - level) * d;
            if (std::abs(level - s) < 0.0001f) { level = s; stage = EnvStage::Sustain; }
            break;
        case EnvStage::Sustain: level = s; break;
        case EnvStage::Release:
            level += (0.0f - level) * r;
            if (level < 0.00001f) { level = 0.0f; stage = EnvStage::Off; }
            break;
    }
    return level;
}

void Synth::Filter::coefficients(float hz, float resonance, double sr) noexcept {
    const float safeHz = std::clamp(hz, 24.0f, static_cast<float>(sr * 0.44));
    g = std::tan(piF * safeHz / static_cast<float>(sr));
    const float q = 0.707f + 12.0f * resonance * resonance;
    k = 1.0f / q;
    h = 1.0f / (1.0f + k * g + g * g);
}
float Synth::Filter::tick(float x) noexcept {
    const float v1 = (ic1 + g * (x - ic2)) * h;
    const float v2 = ic2 + g * v1;
    ic1 = 2.0f * v1 - ic1;
    ic2 = 2.0f * v2 - ic2;
    return v2;
}

void Synth::Bandpass::set(float freq, float q, double sr) noexcept {
    const float omega = 2.0f*piF*std::clamp(freq,30.0f,static_cast<float>(sr*.43))/static_cast<float>(sr);
    const float alpha = std::sin(omega)/(2.0f*q);
    const float a0 = 1.0f+alpha;
    b0 = alpha/a0; b1=0; b2=-alpha/a0;
    a1 = -2.0f*std::cos(omega)/a0; a2=(1.0f-alpha)/a0;
}
float Synth::Bandpass::tick(float x) noexcept {
    const float y = b0*x+z1;
    z1=b1*x-a1*y+z2;
    z2=b2*x-a2*y;
    return y;
}

float Synth::polyBlep(double t, double dt) noexcept {
    if (dt <= 0.0 || dt >= 1.0) return 0.0f;
    if (t < dt) { const double x=t/dt; return static_cast<float>(x+x-x*x-1.0); }
    if (t > 1.0-dt) { const double x=(t-1.0)/dt; return static_cast<float>(x*x+x+x+1.0); }
    return 0.0f;
}
float Synth::waveSample(Wave wave, double phase, double step, float pw) noexcept {
    switch (wave) {
        case Wave::Saw: return static_cast<float>(2.0*phase-1.0)-polyBlep(phase,step);
        case Wave::Pulse: {
            float v=phase < pw ? 1.0f:-1.0f;
            v += polyBlep(phase,step);
            double shifted = phase-pw;
            if (shifted < 0.0) shifted+=1.0;
            v -= polyBlep(shifted,step);
            return v;
        }
        case Wave::Triangle: return 1.0f-4.0f*std::abs(static_cast<float>(phase)-0.5f);
        case Wave::Sine: return static_cast<float>(std::sin(2.0*pi*phase));
    }
    return 0.0f;
}

Synth::Synth() { prepare(48000.0); }
void Synth::prepare(double sampleRate) {
    sr_ = std::clamp(sampleRate, 8000.0, 192000.0);
    delayL_.assign(static_cast<std::size_t>(sr_*2.0)+1,0.0f);
    delayR_.assign(delayL_.size(),0.0f);
    crystalL_.prepare(sr_); crystalR_.prepare(sr_);
    reset();
}
void Synth::reset() {
    voices_ = {};
    chorusL_.fill(0.0f); chorusR_.fill(0.0f);
    std::fill(delayL_.begin(),delayL_.end(),0.0f);
    std::fill(delayR_.begin(),delayR_.end(),0.0f);
    chorusPos_=0; chorusPhase_=0.0; delayPos_=0;
    pedal_=false; bend_=0.0f; age_=0; lastVowelKey_=-1.0f;
    beautyL_.reset(); beautyR_.reset(); beautyPhase_=0.0f; beautyCounter_=0;
    crystalL_.reset(); crystalR_.reset();
    for(auto& f:vowelL_)f.clear();
    for(auto& f:vowelR_)f.clear();
}
void Synth::setPatch(const Patch& patch) noexcept {
    patch_=patch;
    patch_.unisonA=std::clamp(patch_.unisonA,1,maxUnison);
    patch_.unisonB=std::clamp(patch_.unisonB,1,maxUnison);
    patch_.pulseWidth=clampf(patch_.pulseWidth,0.08f,0.92f);
    patch_.resonance=clampf(patch_.resonance,0.0f,1.0f);
    patch_.chorus=clampf(patch_.chorus,0.0f,1.0f);
    patch_.delay=clampf(patch_.delay,0.0f,1.0f);
    patch_.formant=clampf(patch_.formant,0.0f,1.0f);
    patch_.vowel=std::clamp(patch_.vowel,0,4);
    patch_.delayFeedback=clampf(patch_.delayFeedback,0.0f,0.85f);
    patch_.harmonicColor=clampf(patch_.harmonicColor,0.0f,1.0f);
    patch_.harmonicMix=clampf(patch_.harmonicMix,0.0f,1.0f);
    patch_.vocalBeauty=clampf(patch_.vocalBeauty,0.0f,1.0f);
    patch_.vocalMotion=clampf(patch_.vocalMotion,0.0f,1.0f);
    patch_.crystalClarity=clampf(patch_.crystalClarity,0.0f,1.0f);
    patch_.crystalResonance=clampf(patch_.crystalResonance,0.0f,1.0f);
    patch_.bodyDepth=clampf(patch_.bodyDepth,0.0f,1.0f);
    patch_.bodyDensity=clampf(patch_.bodyDensity,0.0f,1.0f);
    patch_.bodyDynamics=clampf(patch_.bodyDynamics,0.0f,1.0f);
    for(auto& v:voices_) if(v.amp.active()) tune(v);
}
void Synth::tune(Voice& v) noexcept {
    const float base = midiHz(v.note)*std::exp2(bend_/6.0f);
    const auto setup = [this,base](std::array<Lane,maxUnison>& lanes,int count,float spread,float semitones) {
        const float center=0.5f*(count-1);
        const float denom=std::max(center,1.0f);
        for(int i=0;i<count;++i) {
            const float t=(i-center)/denom;
            const float hz=base*std::exp2((semitones+t*spread/100.0f)/12.0f);
            lanes[i].step=std::clamp(static_cast<double>(hz)/sr_,0.0,0.48);
            const float pan=t*patch_.stereo*0.8f;
            lanes[i].panL=std::sqrt(0.5f*(1.0f-pan));
            lanes[i].panR=std::sqrt(0.5f*(1.0f+pan));
        }
    };
    setup(v.a,patch_.unisonA,patch_.detuneA,0.0f);
    setup(v.b,patch_.unisonB,patch_.detuneB,patch_.octaveB);
    v.harmonic.configure(base,sr_,patch_.harmonicColor);
    if (patch_.engineMode == EngineMode::CrystalBody) v.body.configure(base,sr_);
}
void Synth::noteOn(int midiNote,float velocity) noexcept {
    if(midiNote<0 || midiNote>127 || velocity<=0) { if(velocity<=0)noteOff(midiNote); return; }
    auto* dest=&voices_[0];
    for(auto& v:voices_) {
        if(!v.amp.active()) {dest=&v;break;}
        if(v.age<dest->age)dest=&v;
    }
    *dest=Voice{};
    dest->note=midiNote; dest->velocity=clampf(velocity,0,1); dest->age=++age_;
    dest->amp.configure(patch_.attack,patch_.decay,patch_.sustain,patch_.release,sr_);
    dest->filterEnv.configure(patch_.filterAttack,patch_.filterDecay,patch_.filterSustain,patch_.filterRelease,sr_);
    dest->amp.on(); dest->filterEnv.on();
    tune(*dest);
    for(int i=0;i<maxUnison;++i) {
        dest->a[i].phase=fract(0.0789f*midiNote+0.1731f*i);
        dest->b[i].phase=fract(0.1103f*midiNote+0.1297f*i+0.37f);
    }
    dest->harmonic.phase=dest->a[0].phase;
    // Previous voice was value-initialized; body starts at zero on every note.
    if (patch_.engineMode == EngineMode::CrystalBody) dest->body.reset();
}
void Synth::noteOff(int midiNote) noexcept {
    for(auto& v:voices_) if(v.note==midiNote && v.amp.active() && v.amp.stage!=EnvStage::Release) {
        if(pedal_)v.heldByPedal=true;
        else {v.amp.off();v.filterEnv.off();}
    }
}
void Synth::sustainPedal(bool down) noexcept {
    if(pedal_ && !down) for(auto& v:voices_) if(v.heldByPedal) {v.heldByPedal=false;v.amp.off();v.filterEnv.off();}
    pedal_=down;
}
void Synth::pitchBend(float normalized) noexcept {
    bend_=clampf(normalized,-1,1);
    for(auto& v:voices_)if(v.amp.active())tune(v);
}
void Synth::allNotesOff() noexcept {
    for(auto& v:voices_){v.amp.off();v.filterEnv.off();v.heldByPedal=false;}
    pedal_=false;
}
float Synth::chorusRead(const std::array<float,chorusSize>& buf,float delay) const noexcept {
    float pos=static_cast<float>(chorusPos_)-delay;
    while(pos<0)pos+=chorusSize;
    const int i=static_cast<int>(pos)%chorusSize;
    const int j=(i+1)%chorusSize;
    const float t=pos-static_cast<float>(static_cast<int>(pos));
    return buf[static_cast<std::size_t>(i)]*(1.0f-t)+buf[static_cast<std::size_t>(j)]*t;
}
void Synth::render(float* left,float* right,std::size_t frames) noexcept {
    if(!left || !right)return;
    constexpr float fScale=0.27f;
    const float drive=clampf(patch_.drive,0,1);
    const float chorusMix=patch_.chorus*0.38f;
    const float delayMix=patch_.delay*0.34f;
    const bool hasHarmonics = patch_.engineMode==EngineMode::Harmonic || patch_.engineMode==EngineMode::Fusion;
    const bool hasVocal = patch_.engineMode==EngineMode::Vocal || patch_.engineMode==EngineMode::Fusion;
    const bool hasBody = patch_.engineMode==EngineMode::CrystalBody;
    const bool hasCrystal = patch_.engineMode==EngineMode::CrystalClarity || hasBody;
    const std::size_t delaySamples=std::min(delayL_.size()-1,static_cast<std::size_t>(clampf(patch_.delaySeconds,0.05f,1.8f)*static_cast<float>(sr_)));
    const float vowelHz[5][2]={{760,1200},{530,1780},{310,2200},{570,920},{370,1000}};
    const float vowelKey=static_cast<float>(patch_.vowel);
    if(lastVowelKey_!=vowelKey) {
        for(int j=0;j<2;++j){vowelL_[j].set(vowelHz[patch_.vowel][j],j==0?3.2f:4.5f,sr_);
                              vowelR_[j].set(vowelHz[patch_.vowel][j],j==0?3.2f:4.5f,sr_);}
        lastVowelKey_=vowelKey;
    }
    for(std::size_t n=0;n<frames;++n) {
        float l=0.0f,r=0.0f;
        for(auto& v:voices_) {
            if(!v.amp.active())continue;
            const float amp=v.amp.tick();
            const float fenv=v.filterEnv.tick();
            if(!v.amp.active())continue;
            if((v.filterCounter++ & 15)==0){
                const float keyOctaves=patch_.keytrack*((v.note-60)/12.0f);
                const float cutoff=patch_.cutoffHz*std::exp2(patch_.filterEnvOctaves*fenv+keyOctaves);
                v.filterL.coefficients(cutoff,patch_.resonance,sr_);
                v.filterR.coefficients(cutoff,patch_.resonance,sr_);
            }
            float xL=0.0f,xR=0.0f;
            const auto add=[this,&xL,&xR](std::array<Lane,maxUnison>& lanes,int count,float level,Wave wave) {
                const float scale=level/std::sqrt(static_cast<float>(count));
                for(int i=0;i<count;++i){
                    auto& lane=lanes[i];
                    const float val=waveSample(wave,lane.phase,lane.step,patch_.pulseWidth)*scale;
                    xL+=val*lane.panL; xR+=val*lane.panR;
                    lane.phase+=lane.step;
                    if(lane.phase>=1.0)lane.phase-=1.0;
                }
            };
            add(v.a,patch_.unisonA,patch_.levelA,patch_.waveA);
            add(v.b,patch_.unisonB,patch_.levelB,patch_.waveB);
            if (hasHarmonics) {
                const float blend = patch_.harmonicMix;
                const float add = v.harmonic.tick(fenv) * 0.70710678f;
                xL = (1.0f-blend)*xL + blend*add;
                xR = (1.0f-blend)*xR + blend*add;
            }
            if(drive>0.0001f){
                const float amount=1.0f+4.0f*drive;
                const float norm=1.0f/std::tanh(amount);
                xL=std::tanh(xL*amount)*norm;
                xR=std::tanh(xR*amount)*norm;
            }
            float voiceL=v.filterL.tick(xL);
            float voiceR=v.filterR.tick(xR);
            if (hasBody) v.body.tick(voiceL,voiceR,patch_.bodyDepth,patch_.bodyDensity,patch_.bodyDynamics);
            l+=voiceL*amp*v.velocity*fScale;
            r+=voiceR*amp*v.velocity*fScale;
        }
        const float formant=patch_.formant;
        if(!hasVocal && formant>0.0001f){
            const float fl=vowelL_[0].tick(l)+0.80f*vowelL_[1].tick(l);
            const float fr=vowelR_[0].tick(r)+0.80f*vowelR_[1].tick(r);
            l=(1.0f-formant*0.5f)*l+formant*1.6f*fl;
            r=(1.0f-formant*0.5f)*r+formant*1.6f*fr;
        }
        if (hasVocal) {
            if ((beautyCounter_++ & 127u)==0u) {
                beautyL_.configure(patch_.vowel,sr_,patch_.vocalMotion,beautyPhase_);
                beautyR_.configure(patch_.vowel,sr_,patch_.vocalMotion,beautyPhase_);
            }
            const float wet = 0.76f*patch_.vocalBeauty;
            l = (1.0f-0.48f*wet)*l + 1.40f*wet*beautyL_.tick(l);
            r = (1.0f-0.48f*wet)*r + 1.40f*wet*beautyR_.tick(r);
            beautyPhase_ += static_cast<float>(2.0*pi*0.17/sr_);
            if (beautyPhase_ > 2.0f*piF) beautyPhase_ -= 2.0f*piF;
        }
        if (hasCrystal) {
            l=crystalL_.tick(l,patch_.crystalClarity,patch_.crystalResonance);
            r=crystalR_.tick(r,patch_.crystalClarity,patch_.crystalResonance);
        }
        chorusL_[static_cast<std::size_t>(chorusPos_)]=l;
        chorusR_[static_cast<std::size_t>(chorusPos_)]=r;
        if(chorusMix>0.0001f){
            const float d0=static_cast<float>(sr_)*(0.014f+0.003f*static_cast<float>(std::sin(chorusPhase_)));
            const float d1=static_cast<float>(sr_)*(0.018f+0.004f*static_cast<float>(std::sin(chorusPhase_*1.07+1.4)));
            const float cl=chorusRead(chorusL_,d0),cr=chorusRead(chorusR_,d1);
            l=l*(1.0f-chorusMix)+chorusMix*cl;
            r=r*(1.0f-chorusMix)+chorusMix*cr;
        }
        chorusPos_=(chorusPos_+1)%chorusSize;
        chorusPhase_+=2.0*pi*0.23/sr_;
        if(chorusPhase_>2.0*pi)chorusPhase_-=2.0*pi;
        if(!delayL_.empty()){
            const std::size_t rd=(delayPos_+delayL_.size()-delaySamples)%delayL_.size();
            const float dl=delayL_[rd],dr=delayR_[rd];
            delayL_[delayPos_]=l+dr*patch_.delayFeedback;
            delayR_[delayPos_]=r+dl*patch_.delayFeedback;
            delayPos_=(delayPos_+1)%delayL_.size();
            l+=delayMix*dl;r+=delayMix*dr;
        }
        left[n]=l*patch_.gain;
        right[n]=r*patch_.gain;
    }
}

const std::array<Patch,8>& factoryPatches() {
    static const auto list=[] {
        std::array<Patch,8> p{};
        p[0].name="01 Silver Halo";
        p[0].unisonA=7;p[0].unisonB=3;p[0].detuneA=17;p[0].detuneB=6;
        p[0].levelA=.78f;p[0].levelB=.22f;p[0].cutoffHz=11000;p[0].filterEnvOctaves=.35f;
        p[0].attack=.012f;p[0].decay=.7f;p[0].sustain=.8f;p[0].release=.8f;
        p[0].chorus=.25f;p[0].delay=.12f;

        p[1].name="02 Prism Pluck";p[1].waveB=Wave::Triangle;
        p[1].unisonA=3;p[1].unisonB=1;p[1].detuneA=8;p[1].levelB=.2f;
        p[1].cutoffHz=950;p[1].filterEnvOctaves=3.5f;p[1].filterDecay=.24f;
        p[1].attack=.002f;p[1].decay=.33f;p[1].sustain=.13f;p[1].release=.42f;
        p[1].chorus=.12f;p[1].delay=.28f;p[1].delaySeconds=.275f;

        p[2].name="03 Blue Current";p[2].waveA=Wave::Pulse;p[2].waveB=Wave::Saw;
        p[2].unisonA=2;p[2].unisonB=1;p[2].octaveB=-12;p[2].pulseWidth=.36f;
        p[2].detuneA=3;p[2].cutoffHz=470;p[2].resonance=.23f;p[2].filterEnvOctaves=2.3f;
        p[2].attack=.004f;p[2].decay=.23f;p[2].sustain=.72f;p[2].release=.19f;
        p[2].filterDecay=.19f;p[2].drive=.18f;p[2].chorus=.02f;p[2].gain=.73f;

        p[3].name="04 Midnight Vowel";p[3].waveA=Wave::Saw;p[3].waveB=Wave::Pulse;
        p[3].unisonA=2;p[3].unisonB=2;p[3].detuneA=5;p[3].detuneB=3;
        p[3].cutoffHz=5500;p[3].filterEnvOctaves=.5f;p[3].formant=.82f;p[3].vowel=3;
        p[3].attack=.022f;p[3].decay=.9f;p[3].sustain=.73f;p[3].release=.58f;
        p[3].chorus=.18f;p[3].delay=.08f;

        p[4].name="05 Afterimage";p[4].waveA=Wave::Triangle;p[4].waveB=Wave::Saw;
        p[4].unisonA=5;p[4].unisonB=4;p[4].detuneA=11;p[4].detuneB=17;
        p[4].levelA=.75f;p[4].levelB=.27f;p[4].octaveB=12;
        p[4].cutoffHz=3700;p[4].filterEnvOctaves=.6f;p[4].resonance=.04f;
        p[4].attack=.9f;p[4].decay=2.1f;p[4].sustain=.82f;p[4].release=2.9f;
        p[4].chorus=.6f;p[4].delay=.32f;p[4].delaySeconds=.48f;p[4].gain=.74f;

        p[5].name="06 Crystal Edge";p[5].waveA=Wave::Saw;p[5].waveB=Wave::Sine;
        p[5].unisonA=4;p[5].unisonB=1;p[5].detuneA=9;p[5].octaveB=12;
        p[5].levelA=.83f;p[5].levelB=.23f;p[5].cutoffHz=3900;
        p[5].filterEnvOctaves=2.0f;p[5].filterDecay=.42f;p[5].resonance=.18f;
        p[5].attack=.005f;p[5].decay=.51f;p[5].sustain=.55f;p[5].release=.44f;
        p[5].chorus=.10f;p[5].delay=.18f;

        p[6].name="07 Satin Pulse";p[6].waveA=Wave::Pulse;p[6].waveB=Wave::Triangle;
        p[6].unisonA=4;p[6].unisonB=2;p[6].detuneA=6;p[6].pulseWidth=.41f;
        p[6].levelA=.72f;p[6].levelB=.4f;p[6].cutoffHz=2000;p[6].filterEnvOctaves=1.1f;
        p[6].attack=.08f;p[6].decay=.6f;p[6].sustain=.77f;p[6].release=.96f;
        p[6].chorus=.29f;p[6].delay=.12f;

        p[7].name="08 Arctic Dawn";p[7].waveA=Wave::Sine;p[7].waveB=Wave::Saw;
        p[7].unisonA=3;p[7].unisonB=5;p[7].detuneA=2;p[7].detuneB=14;
        p[7].levelA=.75f;p[7].levelB=.22f;p[7].octaveB=12;
        p[7].cutoffHz=6500;p[7].resonance=.09f;p[7].filterEnvOctaves=.3f;
        p[7].attack=.27f;p[7].decay=.8f;p[7].sustain=.8f;p[7].release=1.75f;
        p[7].chorus=.38f;p[7].delay=.25f;p[7].delaySeconds=.40f;
        return p;
    }();
    return list;
}
} // namespace sylenth2
