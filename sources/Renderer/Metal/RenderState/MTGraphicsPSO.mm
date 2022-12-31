/*
 * MTGraphicsPSO.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTGraphicsPSO.h"
#include "MTRenderPass.h"
#include "../Shader/MTShader.h"
#include "../MTEncoderScheduler.h"
#include "../MTTypes.h"
#include "../MTCore.h"
#include "../../CheckedCast.h"
#include "../../PipelineStateUtils.h"
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/Platform/Platform.h>


namespace LLGL
{


static void Convert(MTLStencilDescriptor* dst, const StencilFaceDescriptor& src)
{
    dst.stencilFailureOperation     = MTTypes::ToMTLStencilOperation(src.stencilFailOp);
    dst.depthFailureOperation       = MTTypes::ToMTLStencilOperation(src.depthFailOp);
    dst.depthStencilPassOperation   = MTTypes::ToMTLStencilOperation(src.depthPassOp);
    dst.stencilCompareFunction      = MTTypes::ToMTLCompareFunction(src.compareOp);
    dst.readMask                    = src.readMask;
    dst.writeMask                   = src.writeMask;
}

static void FillDefaultMTStencilDesc(MTLStencilDescriptor* dst)
{
    dst.stencilFailureOperation     = MTLStencilOperationKeep;
    dst.depthFailureOperation       = MTLStencilOperationKeep;
    dst.depthStencilPassOperation   = MTLStencilOperationKeep;
    dst.stencilCompareFunction      = MTLCompareFunctionAlways;
    dst.readMask                    = 0;
    dst.writeMask                   = 0;
}

MTGraphicsPSO::MTGraphicsPSO(
    id<MTLDevice>                       device,
    const GraphicsPipelineDescriptor&   desc,
    const MTRenderPass*                 defaultRenderPass)
:
    MTPipelineState { /*isGraphicsPSO:*/ true }
{
    /* Convert standalone parameters */
    cullMode_       	= MTTypes::ToMTLCullMode(desc.rasterizer.cullMode);
    winding_            = (desc.rasterizer.frontCCW ? MTLWindingCounterClockwise : MTLWindingClockwise);
    fillMode_           = MTTypes::ToMTLTriangleFillMode(desc.rasterizer.polygonMode);
    primitiveType_  	= MTTypes::ToMTLPrimitiveType(desc.primitiveTopology);
    clipMode_           = (desc.rasterizer.depthClampEnabled ? MTLDepthClipModeClamp : MTLDepthClipModeClip);

    depthBias_          = desc.rasterizer.depthBias.constantFactor;
    depthSlope_         = desc.rasterizer.depthBias.slopeFactor;
    depthClamp_         = desc.rasterizer.depthBias.clamp;

    blendColorDynamic_  = desc.blend.blendFactorDynamic;
    blendColorEnabled_  = IsStaticBlendFactorEnabled(desc.blend);
    blendColor_[0]      = desc.blend.blendFactor.r;
    blendColor_[1]      = desc.blend.blendFactor.g;
    blendColor_[2]      = desc.blend.blendFactor.b;
    blendColor_[3]      = desc.blend.blendFactor.a;

    /* Create render pipeline and depth-stencil states */
    CreateRenderPipelineState(device, desc, defaultRenderPass);
    CreateDepthStencilState(device, desc);
}

void MTGraphicsPSO::Bind(id<MTLRenderCommandEncoder> renderEncoder)
{
    [renderEncoder setRenderPipelineState:renderPipelineState_];
    [renderEncoder setDepthStencilState:depthStencilState_];
    [renderEncoder setCullMode:cullMode_];
    [renderEncoder setFrontFacingWinding:winding_];
    [renderEncoder setTriangleFillMode:fillMode_];

    if (@available(macOS 10.11, iOS 11, *))
        [renderEncoder setDepthClipMode:clipMode_];

    #ifndef LLGL_OS_IOS//TODO: disabled for testing iOS
    [renderEncoder setDepthBias:depthBias_ slopeScale:depthSlope_ clamp:depthClamp_];

    if (!stencilRefDynamic_)
    {
        if (stencilFrontRef_ != stencilBackRef_)
            [renderEncoder setStencilFrontReferenceValue:stencilFrontRef_ backReferenceValue:stencilBackRef_];
        else
            [renderEncoder setStencilReferenceValue:stencilFrontRef_];
    }
    #endif

    if (blendColorEnabled_)
    {
        [renderEncoder
            setBlendColorRed:   blendColor_[0]
            green:              blendColor_[1]
            blue:               blendColor_[2]
            alpha:              blendColor_[3]
        ];
    }
}


/*
 * ======= Private: =======
 */

static MTLColorWriteMask ToMTLColorWriteMask(const ColorRGBAb& color)
{
    MTLColorWriteMask mask = MTLColorWriteMaskNone;

    if (color.r)
        mask |= MTLColorWriteMaskRed;
    if (color.g)
        mask |= MTLColorWriteMaskGreen;
    if (color.b)
        mask |= MTLColorWriteMaskBlue;
    if (color.a)
        mask |= MTLColorWriteMaskAlpha;

    return mask;
}

static void FillColorAttachmentDesc(
    MTLRenderPipelineColorAttachmentDescriptor* dst,
    MTLPixelFormat                              pixelFormat,
    const BlendDescriptor&                      blendDesc,
    const BlendTargetDescriptor&                targetDesc)
{
    /* Render pipeline state */
    dst.pixelFormat                 = pixelFormat;
    dst.writeMask                   = ToMTLColorWriteMask(targetDesc.colorMask);

    /* Controlling blend operation */
    dst.blendingEnabled             = (targetDesc.blendEnabled ? YES : NO);
    dst.alphaBlendOperation         = MTTypes::ToMTLBlendOperation(targetDesc.alphaArithmetic);
    dst.rgbBlendOperation           = MTTypes::ToMTLBlendOperation(targetDesc.colorArithmetic);

    /* Blend factors */
    dst.destinationAlphaBlendFactor = MTTypes::ToMTLBlendFactor(targetDesc.dstAlpha);
    dst.destinationRGBBlendFactor   = MTTypes::ToMTLBlendFactor(targetDesc.dstColor);
    dst.sourceAlphaBlendFactor      = MTTypes::ToMTLBlendFactor(targetDesc.srcAlpha);
    dst.sourceRGBBlendFactor        = MTTypes::ToMTLBlendFactor(targetDesc.srcColor);
}

static id<MTLFunction> GetNativeMTShader(const Shader* shader)
{
    return (shader != nullptr ? LLGL_CAST(const MTShader*, shader)->GetNative() : nullptr);
}

void MTGraphicsPSO::CreateRenderPipelineState(
    id<MTLDevice>                       device,
    const GraphicsPipelineDescriptor&   desc,
    const MTRenderPass*                 defaultRenderPass)
{
    /* Get native shader functions */
    auto vertexShaderMT = LLGL_CAST(const MTShader*, desc.vertexShader);
    if (!vertexShaderMT)
        throw std::invalid_argument("cannot create Metal pipeline without vertex shader");

    /* Get number of patch control points if a post-tessellation vertex function is specified */
    numPatchControlPoints_ = vertexShaderMT->GetNumPatchControlPoints();

    if (id<MTLFunction> vertexFunc = vertexShaderMT->GetNative())
        patchType_ = [vertexFunc patchType];

    /* Get render pass object */
    const MTRenderPass* renderPassMT = nullptr;
    if (auto renderPass = desc.renderPass)
        renderPassMT = LLGL_CAST(const MTRenderPass*, renderPass);
    else if (defaultRenderPass != nullptr)
        renderPassMT = defaultRenderPass;
    else
        throw std::invalid_argument("cannot create graphics pipeline without render pass");

    /* Create render pipeline state */
    MTLRenderPipelineDescriptor* psoDesc = [[MTLRenderPipelineDescriptor alloc] init];
    {
        psoDesc.vertexDescriptor        = vertexShaderMT->GetMTLVertexDesc();
        psoDesc.alphaToCoverageEnabled  = MTBoolean(desc.blend.alphaToCoverageEnabled);
        psoDesc.alphaToOneEnabled       = NO;
        psoDesc.fragmentFunction        = GetNativeMTShader(desc.fragmentShader);
        psoDesc.vertexFunction          = GetNativeMTShader(desc.vertexShader);

        if (@available(iOS 12.0, *))
            psoDesc.inputPrimitiveTopology = MTTypes::ToMTLPrimitiveTopologyClass(desc.primitiveTopology);

        /* Initialize pixel formats from render pass */
        const auto& colorAttachments = renderPassMT->GetColorAttachments();
        for (std::size_t i = 0, n = std::min(colorAttachments.size(), std::size_t(8u)); i < n; ++i)
        {
            FillColorAttachmentDesc(
                psoDesc.colorAttachments[i],
                colorAttachments[i].pixelFormat,
                desc.blend,
                desc.blend.targets[desc.blend.independentBlendEnabled ? i : 0]
            );
        };

        psoDesc.depthAttachmentPixelFormat      = renderPassMT->GetDepthAttachment().pixelFormat;
        psoDesc.stencilAttachmentPixelFormat    = renderPassMT->GetStencilAttachment().pixelFormat;
        psoDesc.rasterizationEnabled            = (desc.rasterizer.discardEnabled ? NO : YES);
        psoDesc.rasterSampleCount               = (desc.rasterizer.multiSampleEnabled ? renderPassMT->GetSampleCount() : 1u);

        /* Specify tessellation state */
        if (numPatchControlPoints_ > 0)
        {
            #ifdef LLGL_OS_IOS
            psoDesc.maxTessellationFactor               = std::min(desc.tessellation.maxTessFactor, 16u);
            #else
            psoDesc.maxTessellationFactor               = std::min(desc.tessellation.maxTessFactor, 64u);
            #endif
            psoDesc.tessellationFactorScaleEnabled      = NO;
            psoDesc.tessellationFactorFormat            = MTLTessellationFactorFormatHalf; // Can only be <half>
            psoDesc.tessellationControlPointIndexType   = MTTypes::ToMTLPatchIndexType(desc.tessellation.indexFormat);
            psoDesc.tessellationFactorStepFunction      = MTLTessellationFactorStepFunctionPerPatchAndPerInstance; // Same behavior as in D3D
            psoDesc.tessellationOutputWindingOrder      = (desc.tessellation.outputWindingCCW ? MTLWindingCounterClockwise : MTLWindingClockwise);
            psoDesc.tessellationPartitionMode           = MTTypes::ToMTLPartitionMode(desc.tessellation.partition);
        }
    }
    id<NSError> error = nil;
    renderPipelineState_ = [device newRenderPipelineStateWithDescriptor:psoDesc error:&error];
    [psoDesc release];

    if (!renderPipelineState_)
        MTThrowIfCreateFailed(error, "MTLRenderPipelineState");

    /* Create compute PSO for tessellation stage */
    if (numPatchControlPoints_ > 0)
    {
        if (auto tessComputeShaderMT = LLGL_CAST(const MTShader*, desc.tessControlShader))
        {
            tessPipelineState_ = [device newComputePipelineStateWithFunction:tessComputeShaderMT->GetNative() error:&error];
            if (!tessPipelineState_)
                MTThrowIfCreateFailed(error, "MTLComputePipelineState");
        }
        else
            throw std::invalid_argument("cannot create Metal tessellation pipeline without tessellation compute shader");
    }
}

void MTGraphicsPSO::CreateDepthStencilState(
    id<MTLDevice>                       device,
    const GraphicsPipelineDescriptor&   desc)
{
    MTLDepthStencilDescriptor* depthStencilDesc = [[MTLDepthStencilDescriptor alloc] init];
    {
        /* Convert depth descriptor */
        depthStencilDesc.depthWriteEnabled          = MTBoolean(desc.depth.writeEnabled);
        if (desc.depth.testEnabled)
            depthStencilDesc.depthCompareFunction   = MTTypes::ToMTLCompareFunction(desc.depth.compareOp);
        else
            depthStencilDesc.depthCompareFunction   = MTLCompareFunctionAlways;

        /* Convert stencil descriptor */
        if (desc.stencil.testEnabled)
        {
            Convert(depthStencilDesc.frontFaceStencil, desc.stencil.front);
            Convert(depthStencilDesc.backFaceStencil, desc.stencil.back);
        }
        else
        {
            FillDefaultMTStencilDesc(depthStencilDesc.frontFaceStencil);
            FillDefaultMTStencilDesc(depthStencilDesc.backFaceStencil);
        }

        /* Store stencil reference values */
        stencilFrontRef_    = desc.stencil.front.reference;
        stencilBackRef_     = desc.stencil.back.reference;
        stencilRefDynamic_  = desc.stencil.referenceDynamic;
    }
    depthStencilState_ = [device newDepthStencilStateWithDescriptor:depthStencilDesc];
    [depthStencilDesc release];
}


} // /namespace LLGL



// ================================================================================
