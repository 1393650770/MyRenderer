#pragma once
#ifndef _VK_MESH_
#define _VK_MESH_
#include <vector>
#include <string>


namespace MXRender
{
    struct SimpleVertex;
    class MeshBase
    {
    private:
    protected:

    public:
        std::vector<SimpleVertex> vertices;
        std::vector<uint32_t> indices;
        void load_model(const std::string& filename);
    };

}
#endif 
