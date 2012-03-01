#include "stdafx.h"
#include "DX11SVM.h"
#include "CrossValidation.h"
#include "PercentageSplit.h"
#include "ConfigManager.h"
#include "GUIManager.h"

namespace SVM_Framework{
	DX11SVM::DX11SVM(GraphicsManagerPtr dxMgr){
		m_dxMgr = boost::static_pointer_cast<DirectXManager>(dxMgr);

		ID3D11ComputeShader* shader;
		if(m_dxMgr->createComputeShader("SMO_SelfProd.hlsl",&shader) < 0){
			assert(0);
		}
		m_shaders[GS_SelfProd] = shader;
		if(m_dxMgr->createComputeShader("SMO_Test.hlsl",&shader) < 0){
			assert(0);
		}
		m_shaders[GS_Testing] = shader;
		if(m_dxMgr->createComputeShader("SMO_UpdateErrorCache.hlsl",&shader) < 0){
			assert(0);
		}
		m_shaders[GS_UpdateErrorCache] = shader;
		if(m_dxMgr->createComputeShader("SMO_SVMOutput.hlsl",&shader) < 0){
			assert(0);
		}
		m_shaders[GS_SVMOutput] = shader;
		if(m_dxMgr->createComputeShader("SMO_UpdateError.hlsl",&shader) < 0){
			assert(0);
		}
		m_shaders[GS_ErrorUpdate] = shader;
		if(m_dxMgr->createComputeShader("SMO_FindBI.hlsl",&shader) < 0){
			assert(0);
		}
		m_shaders[GS_FindBI] = shader;
		if(m_dxMgr->createComputeShader("SMO_KernelEvaluations.hlsl",&shader) < 0){
			assert(0);
		}
		m_shaders[GS_KernelEvaluations] = shader;
		m_sharedBuffer.push_back(SharedBuffer());
	}

	DX11SVM::~DX11SVM(){
		std::map<int,ID3D11ComputeShader*>::iterator gs_itr = m_shaders.begin();
		while(gs_itr != m_shaders.end()){
			gs_itr->second->Release();
			gs_itr++;
		}
		m_shaders.clear();

		cleanGPUResources();
	}

	void DX11SVM::execute(){
		cleanGPUResources();

		initGPUResources();
		unsigned int timerId = ConfigManager::startTimer();

		m_params.assign(1,AlgoParams());

		std::string eval = m_data->m_gui->getEditText(IDC_COMBO_EVALUATION);
		if(eval.compare("CrossValidation") == 0){
			unsigned int folds = 0;
			try{
				folds = boost::lexical_cast<unsigned int>(m_data->m_gui->getEditText(IDC_EDIT_EVALPARAM));
			}catch(...){
				folds = 10;
			}
			m_params[0].m_evaluation = IEvaluationPtr(new CrossValidation(folds));
		}
		else if(eval.compare("PercentageSplit") == 0){
			float percent = 0;
			try{
				percent = boost::lexical_cast<float>(m_data->m_gui->getEditText(IDC_EDIT_EVALPARAM));
			}catch(...){
				percent = 66;
			}
			m_params[0].m_evaluation = IEvaluationPtr(new PercentageSplit(percent));
		}
		else{
			m_params[0].m_evaluation = IEvaluationPtr(new CrossValidation(10));
		}
		m_params[0].m_evaluation->setData(m_document,m_data);
		for(unsigned int i=0; i<m_params[0].m_evaluation->getNumStages(); i++){
			m_params[0].m_evaluation->setStage(i);
			executeStage(0);
			m_resultPack.cl1Correct += m_params[0].cl1Correct;
			m_resultPack.cl2Correct += m_params[0].cl2Correct;
			m_resultPack.cl1Wrong += m_params[0].cl1Wrong;
			m_resultPack.cl2Wrong += m_params[0].cl2Wrong;
			m_resultPack.iterations += m_params[0].iterations;
			m_resultPack.supportVectors += m_params[0].m_supportVectors->numElements();

			m_resultPack.cacheHits += m_params[0].m_kernel->getCacheHits();
			m_resultPack.kernelEvals += m_params[0].m_kernel->getKernelEvals();
			m_params[0].m_kernel->resetCounters();
		}

		m_resultPack.totalTime = ConfigManager::getTime(timerId);
		ConfigManager::removeTimer(timerId);

		// Clean up graphics resources
		cleanGPUResources();
	}

	void DX11SVM::lagrangeThresholdUpdate(double p1, double p2, int id, int i1, int i2){
		m_dxMgr->setComputeShader(m_shaders[GS_UpdateErrorCache]);

		// Collect kernel evaluations
		std::vector<unsigned int> kernelEvals;
		kernelEvals.reserve(m_params[id].m_I0->numElements());
		for(int j = m_params[id].m_I0->getNext(-1); j != -1; j = m_params[id].m_I0->getNext(j)) {
			kernelEvals.push_back(j);
		}

		// Run kernel evals on GPU
		std::vector<algorithmParams> biParams;
		if(!kernelEvals.empty()){
			m_sharedBuffer[0].param1 = p1;
			m_sharedBuffer[0].param2 = p2;
			m_sharedBuffer[0].cb_ind1 = i1;
			m_sharedBuffer[0].cb_ind2 = i2;
			m_sharedBuffer[0].instanceCount = unsigned int(kernelEvals.size());

			// Update error cache
			m_dxMgr->copyToGraphicsCard(m_constantBuffers[CB_Shared],m_sharedBuffer);
			m_dxMgr->copyToGraphicsCard(m_buffers[GB_InputInds],kernelEvals);
			m_dxMgr->launchComputation(int(float(kernelEvals.size())/32.0f)+1,1,1);
			m_dxMgr->copyFromGraphicsCard(m_buffers[GB_ErrorBuffer],m_params[id].m_errors);

			// Find new BI
			m_dxMgr->setComputeShader(m_shaders[GS_FindBI]);
			m_sharedBuffer[0].cb_ind1 = 0;
			m_dxMgr->copyToGraphicsCard(m_constantBuffers[CB_Shared],m_sharedBuffer);
			
			unsigned int groups = (int(float(kernelEvals.size())/32.0f)+1);
			m_dxMgr->launchComputation(groups,1,1);
			m_sharedBuffer[0].cb_ind1 = 1;
			m_sharedBuffer[0].instanceCount = groups;

			while(groups > 1){
				groups = int(float(groups)/32.0f)+1;
				m_dxMgr->copyToGraphicsCard(m_constantBuffers[CB_Shared],m_sharedBuffer);
				m_dxMgr->launchComputation(groups,1,1);

				m_sharedBuffer[0].instanceCount = groups;
			}
			
			biParams.assign(1,algorithmParams());
			m_dxMgr->copyFromGraphicsCard(m_buffers[GB_FindBI],biParams);
		}

		// Update thresholds
		m_params[id].m_bLow = -DBL_MAX; m_params[id].m_bUp = DBL_MAX;
		m_params[id].m_iLow = -1; m_params[id].m_iUp = -1;
		if(!biParams.empty()){
			m_params[id].m_bUp = biParams[0].s_bUp;
			m_params[id].m_bLow = biParams[0].s_bLow;
			m_params[id].m_iUp = biParams[0].s_iUp;
			m_params[id].m_iLow = biParams[0].s_iLow;
		}
	}

	void DX11SVM::updateErrorCache(float f, int i, int id){
		m_sharedBuffer[0].cb_ind1 = i;
		m_sharedBuffer[0].param1 = f;
		m_dxMgr->copyToGraphicsCard(m_constantBuffers[CB_Shared],m_sharedBuffer);
		m_dxMgr->setComputeShader(m_shaders[GS_ErrorUpdate]);
		m_dxMgr->launchComputation(1,1,1);
	}

	double DX11SVM::SVMOutput(int index, InstancePtr inst, int id){
		double result = 0;

		std::vector<unsigned int> vectorInds;
		vectorInds.reserve(m_params[id].m_supportVectors->numElements());
		for (int i = m_params[id].m_supportVectors->getNext(-1); i != -1; i = m_params[id].m_supportVectors->getNext(i)) {
			vectorInds.push_back(i);
		}

		std::vector<float> results;
		if(!vectorInds.empty()){
			m_dxMgr->copyToGraphicsCard(m_buffers[GB_AlphaBuffer],m_params[id].m_alpha);
			m_sharedBuffer[0].cb_ind1 = index;
			m_sharedBuffer[0].instanceCount = unsigned int(vectorInds.size());

			m_dxMgr->copyToGraphicsCard(m_constantBuffers[CB_Shared],m_sharedBuffer);
			m_dxMgr->copyToGraphicsCard(m_buffers[GB_InputInds],vectorInds);

			m_dxMgr->setComputeShader(m_shaders[GS_SVMOutput]);
			m_dxMgr->launchComputation(int(vectorInds.size()),1,1);

			results.assign(vectorInds.size(),0);
			m_dxMgr->copyFromGraphicsCard(m_buffers[GB_OutputBuffer],results);

			for(unsigned int i=0; i<results.size(); i++){
				result += results[i];
			}
		}
		result -= m_params[id].m_b;
		
		return result;
	}

	void DX11SVM::testInstances(int id){
		m_dxMgr->copyToGraphicsCard(m_buffers[GB_AlphaBuffer],m_params[id].m_alpha);
		m_dxMgr->setComputeShader(m_shaders[GS_Testing]);

		std::vector<unsigned int> vectorInds;
		vectorInds.reserve(m_params[id].m_supportVectors->numElements());
		for (int i = m_params[id].m_supportVectors->getNext(-1); i != -1; i = m_params[id].m_supportVectors->getNext(i)) {
			vectorInds.push_back(i);
		}
		
		m_dxMgr->copyToGraphicsCard(m_buffers[GB_InputInds],vectorInds);

		m_sharedBuffer[0].instanceCount = m_params[id].m_evaluation->getNumTestingInstances();
		m_sharedBuffer[0].param1 = m_params[id].m_b;
		m_sharedBuffer[0].cb_ind1 = int(vectorInds.size());
		m_dxMgr->copyToGraphicsCard(m_constantBuffers[CB_Shared],m_sharedBuffer);

		m_dxMgr->launchComputation(m_sharedBuffer[0].instanceCount,1,1);

		std::vector<float> results;
		results.assign(m_sharedBuffer[0].instanceCount,0);
		m_dxMgr->copyFromGraphicsCard(m_buffers[GB_OutputBuffer],results);

		// Testing
		InstancePtr testInst;
		int classValue;
		for(unsigned int i=0; i<results.size(); i++){
			testInst = m_params[id].m_evaluation->getTestingInstance(i);
			classValue = testInst->classValue();
			if(results[i] < 0){
				if(classValue == m_document->m_cl1Value)
					m_params[id].cl1Correct++;
				else
					m_params[id].cl1Wrong++;
			}
			else{
				if(classValue == m_document->m_cl2Value)
					m_params[id].cl2Correct++;
				else
					m_params[id].cl2Wrong++;
			}
		}
	}

	void DX11SVM::performTestCycle(std::vector<int> &inds){
		
	}

	void DX11SVM::kernelEvaluations(std::vector<int> &inds, std::vector<float> &result, int id){
		if(inds.size() < 2)
			return;

		for(unsigned int i=0; i<inds.size(); i+=2){
			result.push_back(m_params[id].m_kernel->eval(inds[i], inds[i+1], m_params[id].m_evaluation->getTrainingInstance(inds[i])));
		}

		/*m_dxMgr->setComputeShader(m_shaders[GS_KernelEvaluations]);
		m_sharedBuffer[0].instanceCount = inds.size()/2;
		m_dxMgr->copyToGraphicsCard(m_constantBuffers[CB_Shared],m_sharedBuffer);
		m_dxMgr->copyToGraphicsCard(m_buffers[GB_InputInds],inds);

		m_dxMgr->launchComputation(int(float(float(inds.size())/2.0f)/32.0f)+1,1,1);
		result.assign(inds.size()/2, 0);
		m_dxMgr->copyFromGraphicsCard(m_buffers[GB_OutputBuffer],result);*/
	}

	void DX11SVM::initGPUResources(){
		// Create gfx resources
		ID3D11UnorderedAccessView *uav;
		ID3D11ShaderResourceView *srv;
		ID3D11Buffer *buff;

		// Input buffer
		m_dxMgr->createStructuredBufferSRV<Value>(D3D11_USAGE_DEFAULT,0,UINT(m_document->m_data.size()),&buff,&srv,&m_document->m_data[0]);
		m_resourceViews[SRV_Data] = srv;
		m_buffers[GB_DataBuffer] = buff;

		// Self product
		m_dxMgr->createStructuredBufferUAV<float>(D3D11_USAGE_DEFAULT,0,m_document->getNumInstances(),&buff,&uav,NULL);
		m_accessViews[UAV_SelfProd] = uav;
		m_buffers[GB_SelfProdBuffer] = buff;

		// Constant buffer
		m_sharedBuffer[0].instanceCount = m_document->getNumInstances();
		m_sharedBuffer[0].instanceLength = m_document->getNumAttributes()+1;
		m_sharedBuffer[0].classIndex = m_document->m_classAttributeId;

		m_dxMgr->createConstantBuffer<SharedBuffer>(&buff,&m_sharedBuffer[0]);
		m_constantBuffers[CB_Shared] = buff;

		// Compute self product
		m_dxMgr->setComputeShader(m_shaders[GS_SelfProd]);
		m_dxMgr->setComputeShaderConstantBuffers(m_constantBuffers);
		m_dxMgr->setComputeShaderResourceViews(m_resourceViews);
		m_dxMgr->setComputeShaderUnorderedAccessViews(m_accessViews);

		m_dxMgr->launchComputation((m_document->getNumInstances()/32)+1,1,1);
	}

	void DX11SVM::cleanGPUResources(){
		std::map<int,ID3D11UnorderedAccessView*>::iterator uav_itr = m_accessViews.begin();
		while(uav_itr != m_accessViews.end()){
			uav_itr->second->Release();
			uav_itr++;
		}
		m_accessViews.clear();

		std::map<int,ID3D11ShaderResourceView*>::iterator srv_itr = m_resourceViews.begin();
		while(srv_itr != m_resourceViews.end()){
			srv_itr->second->Release();
			srv_itr++;
		}
		m_resourceViews.clear();

		std::map<int,ID3D11Buffer*>::iterator gb_itr = m_buffers.begin();
		while(gb_itr != m_buffers.end()){
			gb_itr->second->Release();
			gb_itr++;
		}
		m_buffers.clear();

		std::map<int,ID3D11Buffer*>::iterator cb_itr = m_constantBuffers.begin();
		while(cb_itr != m_constantBuffers.end()){
			cb_itr->second->Release();
			cb_itr++;
		}
		m_constantBuffers.clear();
	}

	void DX11SVM::initializeFold(int id){
		// Create gfx resources
		ID3D11UnorderedAccessView *uav;
		ID3D11ShaderResourceView *srv;
		ID3D11Buffer *buff;

		// Kernel inds
		m_dxMgr->createStructuredBufferSRV<unsigned int>(D3D11_USAGE_DYNAMIC,D3D11_CPU_ACCESS_WRITE,m_params[id].m_evaluation->getNumTrainingInstances(),&buff,&srv,NULL);
		m_resourceViews[SRV_InputInds] = srv;
		m_buffers[GB_InputInds] = buff;

		// Training instance inds
		m_dxMgr->createStructuredBufferSRV<unsigned int>(D3D11_USAGE_DEFAULT,0,m_params[id].m_evaluation->getNumTrainingInstances(),&buff,&srv,&m_params[id].m_evaluation->getTrainingInds()[0]);
		m_resourceViews[SRV_TrainingIndices] = srv;
		m_buffers[GB_TrainingIndices] = buff;

		// Testing instance inds
		m_dxMgr->createStructuredBufferSRV<unsigned int>(D3D11_USAGE_DEFAULT,0,m_params[id].m_evaluation->getNumTestingInstances(),&buff,&srv,&m_params[id].m_evaluation->getTestingInds()[0]);
		m_resourceViews[SRV_TestingIndices] = srv;
		m_buffers[GB_TestingIndices] = buff;

		// Class
		m_dxMgr->createStructuredBufferSRV<float>(D3D11_USAGE_DEFAULT,0,m_params[id].m_evaluation->getNumTrainingInstances(),&buff,&srv,&m_params[id].m_class[0]);
		m_resourceViews[SRV_Class] = srv;
		m_buffers[GB_ClassBuffer] = buff;

		// Alpha
		m_dxMgr->createStructuredBufferSRV<float>(D3D11_USAGE_DYNAMIC,D3D11_CPU_ACCESS_WRITE,m_params[id].m_evaluation->getNumTrainingInstances(),&buff,&srv,&m_params[id].m_alpha[0]);
		m_resourceViews[SRV_Alpha] = srv;
		m_buffers[GB_AlphaBuffer] = buff;

		// Error
		m_dxMgr->createStructuredBufferUAV<float>(D3D11_USAGE_DEFAULT,0,m_params[id].m_evaluation->getNumTrainingInstances(),&buff,&uav,&m_params[id].m_errors[0]);
		m_accessViews[UAV_Error] = uav;
		m_buffers[GB_ErrorBuffer] = buff;

		// Output
		m_dxMgr->createStructuredBufferUAV<float>(D3D11_USAGE_DEFAULT,0,m_params[id].m_evaluation->getNumTrainingInstances(),&buff,&uav,NULL);
		m_accessViews[UAV_Output] = uav;
		m_buffers[GB_OutputBuffer] = buff;

		// Find BI
		m_dxMgr->createStructuredBufferUAV<algorithmParams>(D3D11_USAGE_DEFAULT,0,int(float(m_params[id].m_evaluation->getNumTrainingInstances())/32.0f)+1,&buff,&uav,NULL);
		m_accessViews[UAV_FindBI] = uav;
		m_buffers[GB_FindBI] = buff;

		m_sharedBuffer[0].instanceCount = m_params[id].m_evaluation->getNumTrainingInstances();
		m_sharedBuffer[0].kernelParam1 = m_params[id].m_kernel->getParameter(0);
		m_sharedBuffer[0].kernelParam2 = m_params[id].m_kernel->getParameter(1);
		m_sharedBuffer[0].kernel = m_params[id].m_kernel->getId();
		m_dxMgr->copyToGraphicsCard(m_constantBuffers[CB_Shared],m_sharedBuffer);

		m_dxMgr->setComputeShader(m_shaders[GS_UpdateErrorCache]);
		m_dxMgr->setComputeShaderConstantBuffers(m_constantBuffers);
		m_dxMgr->setComputeShaderResourceViews(m_resourceViews);
		m_dxMgr->setComputeShaderUnorderedAccessViews(m_accessViews);
	}

	void DX11SVM::endFold(int id){
		m_accessViews[UAV_Output]->Release();
		m_accessViews[UAV_Error]->Release();
		m_accessViews[UAV_FindBI]->Release();

		m_accessViews.erase(UAV_Output);
		m_accessViews.erase(UAV_Error);
		m_accessViews.erase(UAV_FindBI);

		m_resourceViews[SRV_Alpha]->Release();
		m_resourceViews[SRV_Class]->Release();
		m_resourceViews[SRV_InputInds]->Release();
		m_resourceViews[SRV_TestingIndices]->Release();
		m_resourceViews[SRV_TrainingIndices]->Release();

		m_resourceViews.erase(SRV_Alpha);
		m_resourceViews.erase(SRV_Class);
		m_resourceViews.erase(SRV_InputInds);
		m_resourceViews.erase(SRV_TrainingIndices);
		m_resourceViews.erase(SRV_TestingIndices);

		m_buffers[GB_OutputBuffer]->Release();
		m_buffers[GB_ErrorBuffer]->Release();
		m_buffers[GB_AlphaBuffer]->Release();
		m_buffers[GB_ClassBuffer]->Release();
		m_buffers[GB_TestingIndices]->Release();
		m_buffers[GB_TrainingIndices]->Release();
		m_buffers[GB_InputInds]->Release();
		m_buffers[GB_FindBI]->Release();

		m_buffers.erase(GB_OutputBuffer);
		m_buffers.erase(GB_ErrorBuffer);
		m_buffers.erase(GB_AlphaBuffer);
		m_buffers.erase(GB_ClassBuffer);
		m_buffers.erase(GB_TestingIndices);
		m_buffers.erase(GB_TrainingIndices);
		m_buffers.erase(GB_InputInds);
		m_buffers.erase(GB_FindBI);
	}
}