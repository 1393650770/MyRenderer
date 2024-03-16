glslangValidator.exe -V  test.vert -o vert.spv
glslangValidator.exe -V  test.frag -o frag.spv
glslangValidator.exe -V  skybox.frag -o skybox_frag.spv
glslangValidator.exe -V skybox.vert -o skybox_vert.spv
glslangValidator.exe -V -o mesh_rock_frag.spv -DSET_NUMBER=1 mesh_rock.frag 
glslangValidator.exe -V -o mesh_rock_vert.spv -DSET_NUMBER=1 mesh_rock.vert
glslangValidator.exe -V  pre_compute_ibl.comp -o pre_compute_ibl_comp.spv
glslangValidator.exe -V default_prefabs_mesh.frag -o default_prefabs_mesh_frag.spv
glslangValidator.exe -V default_prefabs_mesh.vert -o default_prefabs_mesh_vert.spv
glslangValidator.exe -V pbr_mesh.frag -o pbr_mesh_frag.spv
glslangValidator.exe -V pbr_mesh.vert -o pbr_mesh_vert.spv
glslangValidator.exe -V gpu_driven_tri.vert -o gpu_driven_tri_vert.spv
glslangValidator.exe -V upload.comp -o upload_comp.spv
glslangValidator.exe -V gpu_culling.comp -o gpu_culling_comp.spv
glslangValidator.exe -V depth_reduce.comp -o depth_reduce_comp.spv
glslangValidator.exe -V mesh_rock_transparency.frag -o mesh_rock_transparency_frag.spv
glslangValidator.exe -V fullscreen.vert -o fullscreen_vert.spv
glslangValidator.exe -V copy.frag -o copy_frag.spv
glslangValidator.exe -V mesh_rock_copy.vert -o mesh_rock_copy_vert.spv
glslangValidator.exe -V default_prefabs_mesh_copy.frag -o default_prefabs_mesh_copy_frag.spv

glslangValidator.exe -V Sample/texture_test.frag -o Sample/texture_test_frag.spv
glslangValidator.exe -V Sample/texture_test.vert -o Sample/texture_test_vert.spv
glslangValidator.exe -V Sample/triangle_test.frag -o Sample/triangle_test_frag.spv
glslangValidator.exe -V Sample/triangle_test.vert -o Sample/triangle_test_vert.spv
glslangValidator.exe -V Sample/skybox_test.frag -o Sample/skybox_test_frag.spv
glslangValidator.exe -V Sample/skybox_test.vert -o Sample/skybox_test_vert.spv
pause
