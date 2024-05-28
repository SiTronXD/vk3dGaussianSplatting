# 3D Gaussian Splatting
A Vulkan renderer for 3D Gaussian Splatting (original paper: https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/3d_gaussian_splatting_low.pdf). It works by first importing 3D gaussians from a given .ply-file in a pre-processing step. The renderer sorts gaussians by depth, projects, rasterizes and then blends gaussians in real-time. A GPU-driven approach is being used and relies only on compute shaders, rather than utilizing the graphics pipeline. Still, a number of optimizations needed to be implemented in order for the application to run as one would expect.

# Optimizations
Implemented optimizations which were proposed by the original paper:
* Interleaving loading and rendering of gaussians to increase throughput of both bandwidth and arithmetic operations
* Tiled shading
* Frustum culling

Optimizations from my own experimentation:
* Utilizing GPU-based radix sort rather than bitonic merge sort (there is still room for improvement of the sorting implementation)
* Indirect dispatches, to sort only the necessary number of gaussians per screen space tile
* Subgroups, to share data and operations among threads where possible
* Ordering gaussians in the GPU buffer according to a Z-order curve w.r.t. 3D position, to increase cache coherency

# Vulkan features used
* Version 1.3
* Synchronization 2
* Push descriptors

# Assets used
* Pre-trained models: https://github.com/graphdeco-inria/gaussian-splatting

# Libraries and APIs used
* Dear ImGUI: GUI for debugging
* EnTT: entity components system
* hapPLY: 3D gaussian importing
* fast_obj: OBJ mesh importing
* GLFW: window management
* GLM: vector and matrix math
* stb: image importing
* Vulkan: graphics and GPU management
* Vulkan Memory allocator: GPU memory management
