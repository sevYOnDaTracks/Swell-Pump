#pragma once
#include <array>
#include <cmath>
// Gain curves over one cycle: 0 = full duck at the beat, 1 = untouched.
// hold = time spent fully ducked, rise = recovery length, shape picks the recovery law.
struct PumpCurve { const char* name; float hold, rise; int shape; float power; };
inline constexpr int curveCount=8;
inline constexpr std::array<PumpCurve,curveCount> curves={{
{"Classic",0.f,.50f,0,2.f},{"Smooth",0.f,.85f,1,1.f},{"Punch",.02f,.30f,0,3.f},{"Long",.04f,.95f,0,1.6f},
{"Hard",.12f,.22f,1,1.f},{"Breath",0.f,1.f,2,2.f},{"Lift",0.f,.75f,2,3.f},{"Gate",.45f,.05f,3,1.f}}};
inline float curveGain(int index,float t){
 const auto& c=curves[(size_t)(index<0?0:index>=curveCount?curveCount-1:index)];
 if(t<c.hold)return 0;float x=(t-c.hold)/c.rise;if(x>=1)return 1;
 switch(c.shape){case 0:return 1-std::pow(1-x,c.power);case 1:return .5f-.5f*std::cos(3.14159265f*x);case 2:return std::pow(x,c.power);default:return x;}
}
inline constexpr std::array<const char*,5> rateNames={"1/1","1/2","1/4","1/8","1/16"};
inline constexpr std::array<float,5> rateBeats={4,2,1,.5f,.25f};
struct PumpPreset { const char* category; const char* name; int curve, rate, trigger; float mix; };
inline constexpr int presetCount=16;
inline constexpr std::array<PumpPreset,presetCount> presets={{
{"House","Classic Pump",0,2,0,100},{"House","Deep Groove",1,2,0,70},{"House","Disco Bounce",2,3,0,60},
{"EDM","Big Room Duck",4,2,0,100},{"EDM","Future Pump",3,2,0,100},{"EDM","Festival Breath",5,1,0,85},
{"Trap / Rap","808 Room",2,2,0,60},{"Trap / Rap","Half-Time",3,1,0,80},{"Trap / Rap","Subtle Glue",1,2,0,35},
{"Lo-fi","Tape Breath",5,2,0,55},{"Lo-fi","Lazy Sway",1,1,0,45},
{"Creative","Chopper 1/16",7,4,0,100},{"Creative","Stutter 1/8",7,3,0,90},{"Creative","Swell Rise",6,0,0,100},{"Creative","Ghost Pump",0,3,0,40},
{"Sidechain","Kick Trigger",2,2,1,100}}};
inline constexpr std::array<const char*,6> categories={"House","EDM","Trap / Rap","Lo-fi","Creative","Sidechain"};
