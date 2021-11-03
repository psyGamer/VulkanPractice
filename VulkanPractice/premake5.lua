project "VulkanPractice"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	
	fastuptodate "false"
	
	targetdir("../Bin/" .. outputdir .. "/%{prj.name}")
	objdir("../Bin_int/" .. outputdir .. "/%{prj.name}")
		
	includedirs {
		os.getenv("VULKAN_SDK").."/Include",
		os.getenv("GLFW").."/include",
		
		"C:/Programs/glm",
		
		"vendor"
	}
	
	libdirs {
		os.getenv("VULKAN_SDK").."/Lib",
		os.getenv("GLFW").."/lib-vc2019"
	}
	
	links {
		"vulkan-1",
		"glfw3_mt"
	}
		
	files {
		"src/**.cpp",
		"src/**.tpp",
		"src/**.hpp",
		"src/**.h"
	}
	
	prebuildcommands {
		"call compile_shaders.bat"
	}
	
	filter { "configurations:Debug" }
		buildoptions "/MTd"
		runtime "Debug"
		symbols "on"
		
	filter { "configurations:Release" }
		buildoptions "/MT"
		runtime "Release"
		optimize "on"
	