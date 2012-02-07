#include "stdafx.h"
#include "DX11SVM.h"

namespace SVM_Framework{
	DX11SVM::DX11SVM(GraphicsManagerPtr dxMgr):m_shader(NULL){
		m_dxMgr = boost::static_pointer_cast<DirectXManager>(dxMgr);
		if(m_dxMgr->createComputeShader("Kernel.hlsl",&m_shader) < 0){
			assert(0);
		}
	}

	DX11SVM::~DX11SVM(){
		if(m_shader)
			m_shader->Release();
		for(unsigned int i=0; i<m_accessViews.size(); i++){
			m_accessViews[i]->Release();
		}
		for(unsigned int i=0; i<m_uavBuffers.size(); i++){
			m_uavBuffers[i]->Release();
		}
		for(unsigned int i=0; i<m_constantBuffers.size(); i++){
			m_constantBuffers[i]->Release();
		}
	}

	void DX11SVM::execute(){
		// Create gfx resources
		ID3D11UnorderedAccessView* uav;
		ID3D11Buffer* buff;
		m_dxMgr->createStructuredBuffer<Value>(m_document->m_data.size(),&buff,&uav,&m_document->m_data[0]);
		m_accessViews.push_back(uav);
		m_uavBuffers.push_back(buff);

		// Set resources
		m_dxMgr->setComputeShaderUnorderedAccessViews(m_accessViews);
		m_dxMgr->setComputeShader(m_shader);
	}

	int DX11SVM::examineExample(int i2){
		return 0;
	}

	int DX11SVM::takeStep(int i1, int i2, double F2){
		return 0;
	}

	double DX11SVM::SVMOutput(int index, InstancePtr inst){
		return 0;
	}
}