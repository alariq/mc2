# mc2
Mech Commander 2 Linux port.
Work in progress.
Currently game can be run on both Linux and Windows.
Can be compiled in 64-bit.

Fixed a lot of bugs (including ones present in original game).
Sound system is not fully implemented (panning, doppler, etc. not supported yet)

## TODO: 
* fix remaining memory leaks (finish implementation of memory heaps)
* make nice data packs, so not only me can play the game :-)
* actually finish all missions in the game
* make sure no files are created outside of user directory
* reduce draw calls number
* reimplement/optimize priority queue
* optimize geometry T&L (it is all on CPU now! at least move to SIMD, stretch goal (if somebody actually will be interested in a game) move all to GPU and do 2017 year graphics)
* I am sure there is more


Original game was released under Shared Source Limited Permission License (please refer to EULA.txt)
My code is licenced under GPL v.3 (see license.txt)
