-- premake5.lua
workspace "manual-mapper"
   configurations { "Debug", "Release" }
   platforms { "x86", "x86_64" }

project "manual-mapper-core"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   toolset "clang"
   targetdir "bin/%{cfg.buildcfg}"

   files { "include/*.hpp", "src/**.h", "src/**.hpp", "src/**.cpp" }

   includedirs { "include", "src" }

   pchheader "src/common.hpp"
   pchsource "src/common.cpp"
    
   forceincludes
   {
      "common.hpp"
   }   

   defines
   {
      "NOMINMAX",
      "WIN32_LEAN_AND_MEAN"
   }

   
   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

   filter "platforms:x86"
      system "Windows"
      architecture "x86"

   filter "platforms:x86_64"
      system "Windows"
      architecture "x86_64"

      

