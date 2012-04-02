#include "stdafx.h"
#include "DX11SVM.h"
#include "CrossValidation.h"
#include "PercentageSplit.h"
#include "ConfigManager.h"
#include "GUIManager.h"

#define thread_group_size 64

namespace SVM_Framework{
	DX11SVM::DX11SVM(GraphicsManagerPtr dxMgr){
		m_dxMgr = boost::static_pointer_cast<DirectXManager>(dxMgr);

		m_shaders.assign(m_dxMgr->getNumDevices(),std::map<int,ID3D11ComputeShader*>());

		ID3D11ComputeShader* shader;
		for(unsigned int i=0; i<m_dxMgr->getNumDevices(); i++){
			if(m_dxMgr->createComputeShader(i,"SMO_SelfProd.hlsl",&shader) < 0){
				assert(0);
			}
			m_shaders[i][GS_SelfProd] = shader;
			if(m_dxMgr->createComputeShader(i,"SMO_Test.hlsl",&shader) < 0){
				assert(0);
			}
			m_shaders[i][GS_Testing] = shader;
			if(m_dxMgr->createComputeShader(i,"SMO_UpdateErrorCache.hlsl",&shader) < 0){
				assert(0);
			}
			m_shaders[i][GS_UpdateErrorCache] = shader;
			if(m_dxMgr->createComputeShader(i,"SMO_SVMOutput.hlsl",&shader) < 0){
				assert(0);
			}
			m_shaders[i][GS_SVMOutput] = shader;
		}

		m_inds.assign(m_dxMgr->getNumDevices(),std::vector<unsigned int>());
		m_copyBuffer.assign(m_dxMgr->getNumDevices(),std::vector<Value::v_precision>());
	}

	DX11SVM::~DX11SVM(){
		for(unsigned int i=0; i<m_dxMgr->getNumDevices(); i++){
			std::map<int,ID3D11ComputeShader*>::iterator gs_itr = m_shaders[i].begin();
			while(gs_itr != m_shaders[i].end()){
				gs_itr->second->Release();
				gs_itr++;
			}
			m_shaders[i].clear();
		}
		m_shaders.clear();

		cleanGPUResources();
	}

	void DX11SVM::beginExecute(){
		cleanGPUResources();
		initGPUResources();
	}

	void DX11SVM::endExecute(){
		// Clean up graphics resources
		cleanGPUResources();
	}

	void DX11SVM::lagrangeThresholdUpdate(svm_precision p1, svm_precision p2, int id, int i1, int i2){
		//std::vector<Value::v_precision> checkVal;
		
		for(unsigned int i=0; i<m_dxMgr->getNumDevices(); i++){
			m_dxMgr->setComputeShader(i,m_shaders[i][GS_UpdateErrorCache]);
		}

		// Divide between devices
		int distrib = 0;
		Value::v_precision cachedRes = 0;
		for(unsigned int i=0; i<m_dxMgr->getNumDevices(); i++){
			m_inds[i].clear();
		}
		for(int j = m_params[id].m_I0->getNext(-1); j != -1; j = m_params[id].m_I0->getNext(j)){
			/*checkVal.push_back(m_params[id].m_errors[j]);
			checkVal.back() +=	p1 * m_params[id].m_kernel->eval(i1, j, m_params[id].m_evaluation->getTrainingInstance(i1)) + 
								p2 * m_params[id].m_kernel->eval(i2, j, m_params[id].m_evaluation->getTrainingInstance(i2));*/

			if(m_params[id].m_kernel->isCached(i1,j,cachedRes)){
				svm_precision result = p1 * cachedRes;
				if(m_params[id].m_kernel->isCached(i2,j,cachedRes)){
					m_params[id].m_errors[j] += result + p2 * cachedRes;
				}
				else
					m_inds[distrib++].push_back(j);
			}
			else if(m_params[id].m_kernel->isCached(i2,j,cachedRes)){
				svm_precision result = p2 * cachedRes;
				if(m_params[id].m_kernel->isCached(i1,j,cachedRes)){
					m_params[id].m_errors[j] += result + p1 * cachedRes;
				}
				else
					m_inds[distrib++].push_back(j);
			}
			else
				m_inds[distrib++].push_back(j);
			if(distrib >= m_inds.size())
				distrib = 0;
		}

		// Run kernel evals on GPU
		m_sharedBuffer[0].param1 = p1;
		m_sharedBuffer[0].param2 = p2;
		m_sharedBuffer[0].cb_ind1 = i1;
		m_sharedBuffer[0].cb_ind2 = i2;
		for(unsigned int i=0; i<m_inds.size(); i++){
			if(!m_inds[i].empty()){
				m_sharedBuffer[0].instanceCount = unsigned int(m_inds[i].size());
				m_dxMgr->copyToGraphicsCard(i,m_constantBuffers[i][CB_Shared],m_sharedBuffer);
				m_dxMgr->copyToGraphicsCard(i,m_buffers[i][GB_InputInds],m_inds[i]);

				m_dxMgr->launchComputation(i,int(svm_precision(m_inds[i].size())/thread_group_size)+1,1,1);
			}
		}

		// Copy back results
		for(unsigned int i=0; i<m_dxMgr->getNumDevices(); i++){
			m_copyBuffer[i].clear();
		}
		for(unsigned int i=0; i<m_inds.size(); i++){
			if(!m_inds[i].empty()){
				m_copyBuffer[i].assign(m_inds[i].size()*2,0);
				m_dxMgr->copyFromGraphicsCard(i,m_buffers[i][GB_OutputBuffer],m_copyBuffer[i]);
			}
		}

		// Update error buffer
		for(unsigned int i=0; i<m_inds.size(); i++){
			if(!m_inds[i].empty()){
				for(unsigned int j=0; j<m_inds[i].size(); j++){
					m_params[id].m_errors[m_inds[i][j]] += ((p1 * m_copyBuffer[i][j*2]) + (p2 * m_copyBuffer[i][(j*2)+1]));

					//m_params[id].m_errors[m_inds[i][(j/2)]] += checkVal[(j/2)];
					/*assert(abs(checkVal[j])-abs(m_params[id].m_errors[m_inds[i][j]]) < 1.0e-7);
					if(abs(checkVal[j])-abs(m_params[id].m_errors[m_inds[i][j]]) < 1.0e-7){
						m_data->m_gui->postDebugMessage(L"SVMOutput failiure!!!!");
					}*/
				}
			}
			m_params[id].m_kernel->insertIntoCache(m_inds[i],m_copyBuffer[i],i1,i2);
		}

		m_params[id].m_bLow = -DBL_MAX; m_params[id].m_bUp = DBL_MAX;
		m_params[id].m_iLow = -1; m_params[id].m_iUp = -1;
		for (int j = m_params[id].m_I0->getNext(-1); j != -1; j = m_params[id].m_I0->getNext(j)) {
			if (m_params[id].m_errors[j] < m_params[id].m_bUp) {
				m_params[id].m_bUp = m_params[id].m_errors[j];
				m_params[id].m_iUp = j;
			}
			if(m_params[id].m_errors[j] > m_params[id].m_bLow) {
				m_params[id].m_bLow = m_params[id].m_errors[j];
				m_params[id].m_iLow = j;
			}
		}
	}

	void DX11SVM::updateErrorCache(svm_precision f, int i, int id){
		m_params[id].m_errors[i] = f;
	}

	ISVM::svm_precision DX11SVM::SVMOutput(int index, InstancePtr inst, int id){
		svm_precision result = 0;
		//svm_precision resultCheck = 0;

		int distrib = 0;
		Value::v_precision cachedRes = 0;
		for(unsigned int i=0; i<m_dxMgr->getNumDevices(); i++){
			m_inds[i].clear();
		}
		for(int i = m_params[id].m_supportVectors->getNext(-1); i != -1; i = m_params[id].m_supportVectors->getNext(i)){
			//resultCheck += m_params[id].m_class[i] * m_params[id].m_alpha[i] * m_params[id].m_kernel->eval(index, i, inst);

			if(m_params[id].m_kernel->isCached(i,index,cachedRes)){
				result += m_params[id].m_class[i] * m_params[id].m_alpha[i] * cachedRes;
			}
			else
				m_inds[distrib++].push_back(i);
			
			if(distrib >= m_inds.size())
				distrib = 0;
		}

		m_sharedBuffer[0].cb_ind1 = index;
		for(unsigned int i=0; i<m_inds.size(); i++){
			if(!m_inds[i].empty()){
				/*if(m_alphaTransferNeeded){
					m_dxMgr->copyToGraphicsCard(i,m_buffers[i][GB_AlphaBuffer],m_params[id].m_alpha);
				}*/
				
				m_sharedBuffer[0].instanceCount = unsigned int(m_inds[i].size());
				m_dxMgr->copyToGraphicsCard(i,m_constantBuffers[i][CB_Shared],m_sharedBuffer);
				m_dxMgr->copyToGraphicsCard(i,m_buffers[i][GB_InputInds],m_inds[i]);

				m_dxMgr->setComputeShader(i,m_shaders[i][GS_SVMOutput]);
				m_dxMgr->launchComputation(i,int(float(m_inds[i].size())/float(thread_group_size))+1,1,1);
			}
		}
		/*if(m_alphaTransferNeeded){
			m_alphaTransferNeeded = false;
		}*/

		for(unsigned int i=0; i<m_dxMgr->getNumDevices(); i++){
			m_copyBuffer[i].clear();
		}
		for(unsigned int i=0; i<m_inds.size(); i++){
			if(!m_inds[i].empty()){
				m_copyBuffer[i].assign(m_inds[i].size(),0);
				m_dxMgr->copyFromGraphicsCard(i,m_buffers[i][GB_OutputBuffer],m_copyBuffer[i]);
			}
			m_params[id].m_kernel->insertIntoCache(m_inds[i],m_copyBuffer[i],index);
		}

		for(unsigned int i=0; i<m_copyBuffer.size(); i++){
			for(unsigned int j=0; j<m_copyBuffer[i].size(); j++){
				result += m_params[id].m_class[m_inds[i][j]] * m_params[id].m_alpha[m_inds[i][j]] * m_copyBuffer[i][j];
			}
		}
		result -= m_params[id].m_b;

		// Unit test code
		/*std::vector<svm_precision> resultsCheck;
		for (int i = 0; i<m_inds.size(); i++) {
			for(unsigned int j=0; j<m_inds[i].size(); j++){
				resultsCheck.push_back(m_params[id].m_class[m_inds[i][j]] * m_params[id].m_alpha[m_inds[i][j]] * m_params[id].m_kernel->eval(index, m_inds[i][j], inst));
				resultCheck += resultsCheck.back();
			}
		}*/
		//resultCheck -= m_params[id].m_b;

		//assert(abs(result)-abs(resultCheck) < 1.0e-7);
		//if((abs(result)-abs(resultCheck)) > 1.0e-7){
		//	m_data->m_gui->postDebugMessage(L"SVMOutput failiure!!!!");
		//}
		
		return result;
	}

	void DX11SVM::testInstances(std::vector<svm_precision> &finalResult, int id){
		for(unsigned int i=0; i<m_dxMgr->getNumDevices(); i++){
			m_dxMgr->copyToGraphicsCard(i,m_buffers[i][GB_AlphaBuffer],m_params[id].m_alpha);
			m_dxMgr->setComputeShader(i,m_shaders[i][GS_Testing]);
		}

		m_inds[0].clear();
		m_inds[0].reserve(m_params[id].m_supportVectors->numElements());
		for (int i = m_params[id].m_supportVectors->getNext(-1); i != -1; i = m_params[id].m_supportVectors->getNext(i)) {
			m_inds[0].push_back(i);
		}
		
		for(unsigned int i=0; i<m_dxMgr->getNumDevices(); i++){
			m_dxMgr->copyToGraphicsCard(i,m_buffers[i][GB_InputInds],m_inds[0]);
		}

		int instances = m_params[id].m_evaluation->getNumTestingInstances();
		std::vector<std::vector<unsigned int>> instanceMapping;
		instanceMapping.assign(m_dxMgr->getNumDevices(),std::vector<unsigned int>());
		int distrib = 0;
		for(unsigned int i=0; i<instances; i++){
			instanceMapping[distrib++].push_back(i);
			if(distrib >= instanceMapping.size())
				distrib = 0;
		}
		
		int max = 0;
		m_sharedBuffer[0].instanceCount = int(m_inds[0].size());
		for(unsigned int i=0; i<m_dxMgr->getNumDevices(); i++){
			if(instanceMapping[i].size() > max)
				max = instanceMapping[i].size();
			m_copyBuffer[i].clear();
			m_copyBuffer[i].assign(m_sharedBuffer[0].instanceCount,0);
		}
		finalResult.assign(instances,0);
		for(unsigned int i=0; i<max; i++){
			for(unsigned int j=0; j<instanceMapping.size(); j++){
				if(i < instanceMapping[j].size()){
					m_sharedBuffer[0].cb_ind1 = instanceMapping[j][i];
					m_dxMgr->copyToGraphicsCard(j,m_constantBuffers[j][CB_Shared],m_sharedBuffer);
					m_dxMgr->launchComputation(j,(m_sharedBuffer[0].instanceCount/thread_group_size)+1,1,1);
				}
			}

			for(unsigned int j=0; j<instanceMapping.size(); j++){
				if(i != 0 && i <= instanceMapping[j].size()){
					for(unsigned int k=0; k<m_copyBuffer[j].size(); k++){
						finalResult[instanceMapping[j][i-1]] += m_copyBuffer[j][k];
					}
					finalResult[instanceMapping[j][i-1]] -= m_params[id].m_b;
				}
			}

			for(unsigned int j=0; j<instanceMapping.size(); j++){
				if(i < instanceMapping[j].size()){
					m_dxMgr->copyFromGraphicsCard(j,m_buffers[j][GB_OutputBuffer],m_copyBuffer[j]);
				}
			}

			for(unsigned int j=0; j<instanceMapping.size(); j++){
				if(i == max-1 && i < instanceMapping[j].size()){
					for(unsigned int k=0; k<m_copyBuffer[j].size(); k++){
						finalResult[instanceMapping[j][i]] += m_copyBuffer[j][k];
					}
					finalResult[instanceMapping[j][i]] -= m_params[id].m_b;
				}
			}
		}
	}

	void DX11SVM::kernelEvaluations(std::vector<int> &inds, std::vector<svm_precision> &result, int id){
		if(inds.size() < 2)
			return;

		for(unsigned int i=0; i<inds.size(); i+=2){
			result.push_back(m_params[id].m_kernel->eval(inds[i], inds[i+1], m_params[id].m_evaluation->getTrainingInstance(inds[i])));
		}
	}

	void DX11SVM::initGPUResources(){
		// Create gfx resources
		ID3D11UnorderedAccessView *uav;
		ID3D11ShaderResourceView *srv;
		ID3D11Buffer *buff;

		m_resourceViews.assign(m_dxMgr->getNumDevices(),std::map<int,ID3D11ShaderResourceView*>());
		m_accessViews.assign(m_dxMgr->getNumDevices(),std::map<int,ID3D11UnorderedAccessView*>());
		m_buffers.assign(m_dxMgr->getNumDevices(),std::map<int,ID3D11Buffer*>());
		m_constantBuffers.assign(m_dxMgr->getNumDevices(),std::map<int,ID3D11Buffer*>());

		// Input buffer
		for(unsigned int i=0; i<m_dxMgr->getNumDevices(); i++){
			m_dxMgr->createStructuredBufferSRV<Value>(i,D3D11_USAGE_DEFAULT,0,UINT(m_document->m_data.size()),&buff,&srv,&m_document->m_data[0]);
			m_resourceViews[i][SRV_Data] = srv;
			m_buffers[i][GB_DataBuffer] = buff;

			// Self product
			m_dxMgr->createStructuredBufferUAV<svm_precision>(i,D3D11_USAGE_DEFAULT,0,m_document->getNumInstances(),&buff,&uav,NULL);
			m_accessViews[i][UAV_SelfProd] = uav;
			m_buffers[i][GB_SelfProdBuffer] = buff;
		}

		// Constant buffer
		m_sharedBuffer[0].instanceCount = m_document->getNumInstances();
		m_sharedBuffer[0].instanceLength = m_document->getNumAttributes()+1;
		m_sharedBuffer[0].classIndex = m_document->m_classAttributeId;

		for(unsigned int i=0; i<m_dxMgr->getNumDevices(); i++){
			m_dxMgr->createConstantBuffer<SharedBuffer>(i,&buff,&m_sharedBuffer[0]);
			m_constantBuffers[i][CB_Shared] = buff;
		}

		// Compute self product
		for(unsigned int i=0; i<m_dxMgr->getNumDevices(); i++){
			m_dxMgr->setComputeShader(i,m_shaders[i][GS_SelfProd]);
			m_dxMgr->setComputeShaderConstantBuffers(i,m_constantBuffers[i]);
			m_dxMgr->setComputeShaderResourceViews(i,m_resourceViews[i]);
			m_dxMgr->setComputeShaderUnorderedAccessViews(i,m_accessViews[i]);

			m_dxMgr->launchComputation(i,(m_document->getNumInstances()/thread_group_size)+1,1,1);
		}
	}

	void DX11SVM::cleanGPUResources(){
		for(unsigned int i=0; i<m_accessViews.size(); i++){
			std::map<int,ID3D11UnorderedAccessView*>::iterator uav_itr = m_accessViews[i].begin();
			while(uav_itr != m_accessViews[i].end()){
				uav_itr->second->Release();
				uav_itr++;
			}
			m_accessViews[i].clear();
		}
		m_accessViews.clear();

		for(unsigned int i=0; i<m_resourceViews.size(); i++){
			std::map<int,ID3D11ShaderResourceView*>::iterator srv_itr = m_resourceViews[i].begin();
			while(srv_itr != m_resourceViews[i].end()){
				srv_itr->second->Release();
				srv_itr++;
			}
			m_resourceViews[i].clear();
		}
		m_resourceViews.clear();

		for(unsigned int i=0; i<m_buffers.size(); i++){
			std::map<int,ID3D11Buffer*>::iterator gb_itr = m_buffers[i].begin();
			while(gb_itr != m_buffers[i].end()){
				gb_itr->second->Release();
				gb_itr++;
			}
			m_buffers[i].clear();
		}
		m_buffers.clear();

		for(unsigned int i=0; i<m_constantBuffers.size(); i++){
			std::map<int,ID3D11Buffer*>::iterator cb_itr = m_constantBuffers[i].begin();
			while(cb_itr != m_constantBuffers[i].end()){
				cb_itr->second->Release();
				cb_itr++;
			}
			m_constantBuffers[i].clear();
		}
		m_constantBuffers.clear();
	}

	void DX11SVM::initializeFold(int id){
		// Create gfx resources
		ID3D11UnorderedAccessView *uav;
		ID3D11ShaderResourceView *srv;
		ID3D11Buffer *buff;

		for(unsigned int i=0; i<m_dxMgr->getNumDevices(); i++){
			// Kernel inds
			m_dxMgr->createStructuredBufferSRV<unsigned int>(i,D3D11_USAGE_DYNAMIC,D3D11_CPU_ACCESS_WRITE,m_params[id].m_evaluation->getNumTrainingInstances(),&buff,&srv,NULL);
			m_resourceViews[i][SRV_InputInds] = srv;
			m_buffers[i][GB_InputInds] = buff;

			// Training instance inds
			m_dxMgr->createStructuredBufferSRV<unsigned int>(i,D3D11_USAGE_DEFAULT,0,m_params[id].m_evaluation->getNumTrainingInstances(),&buff,&srv,&m_params[id].m_evaluation->getTrainingInds()[0]);
			m_resourceViews[i][SRV_TrainingIndices] = srv;
			m_buffers[i][GB_TrainingIndices] = buff;

			// Testing instance inds
			m_dxMgr->createStructuredBufferSRV<unsigned int>(i,D3D11_USAGE_DEFAULT,0,m_params[id].m_evaluation->getNumTestingInstances(),&buff,&srv,&m_params[id].m_evaluation->getTestingInds()[0]);
			m_resourceViews[i][SRV_TestingIndices] = srv;
			m_buffers[i][GB_TestingIndices] = buff;

			// Class
			m_dxMgr->createStructuredBufferSRV<svm_precision>(i,D3D11_USAGE_DEFAULT,0,m_params[id].m_evaluation->getNumTrainingInstances(),&buff,&srv,&m_params[id].m_class[0]);
			m_resourceViews[i][SRV_Class] = srv;
			m_buffers[i][GB_ClassBuffer] = buff;

			// Alpha
			m_dxMgr->createStructuredBufferSRV<svm_precision>(i,D3D11_USAGE_DYNAMIC,D3D11_CPU_ACCESS_WRITE,m_params[id].m_evaluation->getNumTrainingInstances(),&buff,&srv,&m_params[id].m_alpha[0]);
			m_resourceViews[i][SRV_Alpha] = srv;
			m_buffers[i][GB_AlphaBuffer] = buff;

			// Output
			m_dxMgr->createStructuredBufferUAV<svm_precision>(i,D3D11_USAGE_DEFAULT,0,m_params[id].m_evaluation->getNumTrainingInstances()*2,&buff,&uav,NULL);
			m_accessViews[i][UAV_Output] = uav;
			m_buffers[i][GB_OutputBuffer] = buff;
		}

		m_sharedBuffer[0].instanceCount = m_params[id].m_evaluation->getNumTrainingInstances();
		m_sharedBuffer[0].kernelParam1 = m_params[id].m_kernel->getParameter(0);
		m_sharedBuffer[0].kernelParam2 = m_params[id].m_kernel->getParameter(1);
		m_sharedBuffer[0].kernel = m_params[id].m_kernel->getId();

		for(unsigned int i=0; i<m_dxMgr->getNumDevices(); i++){
			m_dxMgr->copyToGraphicsCard(i,m_constantBuffers[i][CB_Shared],m_sharedBuffer);

			m_dxMgr->setComputeShader(i,m_shaders[i][GS_UpdateErrorCache]);
			m_dxMgr->setComputeShaderConstantBuffers(i,m_constantBuffers[i]);
			m_dxMgr->setComputeShaderResourceViews(i,m_resourceViews[i]);
			m_dxMgr->setComputeShaderUnorderedAccessViews(i,m_accessViews[i]);
		}
	}

	void DX11SVM::endFold(int id){
		for(unsigned int i=0; i<m_dxMgr->getNumDevices(); i++){
			m_accessViews[i][UAV_Output]->Release();

			m_accessViews[i].erase(UAV_Output);

			m_resourceViews[i][SRV_Alpha]->Release();
			m_resourceViews[i][SRV_Class]->Release();
			m_resourceViews[i][SRV_InputInds]->Release();
			m_resourceViews[i][SRV_TestingIndices]->Release();
			m_resourceViews[i][SRV_TrainingIndices]->Release();

			m_resourceViews[i].erase(SRV_Alpha);
			m_resourceViews[i].erase(SRV_Class);
			m_resourceViews[i].erase(SRV_InputInds);
			m_resourceViews[i].erase(SRV_TrainingIndices);
			m_resourceViews[i].erase(SRV_TestingIndices);

			m_buffers[i][GB_OutputBuffer]->Release();
			m_buffers[i][GB_AlphaBuffer]->Release();
			m_buffers[i][GB_ClassBuffer]->Release();
			m_buffers[i][GB_TestingIndices]->Release();
			m_buffers[i][GB_TrainingIndices]->Release();
			m_buffers[i][GB_InputInds]->Release();

			m_buffers[i].erase(GB_OutputBuffer);
			m_buffers[i].erase(GB_AlphaBuffer);
			m_buffers[i].erase(GB_ClassBuffer);
			m_buffers[i].erase(GB_TestingIndices);
			m_buffers[i].erase(GB_TrainingIndices);
			m_buffers[i].erase(GB_InputInds);
		}
	}
}