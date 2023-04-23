---------------------------------------
-- file   : premake for sssg
-- author : sunhuiwei
---------------------------------------

-- Load local premake file if it exists.
function load_local(prj)
    if os.isfile(prj.."/premake4.lua") then
      local func = loadfile(prj.."/premake4.lua");
      func();
    end
end

-- Base header paths.
local base_include_dirs = {
      "base",
      "base/xlib",
      "base/xlib/Recast",
      "base/xlib/Detour",
      "base/protobuf",
      "base/config",
      "Command",
      "Common",
}

-- Generate a server lib project.
function server_lib_project(prj)
  project("lib"..prj)
    targetname(prj)
    kind "StaticLib"
    files {
      prj.."/**.cpp",
      prj.."/**.c",
      prj.."/**.cc",
    }
    excludes {
      prj.."/main.cpp"
    }
    load_local(prj)
    includedirs {
      prj,
    }
    includedirs(base_include_dirs)
end

-- Generate a server executable project.
function server_executable_project(prj)
  project(prj)
    targetname(prj)
    kind "ConsoleApp"
    files {
      prj.."/main.cpp"
    }
    load_local(prj)
    includedirs { prj }
    includedirs(base_include_dirs)
    links {
      "lib"..prj,
      "libCommon",
      "libbase",
    }
end

-- Generate a server test project.
function server_test_project(prj)
  project("test"..prj)
    kind "ConsoleApp"
    files {
      "tests/"..prj.."/**.cpp",
      "tests/"..prj.."/**.c",
      "tests/"..prj.."/**.cc",
    }
    includedirs { prj }
    includedirs(base_include_dirs)
    libdirs {
      gtest
    }
    links {
      "lib"..prj,
      "libCommon",
      "libbase",
      "gtest_main",
      "gtest",
    }
    targetdir "bin/Test"
end

-- Generate a base lib project.
function base_lib_project(prj)
  project("lib"..prj)
    targetname(prj)
    kind "StaticLib"
    files {
      prj.."/**.cpp",
      prj.."/**.c",
      prj.."/**.cc",
    }
    load_local(prj)
    includedirs { prj }
    includedirs(base_include_dirs)
end

-- generate a base test project
function base_test_project(prj)
  project("test"..prj)
    kind "ConsoleApp"
    files {
      "tests/"..prj.."/**.cpp",
      "tests/"..prj.."/**.c",
      "tests/"..prj.."/**.cc",
    }
    includedirs { prj }
    includedirs(base_include_dirs)
    libdirs {
      gtest
    }
    links {
      "lib"..prj,
      "gtest_main",
      "gtest",
    }
    flags { "Symbols" }
    targetdir "bin/Test"
end


-- Generate base projects.
function base_projects(prj)
  base_lib_project(prj)
  base_test_project(prj)
end

-- Generate server projects.
function server_projects(prj)
  server_lib_project(prj)
  server_executable_project(prj)
  server_test_project(prj)
end

-- The solution with configurations.
solution "SSSG"
  location "./build"
  language "C++"
  objdir "./obj"
  
  -- Define _GLIBCXX_USE_CXX11_ABI
  local cxx11abi = os.getenv("GLIBCXX_USE_CXX11_ABI")
  if cxx11abi ~= nil then
    defines { "_GLIBCXX_USE_CXX11_ABI=" .. cxx11abi }
  else
    defines { "_GLIBCXX_USE_CXX11_ABI=0" }
  end

  buildoptions {
    "-std=c++0x",
    "-g3",
    "-Wall", 
  }

  configurations { "Debug", "Release" }

  configuration "linux or macosx"
    includedirs {
      ".",
      "/usr/local/include",
      "/usr/local/include/mysql",
      "/usr/local/mysql/include",
      "/usr/include",
      "/usr/include/log4cxx",
      "/usr/include/libxml2",
      "/usr/include/mysql",
    }
    libdirs {
      "/usr/local/lib",
      "/usr/local/mysql/lib",
      "/usr/lib",
      "/usr/lib/mysql",
      "/lib64",
      "/usr/lib64",
      "/usr/lib64/mysql",
    }
    links {
      "xml2",
      "pthread",
      "mysqlclient",
      "log4cxx",
      "protobuf",
      "hiredis",
      "jansson",
      "jemalloc",
    }

  configuration "Debug"
    defines {
      "_ALL_SUPER_GM",
      -- "_OLD_TRADE",
      "_SQL_DEBUG",
      "_DEBUG",
      "DEBUG",
      "_ROBOT_DEBUG",
    }
    buildoptions {
      -- "-O2",
      "-fno-strict-aliasing",
      "-fno-short-enums",
      "-fno-schedule-insns", 
      "-pg",
      "$(DEBUG_FLAG)",
    }
    flags "Symbols"
    targetdir "bin/Debug"

  configuration "Release"
    defines {
      -- "-D_OLD_TRADE",
      --"_SQL_DEBUG",
      "NDEBUG",
    }    
    buildoptions {
      "-O2",
      "-fno-strict-aliasing", 
      "-fno-short-enums", 
      "-fno-schedule-insns", 
      "-Wno-unknown-pragmas", 
      -- "-Werror",
    }
    flags "Symbols"
    targetdir "bin/Release"
  


  -- All projects.
  base_projects("base")
  base_projects("Common")
  server_projects("SceneServer")
  server_projects("SessionServer")
  server_projects("RecordServer")
  server_projects("GateServer")
  server_projects("SuperServer")
  server_projects("ProxyServer")
  server_projects("StatServer")
  server_projects("TradeServer")
  server_projects("MatchServer")
  server_projects("SocialServer")
  server_projects("TeamServer")
  server_projects("GuildServer")
  server_projects("GlobalServer")
  server_projects("GZoneServer")
  server_projects("AuctionServer")
  server_projects("Robots")
  server_projects("GProxyServer")
  server_projects("WeddingServer")
  server_projects("DataServer")
