# vocaloid
Vocaloid is an audio simple processing library in C/C++, follow W3C Web Audio API Specification.

## Web Audio API Specification
[Web Audio API Specification](https://webaudio.github.io/web-audio-api/)

## Requirement
- Windows Vista
- Visual Studio 2015
- FFmpeg shared version (Copy to '/build' directory)

## Feature
- Pure C++ 11
- Light, No other dependencies
- Audio Graph Processing
- Integrate FFmpeg in decoding and encoding (Support most format in windows, only .wav in other platforms)
- Easy in use, implement

## Components
- GainNode
- FileReaderNode
- FileWriterNode
- PlayerNode
- OscillatorNode
- BufferNode
- BiquadNode
- ConvolutionNode
- DelayNode
- DynamicCompressorNode
- WaveShaperNode
- PitchShifter

## Use

    #include <vocaloid/audio_context.hpp>
    #include <vocaloid/player_node.hpp>
    #include <vocaloid/file_reader_node.hpp>
    #include <iostream>
    using namespace vocaloid;
    using namespace vocaloid::node;

    void main(){
        auto context = new AudioContext();
        auto reader = new FileReaderNode(context);
        reader->SetPath("1.mp3");
        auto player = new PlayerNode(context);
        context->Connect(reader, player);
        context->Prepare();
        std::cout << "Preparing" << std::endl;
        context->Start();
        std::cout << "Start to process" << std::endl;
        getchar();
        std::cout << "End" << std::endl;
        context->Close();
    }

## License
This package is published under MIT license.