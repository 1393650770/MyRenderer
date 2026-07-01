#pragma once
#ifndef _NN_RESIDUALBLOCK_
#define _NN_RESIDUALBLOCK_
#include "Layer.h"

namespace MXNN {

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(ResidualBlock, public ILayer)
#pragma region MATHOD
public:
    explicit ResidualBlock(UInt32 in_n_elem);
    VIRTUAL ~ResidualBlock();
    VIRTUAL void METHOD(Forward)(CommandList*, Tensor& in_input) OVERRIDE FINAL;
    VIRTUAL void METHOD(Backward)(CommandList*, CONST Tensor& in_dL_dout, CONST Tensor& in_input_act) OVERRIDE FINAL;
    VIRTUAL void METHOD(ZeroGradients)(CommandList*) OVERRIDE FINAL;
    VIRTUAL void METHOD(UpdateWeights)(CommandList*, IOptimizer&, Float32, UInt32) OVERRIDE FINAL;
    VIRTUAL Vector<std::tuple<Tensor*,Tensor*,Tensor*>> METHOD(GetParamTriples)() OVERRIDE FINAL;
    VIRTUAL String METHOD(GetLayerTypeName)() CONST OVERRIDE FINAL { return "ResidualBlock"; }
    VIRTUAL void METHOD(SaveParameters)(std::ostream& os) CONST OVERRIDE FINAL;
    VIRTUAL void METHOD(LoadParameters)(std::istream& is) OVERRIDE FINAL;
    VIRTUAL Tensor& METHOD(GetOutput)() OVERRIDE FINAL { return output_; }
    VIRTUAL Tensor& METHOD(GetInputGradient)() OVERRIDE FINAL { return dL_dx_; }
    void AddSubLayer(UniquePtr<ILayer> in_layer);
protected:
    UInt32 n_elem_;
    Vector<UniquePtr<ILayer>> sub_layers_;
    Tensor output_, dL_dx_;
    Tensor pc_buf_{{1}};
    RenderPipelineState* add_pipeline_ = nullptr;
    Vector<ShaderResourceBinding*> temp_srbs_;
#pragma endregion
MYRENDERER_END_CLASS

} // namespace MXNN
#endif
