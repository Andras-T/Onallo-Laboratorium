REM This is a helper script to compile the shaders

SET GLSLC_PATH=%VULKAN_SDK%\Bin\glslangValidator.exe
REM %~dp0 specifies the full path to a file
SET CURRENT_PATH=%~dp0

REM Add new command for new shader!
REM vertex shaders
%GLSLC_PATH% -V %CURRENT_PATH%shader.vert -o %CURRENT_PATH%vert.spv

REM fragment shaders
%GLSLC_PATH% -V %CURRENT_PATH%shader.frag -o %CURRENT_PATH%frag.spv

REM compute shaders
REM %GLSLC_PATH% -V %CURRENT_PATH%shader.comp -o %CURRENT_PATH%comp.spv

REM copying the shaders to the output directory
xcopy /I /Y %CURRENT_PATH%*.spv %CURRENT_PATH%..\..\out\build\x64-Debug\Client\shaders
REM moving the shaders to the compiled directory
xcopy /I /Y %CURRENT_PATH%*.spv %CURRENT_PATH%\compiled
del %CURRENT_PATH%*.spv

REM exit