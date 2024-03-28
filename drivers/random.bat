wcc -ms -s -zdf -r -zu random.c
wasm rndwrap.asm
wlink SYSTEM DOS FILE rndwrap.obj FILE random.obj NAME random OPTION MAP=RANDOM.MAP
