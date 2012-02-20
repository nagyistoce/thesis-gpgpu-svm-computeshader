#include "stdafx.h"
#include "DX11SVM.h"
#include "CrossValidation.h"
#include "ConfigManager.h"
#include "GUIManager.h"

namespace SVM_Framework{
	DX11SVM::DX11SVM(GraphicsManagerPtr dxMgr):m_shaderTraining(NULL),m_shaderTesting(NULL){
		m_dxMgr = boost::static_pointer_cast<DirectXManager>(dxMgr);
		if(m_dxMgr->createComputeShader("SMO_Train.hlsl",&m_shaderTraining) < 0){
			assert(0);
		}
		if(m_dxMgr->createComputeShader("SMO_Test.hlsl",&m_shaderTesting) < 0){
			assert(0);
		}
	}

	DX11SVM::~DX11SVM(){
		if(m_shaderTraining)
			m_shaderTraining->Release();
		if(m_shaderTesting)
			m_shaderTesting->Release();
		for(unsigned int i=0; i<m_accessViews.size(); i++){
			m_accessViews[i]->Release();
		}
		for(unsigned int i=0; i<m_resourceViews.size(); i++){
			m_resourceViews[i]->Release();
		}
		for(unsigned int i=0; i<m_buffers.size(); i++){
			m_buffers[i]->Release();
		}
		for(unsigned int i=0; i<m_constantBuffers.size(); i++){
			m_constantBuffers[i]->Release();
		}
	}

	void DX11SVM::execute(){
		std::wstringstream outputStream;

		// Create gfx resources
		ID3D11UnorderedAccessView *uav;
		ID3D11ShaderResourceView *srv;
		ID3D11Buffer *buff,*readBuffer;

		// Input buffer
		m_dxMgr->createStructuredBufferSRV<Value>(0,m_document->m_data.size(),&buff,&srv,&m_document->m_data[0]);
		m_resourceViews.push_back(srv);
		m_buffers.push_back(buff);

		// Constant buffer
		SharedBuffer sharedBuffer;
		sharedBuffer.instanceLength = m_document->getNumAttributes()+1;
		sharedBuffer.classIndex = m_document->m_classAttributeId;
		sharedBuffer.kernelParam1 = 1.0;
		sharedBuffer.kernelParam2 = 1.0;
		sharedBuffer.instanceCount = 0;

		m_dxMgr->createConstantBuffer<SharedBuffer>(&buff,&sharedBuffer);
		m_constantBuffers.push_back(buff);

		// Evaluation method
		CrossValidationPtr evaluation = CrossValidationPtr(new CrossValidation(10));
		evaluation->setData(m_document,m_data);
		
		while(evaluation->advance()){
			// Create fold specific buffers

			// Output buffer
			m_dxMgr->createStructuredBufferUAV<Value>(0,evaluation->getNumTrainingInstances(),&buff,&uav,NULL);
			m_accessViews.push_back(uav);
			m_buffers.push_back(buff);
			// Ind buffer
			m_dxMgr->createStructuredBufferSRV<unsigned int>(0,evaluation->getNumTrainingInstances(),&buff,&srv,&evaluation->getTrainingInds()[0]);
			m_resourceViews.push_back(srv);
			m_buffers.push_back(buff);

			// Set resources
			m_dxMgr->setComputeShaderUnorderedAccessViews(m_accessViews);
			m_dxMgr->setComputeShaderResourceViews(m_resourceViews);
			m_dxMgr->setComputeShaderConstantBuffers(m_constantBuffers);
			m_dxMgr->setComputeShader(m_shaderTraining);

			sharedBuffer.instanceCount = evaluation->getNumTrainingInstances();
			std::vector<SharedBuffer> buff;
			buff.push_back(sharedBuffer);
			m_dxMgr->copyToGraphicsCard(m_constantBuffers[0],buff);

			// Launch computations
			unsigned int numInst = evaluation->getNumTrainingInstances();
			unsigned int numGroups = ceilf(float(numInst)/32.0f);
			std::vector<Value> result;
			result.assign(numInst,0);

			unsigned int timer = ConfigManager::startTimer();
			m_dxMgr->launchComputation(numGroups,1,1);
			m_dxMgr->copyFromGraphicsCard(m_buffers[1],result);
			double trainingTime = ConfigManager::getTime(timer);
			ConfigManager::removeTimer(timer);

			outputStream << "Time: " << trainingTime << "\r\n";
			m_data->m_gui->setText(IDC_STATIC_INFOTEXT,outputStream.str());

			// Clean up
			m_accessViews[0]->Release();
			m_accessViews.clear();
			m_resourceViews[1]->Release();
			m_resourceViews.pop_back();
			m_buffers[1]->Release();
			m_buffers[2]->Release();
			m_buffers.pop_back();
			m_buffers.pop_back();

			// Test test set
			//m_dxMgr->setComputeShader(m_shaderTesting);
		}

		// Clean up
		for(unsigned int i=0; i<m_accessViews.size(); i++){
			m_accessViews[i]->Release();
		}
		m_accessViews.clear();
		for(unsigned int i=0; i<m_resourceViews.size(); i++){
			m_resourceViews[i]->Release();
		}
		m_resourceViews.clear();
		for(unsigned int i=0; i<m_buffers.size(); i++){
			m_buffers[i]->Release();
		}
		m_buffers.clear();
		for(unsigned int i=0; i<m_constantBuffers.size(); i++){
			m_constantBuffers[i]->Release();
		}
		m_constantBuffers.clear();
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