@echo off
SET GLSLC_PATH=%VULKAN_SDK%\Bin\glslangValidator.exe
SET CURRENT_PATH=%~dp0
%GLSLC_PATH% -V %CURRENT_PATH%shader.vert -o %CURRENT_PATH%vert.spv
%GLSLC_PATH% -V %CURRENT_PATH%shader.frag -o %CURRENT_PATH%frag.spv
xcopy /I /Y %CURRENT_PATH%*.spv %CURRENT_PATH%..\..\out\build\x64-Debug\Client\shaders
xcopy /I /Y %CURRENT_PATH%*.spv %CURRENT_PATH%\compiled
del %CURRENT_PATH%*.spv