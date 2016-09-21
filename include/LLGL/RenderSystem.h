/*
 * RenderSystem.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_RENDER_SYSTEM_H__
#define __LLGL_RENDER_SYSTEM_H__


#include "Export.h"
#include "RenderContext.h"
#include "RenderSystemFlags.h"
#include "RenderingProfiler.h"
#include "RenderingDebugger.h"

#include "VertexBuffer.h"
#include "VertexFormat.h"
#include "IndexBuffer.h"
#include "IndexFormat.h"
#include "ConstantBuffer.h"
#include "StorageBuffer.h"
#include "Texture.h"
#include "RenderTarget.h"
#include "ShaderProgram.h"
#include "GraphicsPipeline.h"
#include "ComputePipeline.h"
#include "Sampler.h"
#include "Query.h"

#include <string>
#include <memory>
#include <vector>


namespace LLGL
{


/**
\brief Render system interface.
\remarks This is the main interface for the entire renderer.
It manages the ownership of all graphics objects and is used to create, modify, and delete all those objects.
The main functions for most graphics objects are "Create...", "Setup...", "Write...", and "Release":
\code
// Create an empty and unspecified vertex buffer
auto vertexBuffer = renderSystem->CreateVertexBuffer();

// Initialize object
renderSystem->SetupVertexBuffer(vertexBuffer, initialData, ...);

// Modify data
renderSystem->WriteVertexBuffer(vertexBuffer, modificationData, ...);

// Release object
renderSystem->Release(*vertexBuffer);
\endcode
*/
class LLGL_EXPORT RenderSystem
{

    public:

        //! Render system configuration structure.
        struct Configuration
        {
            /**
            \brief Specifies the default color for an uninitialized textures. The default value is white (255, 255, 255, 255).
            \remarks This will be used for each "SetupTexture..." function (not the "WriteTexture..." functions), when no initial image data is specified.
            */
            ColorRGBAub defaultImageColor;

            /**
            \brief Specifies whether the render system shall convert the data of texture images in software mode,
            when the renderering API can not automatically convert the image data into hardware image data.
            \remarks This is necessary for Direct3D render systems when the image format is mismatched to the hardware texture format.
            If this value is false, the render system will throw an exception in case of a format mismatch instead of converting the data.
            Such a format mismatch happens when the desired hardware texture format is 'TextureFormat::RGBA' and the image format is 'ImageFormat::RGB' for instance.
            \see ImageConverter
            \note Only relevant for: Direct3D 11, Direct3D 12.
            */
            bool        convertImageData    = true;
        };

        /* ----- Common ----- */

        RenderSystem(const RenderSystem&) = delete;
        RenderSystem& operator = (const RenderSystem&) = delete;

        virtual ~RenderSystem();

        /**
        \brief Returns the list of all available render system modules for the current platform
        (e.g. on Windows this might be { "OpenGL", "Direct3D12" }, but on MacOS it might be only { "OpenGL" }).
        */
        static std::vector<std::string> FindModules();

        /**
        \brief Loads a new render system from the specified module.
        \param[in] moduleName Specifies the name from which the new render system is to be loaded.
        This denotes a dynamic library (*.dll-files on Windows, *.so-files on Unix systems).
        If compiled in debug mode, the postfix "D" is appended to the module name.
        Moreover, the platform dependent file extension is always added automatically
        as well as the prefix "LLGL_", i.e. a module name "OpenGL" will be
        translated to "LLGL_OpenGLD.dll", if compiled on Windows in Debug mode.
        \param[in] profiler Optional pointer to a rendering profiler. If this is used, the counters of the profiler must be reset manually.
        This is only supported if LLGL was compiled with the "LLGL_ENABLE_DEBUG_LAYER" flag.
        \param[in] debugger Optional pointer to a rendering debugger.
        This is only supported if LLGL was compiled with the "LLGL_ENABLE_DEBUG_LAYER" flag.
        \remarks Usually the return type is a std::unique_ptr, but LLGL needs to keep track
        of the existance of this render system because only a single instance can be loaded at a time.
        So a std::weak_ptr is stored internally to check if it has been expired
        (see http://en.cppreference.com/w/cpp/memory/weak_ptr/expired),
        and this type can only refer to a std::shared_ptr.
        \throws std::runtime_error If loading the render system from the specified module failed.
        \throws std::runtime_error If there is already a loaded instance of a render system
        (make sure there are no more shared pointer references to the previous render system!)
        */
        static std::shared_ptr<RenderSystem> Load(
            const std::string& moduleName,
            RenderingProfiler* profiler = nullptr,
            RenderingDebugger* debugger = nullptr
        );

        //! Returns the name of this render system.
        inline const std::string& GetName() const
        {
            return name_;
        }

        //! Returns all available renderer information.
        virtual std::map<RendererInfo, std::string> QueryRendererInfo() const = 0;

        //! Returns the rendering capabilities.
        virtual RenderingCaps QueryRenderingCaps() const = 0;

        //! Returns the highest version of the supported shading language.
        virtual ShadingLanguage QueryShadingLanguage() const = 0;

        /* ----- Render Context ----- */

        /**
        \brief Creates a new render context and returns the raw pointer.
        \remarks The render system takes the ownership of this object. All render contexts are deleted in the destructor of this render system.
        */
        virtual RenderContext* CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window = nullptr) = 0;

        //! Releases the specified render context. This will all release all resources, that are associated with this render context.
        virtual void Release(RenderContext& renderContext) = 0;

        /**
        \brief Makes the specified render context to the current one.
        \param[in] renderContext Specifies the new current render context. If this is null, no render context is active.
        \return True on success, otherwise false.
        \remarks Never draw anything, while no render context is active!
        */
        bool MakeCurrent(RenderContext* renderContext);

        //! Returns the current render context. This may also be null.
        inline RenderContext* GetCurrentContext() const
        {
            return currentContext_;
        }

        /* ----- Hardware Buffers ------ */

        /**
        \brief Creates a new, empty, and unspecified vertex buffer.
        \see SetupVertexBuffer
        */
        virtual VertexBuffer* CreateVertexBuffer() = 0;

        /**
        \brief Creates a new, empty, and unspecified index buffer.
        \see SetupIndexBuffer
        */
        virtual IndexBuffer* CreateIndexBuffer() = 0;

        /**
        \brief Creates a new, empty, and unspecified constant buffer (also called "Uniform Buffer Object").
        \see SetupConstantBuffer
        */
        virtual ConstantBuffer* CreateConstantBuffer() = 0;

        /**
        \brief Creates a new, empty, and unspecified storage buffer (also called "Read/Write Buffer").
        \see SetupStorageBuffer
        */
        virtual StorageBuffer* CreateStorageBuffer() = 0;

        virtual void Release(VertexBuffer& vertexBuffer) = 0;
        virtual void Release(IndexBuffer& indexBuffer) = 0;
        virtual void Release(ConstantBuffer& constantBuffer) = 0;
        virtual void Release(StorageBuffer& storageBuffer) = 0;

        /**
        \brief Initializes the specified vertex buffer.
        \param[in] vertexBuffer Specifies the vertex buffer which is to be initialized.
        \param[in] data Raw pointer to the data with which the vertex buffer is to be initialized.
        This may also be null, to only initialize the size of the buffer. In this case, the buffer must
        be initialized with the "WriteVertexBuffer" function before it is used for drawing operations.
        \param[in] dataSize Specifies the size (in bytes) of the buffer.
        \param[in] usage Specifies the buffer usage, which is typically "BufferUsage::Static" for a vertex buffer, since it is rarely changed.
        \param[in] vertexFormat Specifies the vertex format layout, which is required to tell the renderer how the vertex attributes are stored inside the vertex buffer.
        This must be the same vertex format which is used for the respective graphics pipeline shader program.
        \see VertexFormat
        \see WriteVertexBuffer
        \see ShaderProgram
        */
        virtual void SetupVertexBuffer(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const VertexFormat& vertexFormat) = 0;

        /**
        \brief Initializes the specified index buffer.
        \param[in] indexBuffer Specifies the index buffer which is to be initialized.
        \param[in] data Raw pointer to the data with which the index buffer is to be initialized.
        This may also be null, to only initialize the size of the buffer. In this case, the buffer must
        be initialized with the "WriteIndexBuffer" function before it is used for drawing operations.
        \param[in] dataSize Specifies the size (in bytes) of the buffer.
        \param[in] usage Specifies the buffer usage, which is typically "BufferUsage::Static" for an index buffer, since it is rarely changed.
        \param[in] indexFormat Specifies the index format layout, which is basically only the data type of each index.
        The only valid format types for an index buffer are: DataType::UByte, DataType::UShort, DataType::UInt.
        \see IndexFormat
        \see WriteIndexBuffer
        */
        virtual void SetupIndexBuffer(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const IndexFormat& indexFormat) = 0;
        
        /**
        \brief Initializes the specified constant buffer.
        \param[in] constantBuffer Specifies the constant buffer which is to be initialized.
        \param[in] data Raw pointer to the data with which the constant buffer is to be initialized.
        This may also be null, to only initialize the size of the buffer. In this case, the buffer must
        be initialized with the "WriteConstantBuffer" function before it is used for drawing operations.
        \param[in] dataSize Specifies the size (in bytes) of the buffer.
        \param[in] usage Specifies the buffer usage.
        \see WriteConstantBuffer
        */
        virtual void SetupConstantBuffer(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, const BufferUsage usage) = 0;
        
        /**
        \brief Initializes the specified storage buffer.
        \param[in] storageBuffer Specifies the storage buffer which is to be initialized.
        \param[in] data Raw pointer to the data with which the storage buffer is to be initialized.
        This may also be null, to only initialize the size of the buffer. In this case, the buffer must
        be initialized with the "WriteStorageBuffer" function before it is used for drawing operations.
        \param[in] dataSize Specifies the size (in bytes) of the buffer.
        \param[in] usage Specifies the buffer usage.
        \see WriteStorageBuffer
        */
        virtual void SetupStorageBuffer(StorageBuffer& storageBuffer, const void* data, std::size_t dataSize, const BufferUsage usage) = 0;

        /**
        \brief Updates the data of the specified vertex buffer.
        \param[in] vertexBuffer Specifies the vertex buffer whose data is to be updated.
        \param[in] data Raw pointer to the data with which the vertex buffer is to be updated. This must not be null!
        \param[in] dataSize Specifies the size (in bytes) of the data block which is to be updated.
        This must be less then or equal to the size of the vertex buffer.
        \param[in] offset Specifies the offset (in bytes) at which the vertex buffer is to be updated.
        This offset plus the data block size (i.e. 'offset + dataSize') must be less than or equal to the size of the vertex buffer.
        */
        virtual void WriteVertexBuffer(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, std::size_t offset) = 0;
        
        //! \see WriteVertexBuffer
        virtual void WriteIndexBuffer(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, std::size_t offset) = 0;
        
        //! \see WriteVertexBuffer
        virtual void WriteConstantBuffer(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, std::size_t offset) = 0;
        
        //! \see WriteVertexBuffer
        virtual void WriteStorageBuffer(StorageBuffer& storageBuffer, const void* data, std::size_t dataSize, std::size_t offset) = 0;

        /* ----- Textures ----- */

        /**
        \brief Creates a new, empty, and unspecified texture.
        \remarks The type and dimension size of the this texture will be determined by any of the "SetupTexture..." functions.
        */
        virtual Texture* CreateTexture() = 0;

        virtual void Release(Texture& texture) = 0;

        /**
        \brief Queries a descriptor of the specified texture.
        \remarks This can be used to query the type and dimension size of the texture.
        \see TextureDescriptor
        */
        virtual TextureDescriptor QueryTextureDescriptor(const Texture& texture) = 0;

        /**
        \brief Initializes the specified texture as a 1-dimensional texture.
        \param[in] texture Specifies the texture which is to be initialized.
        \param[in] format Specifies the hardware texture format.
        \param[in] size Specifies the size of the texture (in texels, 'texture elements').
        \param[in] imageDesc Optional pointer to the image data descriptor.
        If this is null, the texture will be initialized with the currently configured default image color (see "Configuration::defaultTextureImageColor").
        If this is non-null, is is used to initialize the texture data.
        \see WriteTexture1D
        \see Configuration::defaultTextureImageColor
        */
        virtual void SetupTexture1D(Texture& texture, const TextureFormat format, int size, const ImageDataDescriptor* imageDesc = nullptr) = 0;

        /**
        \brief Initializes the specified texture as a 2-dimensional texture.
        \see SetupTexture1D
        \see WriteTexture2D
        */
        virtual void SetupTexture2D(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, const ImageDataDescriptor* imageDesc = nullptr) = 0;

        /**
        \brief Initializes the specified texture as a 3-dimensional texture.
        \see SetupTexture1D
        \see WriteTexture3D
        */
        virtual void SetupTexture3D(Texture& texture, const TextureFormat format, const Gs::Vector3i& size, const ImageDataDescriptor* imageDesc = nullptr) = 0;

        /**
        \brief Initializes the specified texture as a cube texture with six faces.
        \remarks If the image data descriptor is used, the image data must be large anough
        to store the image data of all six cube faces (i.e. width * height * 6 texels). The order of the cube faces is:
        AxisDirection::XPos, AxisDirection::XNeg, AxisDirection::YPos, AxisDirection::YNeg, AxisDirection::ZPos, AxisDirection::ZNeg.
        \see SetupTexture1D
        \see WriteTextureCube
        \see AxisDirection
        */
        virtual void SetupTextureCube(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, const ImageDataDescriptor* imageDesc = nullptr) = 0;
        
        /**
        \brief Initializes the specified texture as a 1-dimensional array texture.
        \param[in] layers Specifies the number of array layers.
        \see SetupTexture1D
        \see WriteTexture1DArray
        */
        virtual void SetupTexture1DArray(Texture& texture, const TextureFormat format, int size, unsigned int layers, const ImageDataDescriptor* imageDesc = nullptr) = 0;
        
        /**
        \brief Initializes the specified texture as a 2-dimensional array texture.
        \see SetupTexture1DArray
        \see WriteTexture2DArray
        */
        virtual void SetupTexture2DArray(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, unsigned int layers, const ImageDataDescriptor* imageDesc = nullptr) = 0;
        
        /**
        \brief Initializes the specified texture as a cube array texture with six faces for each layer.
        \see SetupTexture1DArray
        \see WriteTextureCubeArray
        */
        virtual void SetupTextureCubeArray(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, unsigned int layers, const ImageDataDescriptor* imageDesc = nullptr) = 0;
        
        /**
        \brief Updates the data of the specified texture.
        \param[in] texture Specifies the texture whose data is to be updated.
        \param[in] mipLevel Specifies the zero-based MIP ("Multum in Parvo") level which is to be updated.
        \param[in] position Specifies the position offset of the portion which is to be updated.
        \param[in] size Specifies the size of the portion which is to be updated.
        \param[in] imageDesc Specifies the image data descriptor. Its "data" member must not be null!
        \remarks This texture must be initialized as a 1-dimensional texture.
        */
        virtual void WriteTexture1D(Texture& texture, int mipLevel, int position, int size, const ImageDataDescriptor& imageDesc) = 0;
        
        /**
        \see WriteTexture1D
        \remarks This texture must be initialized as a 2-dimensional texture.
        */
        virtual void WriteTexture2D(Texture& texture, int mipLevel, const Gs::Vector2i& position, const Gs::Vector2i& size, const ImageDataDescriptor& imageDesc) = 0;
        
        /**
        \see WriteTexture1D
        \remarks This texture must be initialized as a 3-dimensional texture.
        */
        virtual void WriteTexture3D(Texture& texture, int mipLevel, const Gs::Vector3i& position, const Gs::Vector3i& size, const ImageDataDescriptor& imageDesc) = 0;
        
        /**
        \param[in] cubeFace Specifies the cube face which is to be updated.
        \see WriteTexture1D
        \remarks This texture must be initialized as a cube texture.
        */
        virtual void WriteTextureCube(
            Texture& texture, int mipLevel, const Gs::Vector2i& position, const AxisDirection cubeFace,
            const Gs::Vector2i& size, const ImageDataDescriptor& imageDesc
        ) = 0;
        
        /**
        \param[in] layerOffset Specifies the zero-based layer offset of the portion which is to be updated.
        \param[in] layers Specifies the number of layers to update.
        \see WriteTexture1D
        \remarks This texture must be initialized as a 1-dimensional array texture.
        */
        virtual void WriteTexture1DArray(
            Texture& texture, int mipLevel, int position, unsigned int layerOffset,
            int size, unsigned int layers, const ImageDataDescriptor& imageDesc
        ) = 0;
        
        /**
        \see WriteTexture1DArray
        \remarks This texture must be initialized as a 2-dimensional array texture.
        */
        virtual void WriteTexture2DArray(
            Texture& texture, int mipLevel, const Gs::Vector2i& position, unsigned int layerOffset,
            const Gs::Vector2i& size, unsigned int layers, const ImageDataDescriptor& imageDesc
        ) = 0;

        /**
        \see WriteTexture1DArray
        \param[in] cubeFaceOffset Specifies the cube face offset of the portion which is to be updated.
        \param[in] cubeFaces Specifies the number of cube faces to update.
        This can be out of bounds of the six cube faces, i.e. it can exceed several layers.
        \remarks This texture must be initialized as a cube array texture.
        */
        virtual void WriteTextureCubeArray(
            Texture& texture, int mipLevel, const Gs::Vector2i& position, unsigned int layerOffset, const AxisDirection cubeFaceOffset,
            const Gs::Vector2i& size, unsigned int cubeFaces, const ImageDataDescriptor& imageDesc
        ) = 0;

        /**
        \brief Reads the image data from the specified texture.
        \param[in] texture Specifies the texture object to read from.
        \param[in] mipLevel Specifies the MIP-level from which to read the image data.
        \param[in] dataFormat Specifies the output data format.
        \param[in] dataType Specifies the output data type.
        \param[out] data Specifies the output image data. This must be a pointer to a memory block, which is large enough to fit all the image data.
        \remarks Depending on the data format, data type, and texture size, the output image container must be allocated with enough memory size.
        The "QueryTextureDescriptor" function can be used to determine the texture dimensions.
        \code
        std::vector<LLGL::ColorRGBAub> image(textureWidth*textureHeight);
        renderSystem->ReadTexture(texture, 0, LLGL::ImageFormat::RGBA, LLGL::DataType::UByte, image.data());
        \endcode
        \see QueryTextureDescriptor
        */
        virtual void ReadTexture(const Texture& texture, int mipLevel, ImageFormat dataFormat, DataType dataType, void* data) = 0;

        /* ----- Samplers ---- */

        /**
        \brief Creates a new Sampler object.
        \throws std::runtime_error If the renderer does not support Sampler objects (e.g. if OpenGL 3.1 or lower is used).
        \see RenderContext::QueryRenderingCaps
        */
        virtual Sampler* CreateSampler(const SamplerDescriptor& desc) = 0;

        //! Releases the specified Sampler object. After this call, the specified object must no longer be used.
        virtual void Release(Sampler& sampler) = 0;

        /* ----- Render Targets ----- */

        /**
        \brief Creates a new RenderTarget object with the specified number of samples.
        \throws std::runtime_error If the renderer does not support RenderTarget objects (e.g. if OpenGL 2.1 or lower is used).
        */
        virtual RenderTarget* CreateRenderTarget(unsigned int multiSamples = 0) = 0;

        //! Releases the specified RenderTarget object. After this call, the specified object must no longer be used.
        virtual void Release(RenderTarget& renderTarget) = 0;

        /* ----- Shader ----- */

        /**
        \brief Creates a new and empty shader.
        \param[in] type Specifies the type of the shader, i.e. if it is either a vertex or fragment shader or the like.
        \see Shader
        */
        virtual Shader* CreateShader(const ShaderType type) = 0;

        /**
        \brief Creates a new and empty shader program.
        \remarks At least one shader must be attached to a shader program to be used for a graphics or compute pipeline.
        \see ShaderProgram
        */
        virtual ShaderProgram* CreateShaderProgram() = 0;

        virtual void Release(Shader& shader) = 0;
        virtual void Release(ShaderProgram& shaderProgram) = 0;

        /* ----- Pipeline States ----- */

        /**
        \brief Creates a new and initialized graphics pipeline state object.
        \param[in] desc Specifies the graphics pipeline descriptor.
        This will describe the entire pipeline state, i.e. the blending-, rasterizer-, depth-, stencil- and shader states.
        The "shaderProgram" member of the descriptor must never be null!
        \see GraphicsPipelineDescriptor
        */
        virtual GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc) = 0;

        /**
        \brief Creates a new and initialized compute pipeline state object.
        \param[in] desc Specifies the compute pipeline descriptor. This will describe the shader states.
        The "shaderProgram" member of the descriptor must never be null!
        \see ComputePipelineDescriptor
        */
        virtual ComputePipeline* CreateComputePipeline(const ComputePipelineDescriptor& desc) = 0;

        virtual void Release(GraphicsPipeline& graphicsPipeline) = 0;
        virtual void Release(ComputePipeline& computePipeline) = 0;

        /* ----- Queries ----- */

        //! Creates a new query of the specified type.
        virtual Query* CreateQuery(const QueryType type) = 0;

        virtual void Release(Query& query) = 0;

        /* === Members === */

        /**
        \brief Render system basic configuration.
        \remarks This can be used to change the behavior of default initializion of textures for instance.
        \see Configuration
        */
        Configuration config;

    protected:

        RenderSystem() = default;

        /**
        \brief Callback when a new render context is about to be made the current one.
        \remarks At this point, "GetCurrentContext" returns still the previous render context.
        */
        virtual bool OnMakeCurrent(RenderContext* renderContext);

        //! Creates an RGBA unsigned-byte image buffer for the specified number of pixels.
        std::vector<ColorRGBAub> GetDefaultTextureImageRGBAub(int numPixels) const;

    private:

        std::string     name_;

        RenderContext*  currentContext_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
