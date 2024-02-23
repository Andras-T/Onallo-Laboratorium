# Onallo-Laboratorium

This project is a C++ application that uses Vulkan, a low-level graphics API, to create a networked app that can stream images between a client and a server. The client app will send user input to the server, which will process it and render the current image using Vulkan. The server will then send the image back to the client, which will display it on the screen. The app will use UDP protocol to achieve fast network communication. The app will also use Valve GameNetworkingSockets library, which provides a high-level interface for network sockets, and ImGui library, which provides a simple and easy-to-use graphical user interface.

INSTALL Visual Studio 2022
----------------------

Download the required components for c++ desktop development

INSTALL Vulkan
---------------------

- Step 1: First, you need to check if your graphics card supports Vulkan. You can do this by launching the DirectX Diagnostic Tool1 and clicking on the Display tab Under Drivers, check for the Vulkan Support entry. If it says No, then your graphics card does not support Vulkan. If it says Yes, then your card supports Vulkan but its not enabled by default

- Step 2: Download it from here https://vulkan.lunarg.com/ and follow its instructions.

INSTALL OPENSSL
-----------------------

- Step 1: Install Perl - Install the Strawberry version, much easier to install and it installs everything and also adds them automatically to the Windows PATH variables

- Step 2: Install NASM, and add it to the Windows system (or your user's) PATH variables. I ended up adding it only to my user's variables PATH: C:\Users\<username>\AppData\Local\bin\NASM

- Step 3: Install Visual Studio (I have Visual Studio Community 2022), and install the Desktop development with c++.

- Step 4: Download and install the Build Tools for Visual Studio: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022

- Step 5: After installing the build tools, launch the Visual Studio installer. In the installer, you will now see the Build Tools. Click on "Modify" under the Visual Studio Build Tools and then install the needed packages for the OpenSSL installation (c++ desktop dev), it's what's going to install nmake

- Step 6: Clone the openssl repository to some folder on your PC (I cloned it in C:/dev so I ended up having C:/dev/openssl/), and fix the line endings by running the following commands:
	git clone git://git.openssl.org/openssl.git
	cd openssl
	git config core.autocrlf false
	git config core.eol lf
	git checkout .

- Step 7: In Windows Search, search for "Developer Command Prompt for VS 2022" (Or any of your versions), and run it as administrator:

- Step 8: You need to set the right environment for the version of OpenSSL you want to install, otherwise build will fail. In my case, I wanted to install OpenSSL for 64-bit systems, copy-paste the following (including the quotes, and change the path according to your Visual Studio installation path):
	"C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
	which will then set the environment. The " is needed as well!

- Step 9: From the same Developer Command Prompt, cd into the folder you cloned the openssl source code, in my case it was C:/dev/openssl, and then follow the steps from the OpenSSL guide:
	perl Configure VC-WIN64A
	nmake
	nmake test
	nmake install
	Note that these steps take time, it took me around 20-30 minutes to finish all these 4 commands

- Step 10: That's it! It's installed! You can find the OpenSSL executable (openssl.exe) at C:/dev/openssl/apps. (And add it to Windows system or user's PATH variables if you want)

INSTALL VCPKG
-----------------------

- Step 1: clone the vcpkg repository: git clone https://github.com/Microsoft/vcpkg.git

- Step 2: Run the bootstrap script to build vcpkg: vcpkg\bootstrap-vcpkg.bat

INSTALL Protobuf:
-----------------------

- Step 1: Protobuf and its dependencies can be installed directly by using vcpkg: vcpkg install protobuf protobuf:x64-windows
	If you only need the protoc binary, you can download it from the release page: 
		https://github.com/protocolbuffers/protobuf/releases/latest

- Step 2: Add to the sytem/user variables a new variable. 
	Name: protobuf_DIR   
	Value: C:\dev\vcpkg\packages\protobuf_x64-windows\share\protobuf
