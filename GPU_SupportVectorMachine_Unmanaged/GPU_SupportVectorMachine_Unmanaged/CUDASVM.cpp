#include "stdafx.h"
#include "CUDASVM.h"

#define thread_group_size 64

namespace SVM_Framework{
	extern "C" void CUDA_updateConstantBuffer(void* src);
	extern "C" void CUDA_selfProd(unsigned int num_threads,ISVM::svm_precision* b_selfProd, ISVM::svm_precision* b_inputData);
	extern "C" void CUDA_testInstances(unsigned int num_threads, ISVM::svm_precision* b_output, ISVM::svm_precision* b_selfProd, ISVM::svm_precision* b_inputData, ISVM::svm_precision* b_class, ISVM::svm_precision* b_alpha);
	extern "C" void CUDA_lagrangeUpdate(unsigned int num_threads, ISVM::svm_precision* b_output, ISVM::svm_precision* b_selfProd, ISVM::svm_precision* b_inputData);
	extern "C" void CUDA_SVMOutput(unsigned int num_threads, ISVM::svm_precision* b_output, ISVM::svm_precision* b_selfProd, ISVM::svm_precision* b_inputData, ISVM::svm_precision* b_class, ISVM::svm_precision* b_alpha);

	CUDASVM::CUDASVM(GraphicsManagerPtr dxMgr){
		m_copyBuffer.assign(1,std::vector<Value::v_precision>());
		m_inds.assign(1,std::vector<unsigned int>());
		m_numDevices = 1;
	}
	
	CUDASVM::~CUDASVM(){
		cudaDeviceReset();
	}

	void CUDASVM::beginExecute(){
		cudaSetDevice(0);

		cudaMalloc<svm_precision>(&b_inputData,sizeof(svm_precision)*m_document->m_data.size());
		cudaMemcpy(b_inputData,&m_document->m_data[0],sizeof(svm_precision)*m_document->m_data.size(),cudaMemcpyHostToDevice);

		cudaMalloc<svm_precision>(&b_selfProd,sizeof(svm_precision)*m_document->getNumInstances());

		m_sharedBuffer[0].instanceCount = m_document->getNumInstances();
		m_sharedBuffer[0].instanceLength = m_document->getNumAttributes()+1;
		m_sharedBuffer[0].classIndex = m_document->m_classAttributeId;
		CUDA_updateConstantBuffer(&m_sharedBuffer[0]);
		CUDA_selfProd(m_document->getNumInstances(),b_selfProd,b_inputData);
		cudaDeviceSynchronize();

		m_copyBuffer[0].assign(m_document->getNumInstances(),0);
		cudaMemcpy(&m_copyBuffer[0][0],b_selfProd,sizeof(svm_precision)*m_copyBuffer[0].size(),cudaMemcpyDeviceToHost);
	}

	void CUDASVM::endExecute(){
		cudaFree(b_inputData);
		cudaFree(b_selfProd);

		cudaDeviceReset();
	}

	void CUDASVM::lagrangeThresholdUpdate(svm_precision p1, svm_precision p2, int id, int i1, int i2){
		// Divide between devices
		int distrib = 0;
		Value::v_precision cachedRes = 0;
		for(unsigned int i=0; i<m_numDevices; i++){
			m_inds[i].clear();
		}
		for(int j = m_params[id].m_I0->getNext(-1); j != -1; j = m_params[id].m_I0->getNext(j)){
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
				CUDA_updateConstantBuffer(&m_sharedBuffer[0]);
				cudaMemcpy(b_inputInds,&m_inds[i][0],sizeof(unsigned int)*m_inds[i].size(),cudaMemcpyHostToDevice);
				CUDA_lagrangeUpdate(m_inds[i].size(),b_output,b_selfProd,b_inputData);
			}
		}

		// Copy back results
		for(unsigned int i=0; i<m_numDevices; i++){
			m_copyBuffer[i].clear();
		}
		for(unsigned int i=0; i<m_inds.size(); i++){
			if(!m_inds[i].empty()){
				m_copyBuffer[i].assign(m_inds[i].size()*2,0);
				cudaDeviceSynchronize();
				cudaMemcpy(&m_copyBuffer[i][0],b_output,sizeof(svm_precision)*m_copyBuffer[i].size(),cudaMemcpyDeviceToHost);
			}
		}

		// Update error buffer
		for(unsigned int i=0; i<m_inds.size(); i++){
			if(!m_inds[i].empty()){
				for(unsigned int j=0; j<m_inds[i].size(); j++){
					m_params[id].m_errors[m_inds[i][j]] += ((p1 * m_copyBuffer[i][j*2]) + (p2 * m_copyBuffer[i][(j*2)+1]));
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

	void CUDASVM::updateErrorCache(svm_precision f, int i, int id){
		m_params[id].m_errors[i] = f;
	}

	ISVM::svm_precision CUDASVM::SVMOutput(int index, InstancePtr inst, int id){
		svm_precision result = 0;

		int distrib = 0;
		Value::v_precision cachedRes = 0;
		for(unsigned int i=0; i<m_numDevices; i++){
			m_inds[i].clear();
		}
		for(int i = m_params[id].m_supportVectors->getNext(-1); i != -1; i = m_params[id].m_supportVectors->getNext(i)){
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
					cudaMemcpy(b_alpha,&m_params[id].m_alpha[0],sizeof(svm_precision)*m_params[id].m_alpha.size(),cudaMemcpyHostToDevice);
				}*/
				
				m_sharedBuffer[0].instanceCount = unsigned int(m_inds[i].size());
				CUDA_updateConstantBuffer(&m_sharedBuffer[0]);
				cudaMemcpy(b_inputInds,&m_inds[i][0],sizeof(unsigned int)*m_inds[i].size(),cudaMemcpyHostToDevice);

				CUDA_SVMOutput(m_inds[i].size()*thread_group_size,b_output,b_selfProd,b_inputData,b_class,b_alpha);
			}
		}
		/*if(m_alphaTransferNeeded){
			m_alphaTransferNeeded = false;
		}*/

		for(unsigned int i=0; i<m_numDevices; i++){
			m_copyBuffer[i].clear();
		}
		for(unsigned int i=0; i<m_inds.size(); i++){
			if(!m_inds[i].empty()){
				m_copyBuffer[i].assign(m_inds[i].size(),0);
				cudaDeviceSynchronize();
				cudaMemcpy(&m_copyBuffer[i][0],b_output,sizeof(svm_precision)*m_copyBuffer[i].size(),cudaMemcpyDeviceToHost);
			}
			m_params[id].m_kernel->insertIntoCache(m_inds[i],m_copyBuffer[i],index);
		}

		for(unsigned int i=0; i<m_copyBuffer.size(); i++){
			for(unsigned int j=0; j<m_copyBuffer[i].size(); j++){
				result += m_params[id].m_class[m_inds[i][j]] * m_params[id].m_alpha[m_inds[i][j]] * m_copyBuffer[i][j];
			}
		}
		result -= m_params[id].m_b;
		
		return result;
	}

	void CUDASVM::testInstances(std::vector<svm_precision> &finalResult, int id){
		for(unsigned int i=0; i<m_numDevices; i++){
			cudaMemcpy(b_alpha,&m_params[id].m_alpha[0],sizeof(svm_precision)*m_params[id].m_alpha.size(),cudaMemcpyHostToDevice);
		}

		m_inds[0].clear();
		m_inds[0].reserve(m_params[id].m_supportVectors->numElements());
		for (int i = m_params[id].m_supportVectors->getNext(-1); i != -1; i = m_params[id].m_supportVectors->getNext(i)) {
			m_inds[0].push_back(i);
		}
		
		for(unsigned int i=0; i<m_numDevices; i++){
			cudaMemcpy(b_inputInds,&m_inds[i][0],sizeof(unsigned int)*m_inds[i].size(),cudaMemcpyHostToDevice);
		}

		int instances = m_params[id].m_evaluation->getNumTestingInstances();
		std::vector<std::vector<unsigned int>> instanceMapping;
		instanceMapping.assign(m_numDevices,std::vector<unsigned int>());
		int distrib = 0;
		for(unsigned int i=0; i<instances; i++){
			instanceMapping[distrib++].push_back(i);
			if(distrib >= instanceMapping.size())
				distrib = 0;
		}
		
		int max = 0;
		m_sharedBuffer[0].instanceCount = int(m_inds[0].size());
		for(unsigned int i=0; i<m_numDevices; i++){
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
					CUDA_updateConstantBuffer(&m_sharedBuffer[0]);
					CUDA_testInstances(m_sharedBuffer[0].instanceCount,b_output,b_selfProd,b_inputData,b_class,b_alpha);
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
					cudaDeviceSynchronize();
					cudaMemcpy(&m_copyBuffer[j][0],b_output,sizeof(svm_precision)*m_copyBuffer[j].size(),cudaMemcpyDeviceToHost);
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

	void CUDASVM::kernelEvaluations(std::vector<int> &inds, std::vector<svm_precision> &result, int id){
		if(inds.size() < 2)
			return;

		for(unsigned int i=0; i<inds.size(); i+=2){
			result.push_back(m_params[id].m_kernel->eval(inds[i], inds[i+1], m_params[id].m_evaluation->getTrainingInstance(inds[i])));
		}
	}

	void CUDASVM::initializeFold(int id){
		cudaError_t errorCheck;
		const textureReference* texRefPtr;
		int trainingSetSize = m_params[id].m_evaluation->getNumTrainingInstances();

		cudaMalloc(&b_output,sizeof(svm_precision)*(trainingSetSize*2));

		cudaMalloc(&b_class,sizeof(svm_precision)*trainingSetSize);
		cudaMemcpy(b_class, &m_params[id].m_class[0], sizeof(svm_precision)*m_params[id].m_class.size(),cudaMemcpyHostToDevice);
		
		cudaMalloc(&b_alpha,sizeof(svm_precision)*trainingSetSize);
		cudaMemcpy(b_alpha, &m_params[id].m_alpha[0], sizeof(svm_precision)*m_params[id].m_alpha.size(),cudaMemcpyHostToDevice);

		cudaMalloc(&b_trainingInds,sizeof(unsigned int)*trainingSetSize);
		cudaMemcpy(b_trainingInds, &m_params[id].m_evaluation->getTrainingInds()[0], sizeof(unsigned int)*m_params[id].m_evaluation->getTrainingInds().size(),cudaMemcpyHostToDevice);
		cudaGetTextureReference(&texRefPtr, "t_trainingIndsTexture");
		cudaBindTexture(NULL,texRefPtr,b_trainingInds,&texRefPtr->channelDesc);

		cudaMalloc(&b_testingInds,sizeof(unsigned int)*m_params[id].m_evaluation->getTestingInds().size());
		cudaMemcpy(b_testingInds, &m_params[id].m_evaluation->getTestingInds()[0], sizeof(unsigned int)*m_params[id].m_evaluation->getTestingInds().size(),cudaMemcpyHostToDevice);
		cudaGetTextureReference(&texRefPtr, "t_testingIndsTexture");
		cudaBindTexture(NULL,texRefPtr,b_testingInds,&texRefPtr->channelDesc);

		cudaMalloc(&b_inputInds,sizeof(unsigned int)*trainingSetSize);
		cudaGetTextureReference(&texRefPtr, "t_inputIndsTexture");
		cudaBindTexture(NULL,texRefPtr,b_inputInds,&texRefPtr->channelDesc);
		
		m_sharedBuffer[0].instanceCount = m_params[id].m_evaluation->getNumTrainingInstances();
		m_sharedBuffer[0].kernelParam1 = m_params[id].m_kernel->getParameter(0);
		m_sharedBuffer[0].kernelParam2 = m_params[id].m_kernel->getParameter(1);
		m_sharedBuffer[0].kernel = m_params[id].m_kernel->getId();
	}

	void CUDASVM::endFold(int id){
		cudaFree(b_output);
		cudaFree(b_alpha);
		cudaFree(b_inputInds);
		cudaFree(b_trainingInds);
		cudaFree(b_testingInds);
		cudaFree(b_class);
	}
}