![Plush 3D v1.2.0](/.github/plush.png)

A neat, portable, realtime 3D rendering library originally created between 1996 and 2000 by Justin Frankel and Nullsoft, Inc.

Original homepage: [1014.org](http://1014.org/code/nullsoft/plush/)

## Features

- Rasterization:
	- 8bpp only
	- Z-buffer or painters algorithm
	- Solid, Environment, Textured, Perspective Textured, Perspective Environment, Textured Environment, Translucent fills
	- None, Flat, Gouraud, Distance lightshading
- Unlimited number of cameras
	- Pitch, Pan, and Roll control
	- Target tracking
- Unlimited number of point and directional lights, each with own intensity
	- Point lights with distance falloff
- Hierarchical Objects
	- .3DS Mesh Reader
	- .COB Mesh Reader
	- .JAW Mesh Reader
- Textures
	- PCX texture reader with palette optimization and auto-rescaling
	- Perspective Correct modes have piecewise linear approximation every n pixels.
- Spline interpolation with tension, continuity, and bias control
- 4x4 Matrix manipulation library
- Easy to use, cross-platform API
- Architecture that makes it simple to add new rasterizers

## License

Copyright (C) 1996-1998, Justin Frankel

Copyright (C) 1998-2000, Nullsoft, Inc.

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

Justin Frankel
justin@nullsoft.com
