C:/VulkanSDK/1.3.231.1/Bin/glslangValidator.exe -V  test.vert -o vert.spv
C:/VulkanSDK/1.3.231.1/Bin/glslangValidator.exe -V  test.frag -o frag.spv
C:/VulkanSDK/1.3.231.1/Bin/glslangValidator.exe -V  skybox.frag -o skybox_frag.spv
C:/VulkanSDK/1.3.231.1/Bin/glslangValidator.exe -V skybox.vert -o skybox_vert.spv
C:/VulkanSDK/1.3.231.1/Bin/glslangValidator.exe -V -o mesh_rock_frag.spv -DSET_NUMBER=1 mesh_rock.frag 
C:/VulkanSDK/1.3.231.1/Bin/glslangValidator.exe -V -o mesh_rock_vert.spv -DSET_NUMBER=1 mesh_rock.vert
C:/VulkanSDK/1.3.231.1/Bin/glslangValidator.exe -V  pre_compute_ibl.comp -o pre_compute_ibl_comp.spv
C:/VulkanSDK/1.3.231.1/Bin/glslangValidator.exe -V default_prefabs_mesh.frag -o default_prefabs_mesh_frag.spv
C:/VulkanSDK/1.3.231.1/Bin/glslangValidator.exe -V default_prefabs_mesh.vert -o default_prefabs_mesh_vert.spv
C:/VulkanSDK/1.3.231.1/Bin/glslangValidator.exe -V pbr_mesh.frag -o pbr_mesh_frag.spv
C:/VulkanSDK/1.3.231.1/Bin/glslangValidator.exe -V pbr_mesh.vert -o pbr_mesh_vert.spv
pause
