<div id="header">
    <p align="center">
      <b>audio</b><br>
  	  <span font-size="16px">a cross platform game audio library</span><br>
      <span font-size="12px">Made by <a href="http://tek256.com">Devon</a> with love.</span><br><br>
      <span><img src="https://github.com/tek256/audio/workflows/Build%20Audio/badge.svg"></span>
    </p>
</div>

### About
audio is a cross platform audio engine aimed at showing the approachability of rolling your own audio system. None of the concepts in this library are new or too complex that new programmers can't effectively understand them!

### Features
```
- Layering / Mixing
- Effects (Reverb, Chorus, Distortion, etc)
- Filters (High, Low, Bandpass)
- SFX Playing
- OGG Vorbis streaming/decoding
```

### Target Platforms
```
- Windows
- Linux
- Mac 
```

### Building
Make sure you have [OpenAL-Soft](https://github.com/kcat/openal-soft) and [CMake](https://cmake.org) installed before trying to build!

In order to build this from source you should only have to run these two commands:
```
cmake -Bbuild -S.
cmake --build build
```

### Examples 
There are a few examples in the repository, if you'd like to request one you're welcome to open an issue with a [feature request](https://github.com/tek256/audio/issues/new?assignees=&labels=&template=feature_request.md&title=)!
```
basic - simple usage of sfx
effects - how to use effects
eq - how to use the EQ effect
keyframes - using keyframes to change values over time
layers - how to use layers
music - how to use OGG vorbis song files
stress - a stress test
```
