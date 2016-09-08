# OutOfOrderRasterization 

This is a README file for our first Anvil sample. Anvil is a framework for Vulkan&trade;.
The README is organized as a FAQ.

What license governs sample usage?
------
MIT. See See `LICENSE.txt`.

What is this?
------
OutOfOrderRasterization is an example application based on the Anvil framework
that renders 10k teapots on screen. It uses various Anvil wrappers, so you can
use it to get a better understanding of how various parts of the library can be used.

...and its Vulkan-related code only takes ~45kbytes!

  
What platforms and hardware does it work on?
------
Currently, the sample has been confirmed to build and work correctly under:
- 32- and 64-bit Linux   (Ubuntu)
- 32- and 64-bit Windows (7, 8.1, 10)

What are the requirements?
------
In order to build the sample, you will need the following software:
- C++11 compiler.
- CMake
- Vulkan SDK 1.0.13.0 or newer

To build the sample on Linux, additional packages should be installed:
- libxcb-keysyms (For ubuntu, use "apt-get install libxcb-keysyms1-dev")

Does the sample work only with AMD driver?
------
The sample should (and has been verified to) work on any Linux/Windows Vulkan
implementation, not just AMD.

Attribution
-----------

* AMD, the AMD Arrow logo, Radeon, and combinations thereof are either registered trademarks or trademarks of Advanced Micro Devices, Inc. in the United States and/or other countries.
* Microsoft, Visual Studio, and Windows are either registered trademarks or trademarks of Microsoft Corporation in the United States and/or other countries.
* Linux is the registered trademark of Linus Torvalds in the U.S. and other countries.
* Vulkan and the Vulkan logo are trademarks of the Khronos Group, Inc.
