# gmod-headtrack-oculus

The `headtrack_oculus.dll` was used in gmod for vr support but Rubat removed it from the build process a good while ago since it never worked.  
Rubat never removed the `-vr` option in Gmod so you can still add it but without the headtrack dll gmod will just crash.  

this project currently contains the bare stuff for gmod to start, and maybe in the future it will reach a point where it will implement full vr support.  

## How to Install
1. Build it
2. Put the `headtrack_oculus.dll` into the `bin/` directory of gmod.  
3. Start gmod with the `-vr` startup option.  