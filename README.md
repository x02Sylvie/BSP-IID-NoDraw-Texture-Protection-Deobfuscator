# BSP-IID-NoDraw-Texture-Protection-Deobfuscator
Rough proof-of-concept of reversing one of source engine map obfuscation methods (currently unknown if works with CS:GO maps, tested with TF2 and CS:S era maps).

## Information
This application is meant as helper if you are attempting to decompile map which uses NoDraw IID obfsucation (every texture being nodraw or some other texture).

## How to use
1. Windows
2. Compile (C++14 standard)
3. Drag .bsp onto .exe
4. Deobfuscated map will be in newly created /finished directory

## Bugs
I didn't spend much time on this project which is why its output may lack accuracy but it was enough to work with for my purposes. You may need to fix up some things like triggers textures and such.

## Credits
https://developer.valvesoftware.com/wiki/Source_BSP_File_Format
