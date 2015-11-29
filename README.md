
# GPU-Based Voxelization
It is a GPU-based voxelization method to convert complex 3D scenes into high resolution voxel data in real-time. It uses a combination of vertex, geometry, and pixel shaders to generate voxel data from triangles. This system visualizes voxel data after converting, and it can store the voxel data in a file.

### Dependencies
* Visual Studio 2013 (https://www.visualstudio.com/en-us/downloads/download-visual-studio-vs.aspx)
* DirectX SDK June 2010 (https://www.microsoft.com/en-us/download/details.aspx?id=6812)
* Effects11 in DirectX SDK June 2010 (https://www.microsoft.com/en-us/download/details.aspx?id=6812)
* CEGUI 0.8.4 (http://cegui.org.uk/)

### Interactions
Mouse/Keyboard:
- Mouse : Rotate camera (left click on press)
- W : Move camera front
- S : Move camera back
- A : Move camera left
- D : Move camera right

Menu:
* Model : Select a model to voxelize
* Information : Select a type of data to show
* Resolution: Select the resolution of voxels
* Raw Data
  * Display : Show a slice of 3D texture
  * Slice : The index of slice
* Voxelization : Re-voxelize the same model
* Filename : The output file name
* Output : Output the voxel data to a file