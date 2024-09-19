Project inspired by/based on [dxvk-nvapi](https://github.com/jp7677/dxvk-nvapi/)

By default logging is disabled. To enable logging put [fakenvapi.ini](fakenvapi.ini) next to the exe and edit the config

# Installation
For non frame generation setup use [OptiScaler](https://github.com/cdozdil/OptiScaler/blob/master/Spoofing.md#nvapi).
For frame generation use [DLSS Enabler](https://github.com/artur-graniszewski/DLSS-Enabler)

# Overview
Supports [LatencyFlex](https://github.com/ishitatsuyuki/LatencyFleX) as well as [AntiLag 2](https://github.com/GPUOpen-LibrariesAndSDKs/AntiLag2-SDK).
AntiLag 2 is automatically selected when available.

AL2 can't be used with native FSR FG as DLSSG-specific Reflex calls are required. LatencyFlex can be used in that case but it will require [forcing it](https://github.com/FakeMichau/fakenvapi/blob/master/fakenvapi.ini#L7C1-L7C18) using the ini file on AL2-supported systems.

Benefits of AntiLag 2:
 - Good reduction in latency
 - Overlay indicating that it's working - frame delay should be green
 
Downsides of AntiLag 2:
 - Limited to AMD RDNA1+ cards and Windows
 - Unreliable overlay readings. More accurate values are given by [FrameView](https://www.nvidia.com/en-us/geforce/technologies/frameview/) and looking at PCL. Fail safe way of confirming the latency is using [FLM](https://github.com/GPUOpen-Tools/frame_latency_meter/releases)
 - Currently can only be built using MSVC and not MinGW
&nbsp;

Benefits of LatencyFlex:
 - Crossplatform, crossvendor
 - Open source

Downsides of LatencyFlex:
 - Limited latency reduction compared to AL2
 - Low fps just after launching a game that stabilizes over time