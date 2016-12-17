# Build

1. install [Visual Studio 2015](https://www.visualstudio.com/), [Vulkan SDK](https://lunarg.com/vulkan-sdk/), [FBX SDK 2017.1 VS2015](http://www.autodesk.com/products/fbx/overview)

2. setup environment variable `FBX_DIR`, point to where the FBX SDK installed, like:

    FBX_DIR = D:\app\FBX_SDK_2017.1

3. use cmake 3.1+ to generate vs project files

# TODO

* vulkan texture loader
    * setImageLayout line# 237
* [scene loader](https://nlguillemot.wordpress.com/2016/11/18/opengl-renderer-design/)
	* [Buffer offset](https://developer.nvidia.com/vulkan-memory-management)
* configurable rendering pipeline
    * choose a config file format
    * 
* GUI



