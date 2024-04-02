/*
 * OpenGL.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_OPENGL_H
#define LLGL_OPENGL_H


#if defined LLGL_OPENGL
#   include "GLCoreProfile/OpenGLCore.h"
#elif defined LLGL_OPENGLES3
#   include "GLESProfile/OpenGLES.h"
#endif


#if defined GL_ARB_draw_indirect || defined GL_ES_VERSION_3_1
#   define LLGL_GLEXT_DRAW_INDIRECT
#endif

#if defined GL_ARB_draw_elements_base_vertex || defined GL_ES_VERSION_3_2
#   define LLGL_GLEXT_DRAW_ELEMENTS_BASE_VERTEX
#endif

#if defined GL_ARB_base_instance
#   define LLGL_GLEXT_BASE_INSTANCE
#endif

#if defined GL_ARB_multi_draw_indirect
#   define LLGL_GLEXT_MULTI_DRAW_INDIRECT
#endif

#if defined GL_ARB_compute_shader || defined GL_ES_VERSION_3_1
#   define LLGL_GLEXT_COMPUTE_SHADER
#endif

#if defined GL_KHR_debug || defined GL_ES_VERSION_3_2
#   define LLGL_GLEXT_DEBUG
#endif

//TODO: which extension?
#if defined LLGL_OPENGL
#   define LLGL_GLEXT_CONDITIONAL_RENDER
#endif

#if defined GL_ES_VERSION_3_0 || defined __APPLE__
#   define LLGL_GLEXT_TRANSFORM_FEEDBACK
#endif

#if defined GL_EXT_draw_buffers2 || defined GL_ES_VERSION_3_2
#   define LLGL_GLEXT_DRAW_BUFFERS2
#endif

#if defined GL_ARB_draw_buffers_blend || defined GL_ES_VERSION_3_2
#   define LLGL_GLEXT_DRAW_BUFFERS_BLEND
#endif

#if defined GL_ARB_tessellation_shader || defined GL_ES_VERSION_3_2
#   define LLGL_GLEXT_TESSELLATION_SHADER
#endif

#if defined GL_ARB_shader_storage_buffer_object
#   define LLGL_GLEXT_SHADER_STORAGE_BUFFER_OBJECT
#endif

#if defined LLGL_OPENGL || defined GL_ES_VERSION_3_1
#   define LLGL_GLEXT_GET_TEX_LEVEL_PARAMETER
#endif

//#if defined GL_ARB_clip_control
//#   define LLGL_GLEXT_CLIP_CONTROL
//#endif

#ifdef GL_TEXTURE_BORDER_COLOR
#   define LLGL_SAMPLER_BORDER_COLOR
#endif

// At most one of these should be defined to indicate which API
// we'll be using to implement fixed-index primitive restart.
#if defined GL_ES_VERSION_2_0 || defined GL_VERSION_4_3
#   define LLGL_PRIMITIVE_RESTART_FIXED_INDEX
#elif defined GL_VERSION_3_1
#   define LLGL_PRIMITIVE_RESTART
#endif


#endif



// ================================================================================
