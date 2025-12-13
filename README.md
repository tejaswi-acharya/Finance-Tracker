
Finance Tracker Application


This is a GUI-based finance tracker application built with C++ and the raylib library. The recommended way to set up the raylib dependency is by using the vcpkg package manager.


1. Libraries Required


This project requires the raylib library. The easiest way to install it is by using vcpkg.

- Library: raylib
- Package Manager: vcpkg
- vcpkg GitHub: https://github.com/microsoft/vcpkg

2. Compiling Instructions


-------------------------VISUAL STUDIO 2022------------------------------------------------------------
To compile this project, you need to first install raylib using vcpkg and then configure your build system.

Step 1: Download and Bootstrap vcpkg
1.  Go to the vcpkg GitHub page and download the ZIP file.
2.  Extract the ZIP file to a convenient location on your computer (e.g., C:\vcpkg).
3.  Open a terminal (or PowerShell on Windows) and navigate to the extracted vcpkg folder.
4.  Run the following command to set up vcpkg:
    - On Windows: .\bootstrap-vcpkg.bat
   
Step 2: Install raylib
1.  In the same terminal, use vcpkg to install the raylib library:
    - For 64-bit systems: `vcpkg install raylib:x64-windows` 
    

Step 3: Compile the Project
After installing raylib with vcpkg, you can compile the `main.cpp` file in you Visual Studio 2022. 
---

