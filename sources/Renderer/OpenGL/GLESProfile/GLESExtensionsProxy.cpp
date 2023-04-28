/*
 * GLESExtensionsProxy.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../OpenGL.h"
#include "../GLCore.h"
#include <stdexcept>
#include <string>



namespace LLGL
{


#define LLGL_DEF_GLES_PROXY_PROCS

// Include inline header for proxy function definitions
#include "GLESExtensionsDecl.inl"

#undef LLGL_DEF_GLES_PROXY_PROCS


} // /namespace LLGL



// ================================================================================
