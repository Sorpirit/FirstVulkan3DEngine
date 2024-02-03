@ECHO OFF
if exist "shaders" goto :run

echo "shader folder is missing"
echo error: 1

echo error: %errorlevel%

pause
goto :end

:run
if exist "shaders/compiled" rmdir /s /q "shaders/compiled"
mkdir "shaders/compiled"

dotnet-script pre_compile_shaders.csx "shaders" "shaders//compiled" "C://VulkanSDK//1.3.224.1//Bin//glslc.exe"
pause
:end