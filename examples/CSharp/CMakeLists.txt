#
# CMakeLists.txt file for LLGL C# example projects
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

cmake_minimum_required(VERSION 3.7)

project(LLGL_ExamplesCSharp)


# === Source files ===

# Example project files
find_source_files(FilesExampleBaseCSharpAll "*.cs" "${EXAMPLE_CSHARP_PROJECTS_DIR}/ExampleBase")

find_project_source_files( FilesExampleCSharp_HelloTriangle "${EXAMPLE_CSHARP_PROJECTS_DIR}/HelloTriangle/HelloTriangle.cs" )
find_project_source_files( FilesExampleCSharp_Texturing     "${EXAMPLE_CSHARP_PROJECTS_DIR}/Texturing/Texturing.cs"         )


# === Projects ===

# Example Projects
if(LLGL_BUILD_EXAMPLES AND LLGL_BUILD_WRAPPER_CSHARP)
    # Base project (static lib) for all examples
    add_library(ExampleBase_CSharp SHARED ${FilesExampleBaseCSharpAll})
    set_target_properties(ExampleBase_CSharp PROPERTIES LINKER_LANGUAGE CSharp DEBUG_POSTFIX "D")
    target_link_libraries(ExampleBase_CSharp SharpLLGL)
    set_target_properties(ExampleBase_CSharp PROPERTIES FOLDER "Examples (CSharp)")
    
    # C# wrapper examples
    add_llgl_example_project(Example_CSharp_HelloTriangle CSharp "${FilesExampleCSharp_HelloTriangle}" SharpLLGL)
    add_llgl_example_project(Example_CSharp_Texturing     CSharp "${FilesExampleCSharp_Texturing}"     ExampleBase_CSharp)
endif()


