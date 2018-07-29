/*
 * Test9_Metal.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Helper.h"
#include <LLGL/Utility.h>
#include <memory>
#include <iostream>
#include <string>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include "../tutorial/TutorialBase/stb/stb_image.h"


int main()
{
    try
    {
        // Load render system module
        auto renderer = LLGL::RenderSystem::Load("OpenGL");

        // Create render context
        LLGL::RenderContextDescriptor contextDesc;

        contextDesc.videoMode.resolution    = { 800, 600 };
        contextDesc.vsync.enabled           = true;

        auto context = renderer->CreateRenderContext(contextDesc);

        // Setup window title
        auto& window = static_cast<LLGL::Window&>(context->GetSurface());

        auto title = "LLGL Test 10 ( " + renderer->GetName() + " )";
        window.SetTitle(std::wstring(title.begin(), title.end()));

        // Setup input controller
        auto input = std::make_shared<LLGL::Input>();
        window.AddEventListener(input);

        window.Show();

        // Create vertex buffer
        struct Vertex
        {
            Gs::Vector2f        position;
            LLGL::ColorRGBAub   color;
        }
        vertices[] =
        {
            { { -0.5f, -0.5f }, { 255,   0,   0, 255 } },
            { { -0.5f,  0.5f }, {   0, 255,   0, 255 } },
            { {  0.5f, -0.5f }, { 255,   0, 255, 128 } },
            { {  0.5f,  0.5f }, {   0,   0, 255, 128 } },
        };

        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RG32Float });
        vertexFormat.AppendAttribute({ "color",    LLGL::Format::RGBA8UNorm });
        vertexFormat.stride = sizeof(Vertex);

        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.type                   = LLGL::BufferType::Vertex;
            vertexBufferDesc.size                   = sizeof(vertices);
            vertexBufferDesc.vertexBuffer.format    = vertexFormat;
        }
        auto vertexBuffer = renderer->CreateBuffer(vertexBufferDesc, vertices);

        // Create shader program
        LLGL::ShaderProgramDescriptor shaderProgramDesc;
        {
            shaderProgramDesc.vertexFormats     = { vertexBufferDesc.vertexBuffer.format };
            shaderProgramDesc.vertexShader      = renderer->CreateShader({ LLGL::ShaderType::Vertex,   "BlendTest.vert" });
            shaderProgramDesc.fragmentShader    = renderer->CreateShader({ LLGL::ShaderType::Fragment, "BlendTest.frag" });
        }
        auto shaderProgram = renderer->CreateShaderProgram(shaderProgramDesc);

        if (shaderProgram->HasErrors())
            throw std::runtime_error(shaderProgram->QueryInfoLog());

        // Create graphics pipeline
        static const std::size_t numPipelines = 3;

        LLGL::GraphicsPipeline* pipeline[numPipelines] = {};

        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram      = shaderProgram;
            pipelineDesc.primitiveTopology  = LLGL::PrimitiveTopology::TriangleStrip;
        }
        pipeline[0] = renderer->CreateGraphicsPipeline(pipelineDesc);

        {
            pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::LineLoop;
        }
        pipeline[1] = renderer->CreateGraphicsPipeline(pipelineDesc);

        {
            pipelineDesc.blend.targets[0].blendEnabled = true;
        }
        pipeline[2] = renderer->CreateGraphicsPipeline(pipelineDesc);

        // Create command buffer
        auto commandQueue = renderer->GetCommandQueue();
        auto commands = renderer->CreateCommandBuffer();

        std::size_t pipelineIndex = 0;

        // Main loop
        while (window.ProcessEvents() && !input->KeyDown(LLGL::Key::Escape))
        {
            if (input->KeyDownRepeated(LLGL::Key::Tab))
            {
                if (input->KeyPressed(LLGL::Key::Shift))
                {
                    if (pipelineIndex == 0)
                        pipelineIndex = numPipelines - 1;
                    else
                        pipelineIndex = (pipelineIndex - 1);
                }
                else
                    pipelineIndex = (pipelineIndex + 1) % numPipelines;
            }

            commands->Begin();
            {
                commands->SetVertexBuffer(*vertexBuffer);
                commands->BeginRenderPass(*context);
                {
                    commands->SetViewport(LLGL::Viewport{ { 0, 0 }, context->GetVideoMode().resolution });
                    commands->Clear(LLGL::ClearFlags::Color);

                    commands->SetGraphicsPipeline(*pipeline[pipelineIndex]);
                    commands->Draw(4, 0);
                }
                commands->EndRenderPass();
            }
            commands->End();
            commandQueue->Submit(*commands);

            context->Present();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        #ifdef _WIN32
        system("pause");
        #endif
    }

    return 0;
}
