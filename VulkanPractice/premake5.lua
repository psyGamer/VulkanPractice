project "VulkanPractice"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	
	targetdir("../Bin/" .. outputdir .. "/%{prj.name}")
	objdir("../Bin_int/" .. outputdir .. "/%{prj.name}")
		
	includedirs {
		"%VULKAN_SDK%/Include",
		"%GLFW%/include",
		"%GLM%",
		"Vendor"
	}
		
	files {
		"src/**.cpp",
		"src/**.tpp",
		"src/**.hpp",
		"src/**.h"
	}
	
	prebuildcommands {
		"echo hi"
	}
	
	
	filter { "configurations:Debug" }
		buildoptions "/MTd"
		runtime "Debug"
		symbols "on"
		
	filter { "configurations:Release" }
		buildoptions "/MT"
		runtime "Release"
		optimize "on"
	