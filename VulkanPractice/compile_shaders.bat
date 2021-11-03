@echo OFF

pushd assets
pushd shaders

%VULKAN_SDK%\Bin\glslc.exe shader.vert -o shader.vert.spv
echo Compiled shader.vert
%VULKAN_SDK%\Bin\glslc.exe shader.frag -o shader.frag.spv
echo Compiled shader.frag

popd
popd