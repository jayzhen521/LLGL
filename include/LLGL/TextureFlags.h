/*
 * TextureFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_TEXTURE_FLAGS_H__
#define __LLGL_TEXTURE_FLAGS_H__


#include "Export.h"
#include <Gauss/Vector3.h>
#include <cstddef>


namespace LLGL
{


/* ----- Enumerations ----- */

//! Texture type enumeration.
enum class TextureType
{
    Undefined,          //!< Initial value of a Texture object.
    Texture1D,          //!< 1-Dimensional texture.
    Texture2D,          //!< 2-Dimensional texture.
    Texture3D,          //!< 3-Dimensional texture.
    TextureCube,        //!< Cube texture.
    Texture1DArray,     //!< 1-Dimensional array texture.
    Texture2DArray,     //!< 2-Dimensional array texture.
    TextureCubeArray,   //!< Cube array texture.
};

/**
\brief Hardware texture format enumeration.
\note All integral 32-bit formats are un-normalized!
*/
enum class TextureFormat
{
    Unknown,        //!< Unknown texture format.

    /* --- Base formats --- */
    DepthComponent, //!< Base format: depth component.
    DepthStencil,   //!< Base format: depth- and stencil components.
    R,              //!< Base format: red component.
    RG,             //!< Base format: red and green components.
    RGB,            //!< Base format: red, green, and blue components. \note Only supported with: OpenGL.
    RGBA,           //!< Base format: red, green, blue, and alpha components.

    /* --- Sized formats --- */
    R8,             //!< Sized format: red 8-bit normalized unsigned integer component.
    R8Sgn,          //!< Sized format: red 8-bit normalized signed integer component.

    R16,            //!< Sized format: red 16-bit normalized unsigned interger component.
    R16Sgn,         //!< Sized format: red 16-bit normalized signed interger component.
    R16Float,       //!< Sized format: red 16-bit floating point component.

    R32UInt,        //!< Sized format: red 32-bit un-normalized unsigned interger component.
    R32SInt,        //!< Sized format: red 32-bit un-normalized signed interger component.
    R32Float,       //!< Sized format: red 32-bit floating point component.

    RG8,            //!< Sized format: red, green 8-bit normalized unsigned integer components.
    RG8Sgn,         //!< Sized format: red, green 8-bit normalized signed integer components.

    RG16,           //!< Sized format: red, green 16-bit normalized unsigned interger components.
    RG16Sgn,        //!< Sized format: red, green 16-bit normalized signed interger components.
    RG16Float,      //!< Sized format: red, green 16-bit floating point components.

    RG32UInt,       //!< Sized format: red, green 32-bit un-normalized unsigned interger components.
    RG32SInt,       //!< Sized format: red, green 32-bit un-normalized signed interger components.
    RG32Float,      //!< Sized format: red, green 32-bit floating point components.

    RGB8,           //!< Sized format: red, green, blue 8-bit normalized unsigned integer components. \note Only supported with: OpenGL.
    RGB8Sgn,        //!< Sized format: red, green, blue 8-bit normalized signed integer components. \note Only supported with: OpenGL.

    RGB16,          //!< Sized format: red, green, blue 16-bit normalized unsigned interger components. \note Only supported with: OpenGL.
    RGB16Sgn,       //!< Sized format: red, green, blue 16-bit normalized signed interger components. \note Only supported with: OpenGL.
    RGB16Float,     //!< Sized format: red, green, blue 16-bit floating point components. \note Only supported with: OpenGL.

    RGB32UInt,      //!< Sized format: red, green, blue 32-bit un-normalized unsigned interger components.
    RGB32SInt,      //!< Sized format: red, green, blue 32-bit un-normalized signed interger components.
    RGB32Float,     //!< Sized format: red, green, blue 32-bit floating point components.

    RGBA8,          //!< Sized format: red, green, blue, alpha 8-bit normalized unsigned integer components.
    RGBA8Sgn,       //!< Sized format: red, green, blue, alpha 8-bit normalized signed integer components.

    RGBA16,         //!< Sized format: red, green, blue, alpha 16-bit normalized unsigned interger components.
    RGBA16Sgn,      //!< Sized format: red, green, blue, alpha 16-bit normalized signed interger components.
    RGBA16Float,    //!< Sized format: red, green, blue, alpha 16-bit floating point components.

    RGBA32UInt,     //!< Sized format: red, green, blue, alpha 32-bit un-normalized unsigned interger components.
    RGBA32SInt,     //!< Sized format: red, green, blue, alpha 32-bit un-normalized signed interger components.
    RGBA32Float,    //!< Sized format: red, green, blue, alpha 32-bit floating point components.

    /* --- Compressed formats --- */
    RGB_DXT1,       //!< Compressed format: RGB S3TC DXT1.
    RGBA_DXT1,      //!< Compressed format: RGBA S3TC DXT1.
    RGBA_DXT3,      //!< Compressed format: RGBA S3TC DXT3.
    RGBA_DXT5,      //!< Compressed format: RGBA S3TC DXT5.
};

//! Axis direction (also used for texture cube face).
enum class AxisDirection
{
    XPos = 0,   //!< X+ direction.
    XNeg,       //!< X- direction.
    YPos,       //!< Y+ direction.
    YNeg,       //!< Y- direction.
    ZPos,       //!< Z+ direction.
    ZNeg,       //!< Z- direction.
};


/* ----- Structures ----- */

/**
\brief Texture descriptor structure.
\remarks This is used to specifiy the dimensions of a texture which is to be created.
*/
struct TextureDescriptor
{
    TextureDescriptor()
    {
        type                    = TextureType::Undefined;
        format                  = TextureFormat::Unknown;
        texture3DDesc.width     = 0;
        texture3DDesc.height    = 0;
        texture3DDesc.depth     = 0;
    }
    ~TextureDescriptor()
    {
    }

    TextureType             type;   //!< Texture type.
    TextureFormat           format; //!< Texture hardware format.

    union
    {
        struct Texture1DDescriptor
        {
            int             width;  //!< Texture width.
            unsigned int    layers; //!< Number of texture array layers.
        }
        texture1DDesc;

        struct Texture2DDescriptor
        {
            int             width;  //!< Texture width.
            int             height; //!< Texture height.
            unsigned int    layers; //!< Number of texture array layers.
        }
        texture2DDesc;

        struct Texture3DDescriptor
        {
            int             width;  //!< Texture width.
            int             height; //!< Texture height.
            int             depth;  //!< Texture depth.
        }
        texture3DDesc;

        struct TextureCubeDescriptor
        {
            int             width;  //!< Texture width.
            int             height; //!< Texture height.
            unsigned int    layers; //!< Number of texture array layers (internally it will be a multiple of 6).
        }
        textureCubeDesc;
    };
};

/**
\brief Sub-texture descriptor structure.
\remarks This is used to write (or partially write) the image data of a texture MIP-map level.
*/
struct SubTextureDescriptor
{
    SubTextureDescriptor()
    {
        mipLevel                        = 0;
        textureCubeDesc.x               = 0;
        textureCubeDesc.y               = 0;
        textureCubeDesc.layerOffset     = 0;
        textureCubeDesc.width           = 0;
        textureCubeDesc.height          = 0;
        textureCubeDesc.cubeFaces       = 0;
        textureCubeDesc.cubeFaceOffset  = AxisDirection::XPos;
    }
    ~SubTextureDescriptor()
    {
    }

    int                     mipLevel;       //!< Zero-based MIP-map level for the sub-texture.

    union
    {
        struct Texture1DDescriptor
        {
            int             x;              //!< Sub-texture X-axis offset.
            unsigned int    layerOffset;    //!< Zero-based layer offset.
            int             width;          //!< Sub-texture width.
            unsigned int    layers;         //!< Number of texture array layers.
        }
        texture1DDesc;

        struct Texture2DDescriptor
        {
            int             x;              //!< Sub-texture X-axis offset.
            int             y;              //!< Sub-texture Y-axis offset.
            unsigned int    layerOffset;    //!< Zero-based layer offset.
            int             width;          //!< Sub-texture width.
            int             height;         //!< Sub-texture height.
            unsigned int    layers;         //!< Number of texture array layers.
        }
        texture2DDesc;

        struct Texture3DDescriptor
        {
            int             x;              //!< Sub-texture X-axis offset.
            int             y;              //!< Sub-texture Y-axis offset.
            int             z;              //!< Sub-texture Z-axis offset.
            int             width;          //!< Sub-texture width.
            int             height;         //!< Sub-texture height.
            int             depth;          //!< Number of texture array layers.
        }
        texture3DDesc;

        struct TextureCubeDescriptor
        {
            int             x;              //!< Sub-texture X-axis offset.
            int             y;              //!< Sub-texture Y-axis offset.
            unsigned int    layerOffset;    //!< Zero-based layer offset.
            int             width;          //!< Sub-texture width.
            int             height;         //!< Sub-texture height.
            unsigned int    cubeFaces;      //!< Number of cube-faces. To have all faces of N cube-texture layers, this value must be a N*6.
            AxisDirection   cubeFaceOffset; //!< First cube face in the current layer.
        }
        textureCubeDesc;
    };
};


/* ----- Functions ----- */

/**
\brief Returns the number of MIP-map levels for a texture with the specified size.
\return 1 + floor(log2(max{ x, y, z })).
*/
LLGL_EXPORT int NumMipLevels(const Gs::Vector3i& textureSize);

/**
\brief Returns true if the specified texture format is a compressed format,
i.e. either TextureFormat::RGB_DXT1, TextureFormat::RGBA_DXT1, TextureFormat::RGBA_DXT3, or TextureFormat::RGBA_DXT5.
\see TextureFormat
*/
LLGL_EXPORT bool IsCompressedFormat(const TextureFormat format);


} // /namespace LLGL


#endif



// ================================================================================
