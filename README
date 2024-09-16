Plush 3D v1.2.0 - Source distribution
- Updated 11/06/00

0. CONTENTS
  0.   CONTENTS
  I.   LICENSE
  II.  FEATURES
  III. FILES IN THIS ARCHIVE
  IV.  INSTALLATION
  V.   NOTES
  VI.  LIBRARY INFORMATION
  VII. DOCUMENTATION/MORE INFORMATION

I. LICENSE
  Copyright (C) 1996-1998, Justin Frankel
  Copyright (C) 1998-2000, Nullsoft, Inc.

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Justin Frankel
  justin@nullsoft.com

     
II. FEATURES

  Changes in 1.2.0:
    Static scene maximums (lights, triangles)
    Texture+Environment combining modes
    No more crashing!
    Slower rasterizers (see above)
    Other cleanups/fixes/optimizations.
    Thinking about making Plush++

  Some notable features of Plush 1.1.0 are:
    Totally portable ANSI C source, tested on:
      DOS32: Watcom C 10.5, DJGPP v2 (gcc 2.7.2.1)
      Win32: Visual C++ 4.0
      Unix variants: Linux, Solaris, SunOS, HP/UX, AIX 
                     (gcc, and native CC where available)
      MacOS: Codewarrior 9, 10
    Rasterization: 
      8bpp only
      Z-buffer or painters algorithm
      Solid, Environment, Textured, Perspective Textured, 
        Perspective Environment, Textured Environment, Translucent fills
      None, Flat, Gouraud, Distance lightshading
    Unlimited number of cameras
      Pitch, Pan, and Roll control
      Target tracking  
    Unlimited number of point and directional lights, each with own intensity
      Point lights with distance falloff
    Hierarchical Objects
      .3DS Mesh Reader
      .COB Mesh Reader
      .JAW Mesh Reader
    Textures
      PCX texture reader with palette optimization and auto-rescaling
      Perspective Correct modes have piecewise linear approximation every
        n pixels.
    Spline interpolation with tension, continuity, and bias control
    4x4 Matrix manipulation library
    Easy to use, cross-platform API
    Architecture that makes it simple to add new rasterizers

III. FILES IN THIS ARCHIVE
	Name                    Description
	----                    -----------
	CAM.C			Camera code
	CLIP.C			Frustum clipping code
	LIGHT.C			Light code
	MAKE.C			Object primitive code
	MAT.C			Material code
	MATH.C			Math functions
	OBJ.C			Object code
	PF_PTEX.C		Perspective correct rasterizers
	PF_SOLID.C		Solid rasterizers
	PF_TEX.C		Texture mapping rasterizers
	PF_TRANS.C		Translucent rasterizers
	PLUSH.C			Misc code
	READ_3DS.C		3DS reader
	READ_COB.C		COB reader
	READ_JAW.C		JAW reader
	READ_PCX.C		PCX reader
	RENDER.C		Render manager
	SPLINE.C		Spline interpolator
	TEXT.C			Bitmaped text
	PLUSH.H			Main header file
	PL_CONF.H		Configuration header file
	PL_DEFS.H		Additional defines/config
	PL_TYPES.H		Complex types definitions
	PUTFACE.H		Utility header for pf_*.c
	MAKEFILE.LSH		Linux makefile
	MAKEFILE.WC		Watcom C 10.5 Makefile
	MAKEFILE.DJ		DJGPP v2 Makefile
	LICENSE.TXT		Licensing info
	README.TXT		This file

IV. INSTALLATION/COMPILATION
  Look/modify the appropriate makefile, or create a project with all the 
.c files in it. pl_conf.h and pl_defs.h have some defines that should be
editted for your platform.
  
V. NOTES
  For some platforms (i.e. DOS), in your startup code you *must* disable 
floating point exceptions for some parts of Plush (i.e. perspective correction)
to work right. You can do this using

#include <float.h>
_control87(MCW_EM,MCW_EM);

VI. LIBRARY INFORMATION
  Well, this library is neither fast nor well documented, but it is a good 
example of decent api design and overall engine implementation. Hope it brings
joy to peoples' hearts.


VII. DOCUMENTATION/MORE INFORMATION
  At the time of this release, API documentation is minimal. I am currently 
working on elaborate HTML docs, which are about 10% done. The file PL_API.html
included with this archive contains it. There is also a decent amount
of documentation in plush.h and pl_*.h. Check back to 
http://nullsoft.com periodically to see if new documentation has
been released (update 9/14/00- this will never happen).
  Also, as you might have noticed, no examples have been included with this 
release. Please see http://nullsoft.com for examples.

<EOF>
