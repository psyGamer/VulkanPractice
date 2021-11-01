@echo OFF

pushd Shaders

%VULKAN_SDK%\Bin\glslangValidator.exe -V shader.vert
%VULKAN_SDK%\Bin\glslangValidator.exe -V shader.frag

popd

pause