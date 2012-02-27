#include "stdafx.h"
#include "DX11SVM.h"
#include "CrossValidation.h"
#include "ConfigManager.h"
#include "GUIManager.h"

namespace SVM_Framework{
	DX11SVM::DX11SVM(GraphicsManagerPtr dxMgr){
		m_dxMgr = boost::static_pointer_cast<DirectXManager>(dxMgr);

		ID3D11ComputeShader* shader;
		if(m_dxMgr->createComputeShader("SMO_train_init.hlsl",&shader) < 0){
			assert(0);
		}
		m_shaders.push_back(shader);
		if(m_dxMgr->createComputeShader("SMO_test_init.hlsl",&shader) < 0){
			assert(0);
		}
		m_shaders.push_back(shader);
		if(m_dxMgr->createComputeShader("SMO_KernelEvals.hlsl",&shader) < 0){
			assert(0);
		}
		m_shaders.push_back(shader);
		if(m_dxMgr->createComputeShader("SMO_SVMOutput.hlsl",&shader) < 0){
			assert(0);
		}
		m_shaders.push_back(shader);
		if(m_dxMgr->createComputeShader("SMO_UpdateError.hlsl",&shader) < 0){
			assert(0);
		}
		m_shaders.push_back(shader);
		if(m_dxMgr->createComputeShader("SMO_FindBI.hlsl",&shader) < 0){
			assert(0);
		}
		m_shaders.push_back(shader);
		m_sharedBuffer.push_back(SharedBuffer());
	}

	DX11SVM::~DX11SVM(){
		for(unsigned int i=0; i<m_shaders.size(); i++){
			m_shaders[i]->Release();
		}
		for(unsigned int i=0; i<m_accessViews.size(); i++){
			m_accessViews[i]->Release();
		}
		for(unsigned int i=0; i<m_resourceViews.size(); i++){
			m_resourceViews[i]->Release();
		}
		std::map<GBuffers,ID3D11Buffer*>::iterator itr = m_buffers.begin();
		while(itr != m_buffers.end()){
			itr->second->Release();
			itr++;
		}
		for(unsigned int i=0; i<m_constantBuffers.size(); i++){
			m_constantBuffers[i]->Release();
		}
	}

	void DX11SVM::execute(){
		initGPUResources();

		std::wstringstream outputStream;
		unsigned int timerId = ConfigManager::startTimer();

		long	cl1Correct = 0,
				cl1Wrong = 0,
				cl2Correct = 0,
				cl2Wrong = 0,
				iterations = 0;
		long	cacheHits = 0,
				kernelEvals = 0,
				supportVectors = 0;

		m_params.assign(1,AlgoParams());
		m_params[0].m_evaluation = IEvaluationPtr(new CrossValidation(10));
		m_params[0].m_evaluation->setData(m_document,m_data);
		std::wstringstream stream;
		for(unsigned int i=0; i<10; i++){
			stream << "Working on fold " << i+1 << "...\r\n";
			m_data->m_gui->setText(IDC_STATIC_INFOTEXT,stream.str());
			boost::static_pointer_cast<CrossValidation>(m_params[0].m_evaluation)->setFold(i);
			executeFold(0);
			cl1Correct += m_params[0].cl1Correct;
			cl2Correct += m_params[0].cl2Correct;
			cl1Wrong += m_params[0].cl1Wrong;
			cl2Wrong += m_params[0].cl2Wrong;
			iterations += m_params[0].iterations;
			supportVectors += m_params[0].m_supportVectors->numElements();

			cacheHits += m_params[0].m_kernel->getCacheHits();
			kernelEvals += m_params[0].m_kernel->getKernelEvals();
			m_params[0].m_kernel->resetCounters();
		}

		for(unsigned int i=0; i<m_accessViews.size(); i++){
			m_accessViews[i]->Release();
		}
		m_accessViews.clear();
		for(unsigned int i=0; i<m_resourceViews.size(); i++){
			m_resourceViews[i]->Release();
		}
		m_resourceViews.clear();
		std::map<GBuffers,ID3D11Buffer*>::iterator itr = m_buffers.begin();
		while(itr != m_buffers.end()){
			itr->second->Release();
			itr++;
		}
		m_buffers.clear();
		for(unsigned int i=0; i<m_constantBuffers.size(); i++){
			m_constantBuffers[i]->Release();
		}
		m_constantBuffers.clear();

		outputStream << "Total time: " << ConfigManager::getTime(timerId);

		outputStream	<< "\r\n\r\n" << m_document->m_cl1Value << "	" << m_document->m_cl2Value 
						<< "\r\n" << cl1Correct << "	" << cl1Wrong << "	" << m_document->m_cl1Value 
						<< "\r\n" << cl2Wrong << "	" << cl2Correct << "	" << m_document->m_cl2Value << "\r\n";
		outputStream << "Accuracy: " << (float(cl1Correct+cl2Correct)/float(cl1Correct+cl2Correct+cl1Wrong+cl2Wrong))*100 << "%\r\n\r\n";
		outputStream << "Total number tested: " << cl1Correct+cl2Correct+cl1Wrong+cl2Wrong << "\r\n";
		outputStream << "Iteration count: " << iterations/10 << "\r\n";
		outputStream << "Support vectors: " << supportVectors/10 << "\r\n";
		outputStream << "Cachehits: " << cacheHits/10 << " (" << (double(cacheHits/10)/double((cacheHits/10)+(kernelEvals/10)))*100 << "%)\r\n";
		outputStream <<	"Kernel evals on CPU: " << kernelEvals/10;

		m_data->m_gui->setText(IDC_STATIC_INFOTEXT,outputStream.str());
		ConfigManager::removeTimer(timerId);
	}

	void DX11SVM::lagrangeThresholdUpdate(double p1, double p2, int id, int i1, int i2){
		m_dxMgr->setComputeShader(m_shaders[2]);

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
			m_sharedBuffer[0].instanceCount = kernelEvals.size();

			m_dxMgr->copyToGraphicsCard(m_constantBuffers[0],m_sharedBuffer);
			m_dxMgr->copyToGraphicsCard(m_buffers[GB_InputInds],kernelEvals);

			m_dxMgr->launchComputation((kernelEvals.size()/32)+1,1,1);
			
			m_dxMgr->copyFromGraphicsCard(m_buffers[GB_ErrorBuffer],m_params[id].m_errors);

			/*m_dxMgr->setComputeShader(m_shaders[5]);
			m_dxMgr->launchComputation((kernelEvals.size()/32)+1,1,1);
			biParams.assign((kernelEvals.size()/32)+1,algorithmParams());
			m_dxMgr->copyFromGraphicsCard(m_buffers[1],biParams);*/
		}

		//// Update thresholds
		//m_params[id].m_bLow = -DBL_MAX; m_params[id].m_bUp = DBL_MAX;
		//m_params[id].m_iLow = -1; m_params[id].m_iUp = -1;
		//for(unsigned int i=0; i<biParams.size(); i++){
		//	if(biParams[i].s_bUp < m_params[id].m_bUp){
		//		m_params[id].m_bUp = biParams[i].s_bUp;
		//		m_params[id].m_iUp = biParams[i].s_iUp;
		//	}
		//	if(biParams[i].s_bLow > m_params[id].m_bLow){
		//		m_params[id].m_bLow = biParams[i].s_bLow;
		//		m_params[id].m_iLow = biParams[i].s_iLow;
		//	}
		//}

		// Update thresholds
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

	void DX11SVM::updateErrorCache(float f, int i, int id){
		m_sharedBuffer[0].cb_ind1 = i;
		m_sharedBuffer[0].param1 = f;
		m_dxMgr->copyToGraphicsCard(m_constantBuffers[0],m_sharedBuffer);
		m_dxMgr->setComputeShader(m_shaders[4]);
		m_dxMgr->launchComputation(1,1,1);
	}

	double DX11SVM::SVMOutput(int index, InstancePtr inst, int id){
		float result = 0;
		//float resultCheck = 0;

		std::vector<unsigned int> vectorInds;
		vectorInds.reserve(m_params[id].m_supportVectors->numElements());
		for (int i = m_params[id].m_supportVectors->getNext(-1); i != -1; i = m_params[id].m_supportVectors->getNext(i)) {
			vectorInds.push_back(i);
			//resultCheck += m_params[id].m_class[i] * m_params[id].m_alpha[i] * m_params[id].m_kernel->eval(index, i, inst);
		}

		std::vector<float> results;
		if(!vectorInds.empty()){
			m_dxMgr->copyToGraphicsCard(m_buffers[GB_AlphaBuffer],m_params[id].m_alpha);
			m_sharedBuffer[0].cb_ind1 = index;
			m_sharedBuffer[0].instanceCount = vectorInds.size();

			m_dxMgr->copyToGraphicsCard(m_constantBuffers[0],m_sharedBuffer);
			m_dxMgr->copyToGraphicsCard(m_buffers[GB_InputInds],vectorInds);

			m_dxMgr->setComputeShader(m_shaders[3]);
			m_dxMgr->launchComputation(vectorInds.size(),1,1);

			results.assign(vectorInds.size(),0);
			m_dxMgr->copyFromGraphicsCard(m_buffers[GB_OutputBuffer],results);

			for(unsigned int i=0; i<results.size(); i++){
				result += results[i];
			}
		}
		result -= m_params[id].m_b;
		//resultCheck -= m_params[id].m_b;

		//assert(abs(resultCheck)-abs(result) < 1.0e-04);
		
		return result;
	}

	void DX11SVM::testInstances(int id){
		m_dxMgr->copyToGraphicsCard(m_buffers[GB_AlphaBuffer],m_params[id].m_alpha);
		m_dxMgr->setComputeShader(m_shaders[1]);

		std::vector<unsigned int> vectorInds;
		vectorInds.reserve(m_params[id].m_supportVectors->numElements());
		for (int i = m_params[id].m_supportVectors->getNext(-1); i != -1; i = m_params[id].m_supportVectors->getNext(i)) {
			vectorInds.push_back(i);
		}
		m_dxMgr->copyToGraphicsCard(m_buffers[GB_InputInds],vectorInds);

		m_sharedBuffer[0].instanceCount = m_params[id].m_evaluation->getNumTestingInstances();
		m_sharedBuffer[0].param1 = m_params[id].m_b;
		m_sharedBuffer[0].cb_ind1 = vectorInds.size();
		m_dxMgr->copyToGraphicsCard(m_constantBuffers[0],m_sharedBuffer);

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

	void DX11SVM::initGPUResources(){
		// Create gfx resources
		ID3D11UnorderedAccessView *uav;
		ID3D11ShaderResourceView *srv;
		ID3D11Buffer *buff,*readBuffer;

		// Input buffer
		m_dxMgr->createStructuredBufferSRV<Value>(D3D11_USAGE_DEFAULT,0,m_document->m_data.size(),&buff,&srv,&m_document->m_data[0]);
		m_resourceViews.push_back(srv);
		m_buffers[GB_DataBuffer] = buff;

		//// Params
		//m_dxMgr->createStructuredBufferUAV<algorithmParams>(D3D11_USAGE_DEFAULT,0,m_document->getNumInstances(),&buff,&uav,NULL);
		//m_accessViews.push_back(uav);
		//m_buffers.push_back(buff);

		// Self product
		m_dxMgr->createStructuredBufferUAV<float>(D3D11_USAGE_DEFAULT,0,m_document->getNumInstances(),&buff,&uav,NULL);
		m_accessViews.push_back(uav);
		m_buffers[GB_SelfProdBuffer] = buff;

		// Constant buffer
		m_sharedBuffer[0].instanceCount = m_document->getNumInstances();
		m_sharedBuffer[0].instanceLength = m_document->getNumAttributes()+1;
		m_sharedBuffer[0].classIndex = m_document->m_classAttributeId;

		m_dxMgr->createConstantBuffer<SharedBuffer>(&buff,&m_sharedBuffer[0]);
		m_constantBuffers.push_back(buff);

		// Compute self product
		m_dxMgr->setComputeShader(m_shaders[0]);
		m_dxMgr->setComputeShaderConstantBuffers(m_constantBuffers);
		m_dxMgr->setComputeShaderResourceViews(m_resourceViews);
		m_dxMgr->setComputeShaderUnorderedAccessViews(m_accessViews);

		m_dxMgr->launchComputation((m_document->getNumInstances()/32)+1,1,1);
	}

	void DX11SVM::initializeFold(int id){
		// Create gfx resources
		ID3D11UnorderedAccessView *uav;
		ID3D11ShaderResourceView *srv;
		ID3D11Buffer *buff,*readBuffer;

		// Kernel inds
		m_dxMgr->createStructuredBufferSRV<unsigned int>(D3D11_USAGE_DYNAMIC,D3D11_CPU_ACCESS_WRITE,m_params[id].m_evaluation->getNumTrainingInstances(),&buff,&srv,NULL);
		m_resourceViews.push_back(srv);
		m_buffers[GB_InputInds] = buff;

		// Training instance inds
		m_dxMgr->createStructuredBufferSRV<unsigned int>(D3D11_USAGE_DEFAULT,0,m_params[id].m_evaluation->getNumTrainingInstances(),&buff,&srv,&m_params[id].m_evaluation->getTrainingInds()[0]);
		m_resourceViews.push_back(srv);
		m_buffers[GB_TrainingIndices] = buff;

		// Testing instance inds
		m_dxMgr->createStructuredBufferSRV<unsigned int>(D3D11_USAGE_DEFAULT,0,m_params[id].m_evaluation->getNumTestingInstances(),&buff,&srv,&m_params[id].m_evaluation->getTestingInds()[0]);
		m_resourceViews.push_back(srv);
		m_buffers[GB_TestingIndices] = buff;

		// Class
		m_dxMgr->createStructuredBufferSRV<float>(D3D11_USAGE_DEFAULT,0,m_params[id].m_evaluation->getNumTrainingInstances(),&buff,&srv,&m_params[id].m_class[0]);
		m_resourceViews.push_back(srv);
		m_buffers[GB_ClassBuffer] = buff;

		// Alpha
		m_dxMgr->createStructuredBufferSRV<float>(D3D11_USAGE_DYNAMIC,D3D11_CPU_ACCESS_WRITE,m_params[id].m_evaluation->getNumTrainingInstances(),&buff,&srv,&m_params[id].m_alpha[0]);
		m_resourceViews.push_back(srv);
		m_buffers[GB_AlphaBuffer] = buff;

		// Error
		m_dxMgr->createStructuredBufferUAV<float>(D3D11_USAGE_DEFAULT,0,m_params[id].m_evaluation->getNumTrainingInstances(),&buff,&uav,&m_params[id].m_errors[0]);
		m_accessViews.push_back(uav);
		m_buffers[GB_ErrorBuffer] = buff;

		// Output
		m_dxMgr->createStructuredBufferUAV<float>(D3D11_USAGE_DEFAULT,0,m_params[id].m_evaluation->getNumTrainingInstances(),&buff,&uav,NULL);
		m_accessViews.push_back(uav);
		m_buffers[GB_OutputBuffer] = buff;

		m_sharedBuffer[0].instanceCount = m_params[id].m_evaluation->getNumTrainingInstances();
		m_sharedBuffer[0].kernelParam1 = m_params[id].m_kernel->getParameter(0);
		m_sharedBuffer[0].kernelParam2 = m_params[id].m_kernel->getParameter(1);
		m_sharedBuffer[0].kernel = m_params[id].m_kernel->getId();
		m_dxMgr->copyToGraphicsCard(m_constantBuffers[0],m_sharedBuffer);

		m_dxMgr->setComputeShader(m_shaders[2]);
		m_dxMgr->setComputeShaderConstantBuffers(m_constantBuffers);
		m_dxMgr->setComputeShaderResourceViews(m_resourceViews);
		m_dxMgr->setComputeShaderUnorderedAccessViews(m_accessViews);
	}

	void DX11SVM::endFold(int id){
		m_resourceViews.back()->Release();
		m_resourceViews.pop_back();
		m_resourceViews.back()->Release();
		m_resourceViews.pop_back();
		m_resourceViews.back()->Release();
		m_resourceViews.pop_back();
		m_resourceViews.back()->Release();
		m_resourceViews.pop_back();
		m_resourceViews.back()->Release();
		m_resourceViews.pop_back();
		m_accessViews.back()->Release();
		m_accessViews.pop_back();
		m_accessViews.back()->Release();
		m_accessViews.pop_back();

		m_buffers[GB_OutputBuffer]->Release();
		m_buffers[GB_ErrorBuffer]->Release();
		m_buffers[GB_AlphaBuffer]->Release();
		m_buffers[GB_ClassBuffer]->Release();
		m_buffers[GB_TestingIndices]->Release();
		m_buffers[GB_TrainingIndices]->Release();
		m_buffers[GB_InputInds]->Release();

		m_buffers.erase(GB_OutputBuffer);
		m_buffers.erase(GB_ErrorBuffer);
		m_buffers.erase(GB_AlphaBuffer);
		m_buffers.erase(GB_ClassBuffer);
		m_buffers.erase(GB_TestingIndices);
		m_buffers.erase(GB_TrainingIndices);
		m_buffers.erase(GB_InputInds);
	}
}