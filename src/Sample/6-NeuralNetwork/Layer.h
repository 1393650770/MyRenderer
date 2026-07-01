#pragma once
#include "Tensor.h"
#include "RHI/RenderCommandList.h"
#include "RHI/RenderPipelineState.h"

namespace MXNN {

using namespace MXRender::RHI;

void WriteTensor(std::ostream& os, const Tensor& t);
void ReadTensor(std::istream& is, Tensor& t);

// Param buffer layouts (must match shader binding 10)
struct FwdParams  { Float32 in_dim, out_dim, max_batch_size, active_batch_size, has_relu; };
struct LossParams { Float32 in_dim, out_dim, max_batch_size, active_batch_size; };
struct BwdParams  { Float32 in_dim, out_dim, max_batch_size, active_batch_size, has_relu; };
struct SgdParams  { Float32 lr, momentum, wd, inv_bs, n_elem; };
struct ZeroParams { Float32 num_elements; };

struct IOptimizer;

MYRENDERER_BEGIN_CLASS(ILayer)
#pragma region MATHOD
public:
	ILayer() MYDEFAULT;
	VIRTUAL ~ILayer() MYDEFAULT;

	VIRTUAL void METHOD(Forward)(CommandList* in_cmd, Tensor& in_input) PURE;
	VIRTUAL void METHOD(Backward)(CommandList* in_cmd, CONST Tensor& in_dL_dout, CONST Tensor& in_input_act) PURE;
	VIRTUAL void METHOD(ZeroGradients)(CommandList* in_cmd) PURE;
	VIRTUAL void METHOD(UpdateWeights)(CommandList* in_cmd, IOptimizer& in_opt, Float32 in_inv_bs, UInt32 in_step) PURE;

	VIRTUAL Tensor& METHOD(GetOutput)() PURE;
	VIRTUAL Tensor& METHOD(GetInputGradient)() PURE;
	VIRTUAL Vector<std::tuple<Tensor*, Tensor*, Tensor*>> METHOD(GetParamTriples)() PURE;
		VIRTUAL Vector<std::tuple<Tensor*, Tensor*, Tensor*, Tensor*>> METHOD(GetParamQuads)() { return {}; }
		VIRTUAL void METHOD(SetTrainingMode)(Bool in_training) {}
		VIRTUAL String METHOD(GetLayerTypeName)() CONST { return ""; }
		VIRTUAL void METHOD(SaveParameters)(std::ostream& os) CONST {}
		VIRTUAL void METHOD(LoadParameters)(std::istream& is) {}
		VIRTUAL void METHOD(ClearTempSRBs)() {}
	protected:
private:
#pragma endregion

#pragma region MEMBER
public:
protected:
private:
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(LinearLayer, public MXNN::ILayer)
#pragma region MATHOD
public:
	LinearLayer(UInt32 in_in_dim, UInt32 in_out_dim, UInt32 in_max_batch_size, Bool in_has_relu);
	VIRTUAL ~LinearLayer() MYDEFAULT;

	VIRTUAL void Forward(CommandList* in_cmd, Tensor& in_input) OVERRIDE FINAL;
	VIRTUAL void Backward(CommandList* in_cmd, CONST Tensor& in_dL_dout, CONST Tensor& in_input_act) OVERRIDE FINAL;
	VIRTUAL void ZeroGradients(CommandList* in_cmd) OVERRIDE FINAL;
	VIRTUAL void UpdateWeights(CommandList* in_cmd, IOptimizer& in_opt, Float32 in_inv_bs, UInt32 in_step) OVERRIDE FINAL;

	VIRTUAL Tensor& GetOutput() OVERRIDE FINAL { return output_; }
	VIRTUAL Tensor& GetInputGradient() OVERRIDE FINAL { return dL_dx_; }
	VIRTUAL Vector<std::tuple<Tensor*, Tensor*, Tensor*>> GetParamTriples() OVERRIDE FINAL;
		VIRTUAL String GetLayerTypeName() CONST OVERRIDE FINAL;
		VIRTUAL void SaveParameters(std::ostream& os) CONST OVERRIDE FINAL;
		VIRTUAL void LoadParameters(std::istream& is) OVERRIDE FINAL;
	protected:
private:
	void CreatePipelineAndSRB();
#pragma endregion

#pragma region MEMBER
public:
protected:
private:
	UInt32 in_dim_, out_dim_, max_batch_size_;
	Bool has_relu_;
	Tensor weight_, bias_, grad_w_, grad_b_, v_w_, v_b_;
	Tensor z_preact_, output_, dL_dx_;
	Tensor pc_buf_; // params for binding 10
	RenderPipelineState* fwd_pipeline_ = nullptr;
	RenderPipelineState* bwd_pipeline_ = nullptr;
	ShaderResourceBinding* fwd_srb_ = nullptr;
	ShaderResourceBinding* bwd_srb_ = nullptr;
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(SoftmaxCrossEntropyOutputLayer, public MXNN::ILayer)
#pragma region MATHOD
public:
	SoftmaxCrossEntropyOutputLayer(UInt32 in_in_dim, UInt32 in_num_classes, UInt32 in_max_batch_size);
	VIRTUAL ~SoftmaxCrossEntropyOutputLayer() MYDEFAULT;

	VIRTUAL void Forward(CommandList* in_cmd, Tensor& in_hidden_act) OVERRIDE FINAL;
	VIRTUAL void Backward(CommandList* in_cmd, CONST Tensor& in_dL_dout, CONST Tensor& in_input_act) OVERRIDE FINAL;
	VIRTUAL void ZeroGradients(CommandList* in_cmd) OVERRIDE FINAL;
	VIRTUAL void UpdateWeights(CommandList* in_cmd, IOptimizer& in_opt, Float32 in_inv_bs, UInt32 in_step) OVERRIDE FINAL;

	VIRTUAL Tensor& GetOutput() OVERRIDE FINAL { return dL_dz_; }
	VIRTUAL Tensor& GetInputGradient() OVERRIDE FINAL { return dL_dhidden_; }
	VIRTUAL Vector<std::tuple<Tensor*, Tensor*, Tensor*>> GetParamTriples() OVERRIDE FINAL;

	VIRTUAL String GetLayerTypeName() CONST OVERRIDE FINAL;
	VIRTUAL void SaveParameters(std::ostream& os) CONST OVERRIDE FINAL;
	VIRTUAL void LoadParameters(std::istream& is) OVERRIDE FINAL;

	Float32 GetLoss() CONST;
	void ZeroLossBuffer() { loss_buf_.Zero(); }
	Buffer* GetLossBuffer() CONST { return loss_buf_.GetBuffer(); }
	UInt32 NumClasses() CONST { return num_classes_; }
	ShaderResourceBinding* GetFwdLossSRB() { return fwd_loss_srb_; }
protected:
private:
#pragma endregion

#pragma region MEMBER
public:
protected:
private:
	UInt32 in_dim_, num_classes_, max_batch_size_;
	Tensor weight_, bias_, grad_w_, grad_b_, v_w_, v_b_;
	Tensor dL_dz_, loss_buf_, dL_dhidden_;
	Tensor pc_buf_; // params for binding 10
	RenderPipelineState* fwd_loss_pipeline_ = nullptr;
	RenderPipelineState* bwd_pipeline_ = nullptr;
	ShaderResourceBinding* fwd_loss_srb_ = nullptr;
	ShaderResourceBinding* bwd_srb_ = nullptr;
#pragma endregion
MYRENDERER_END_CLASS

} // namespace MXNN
