#include "stdafx.h"
#include "OpenCLSVM.h"

#define thread_group_size 64

namespace SVM_Framework{
	OpenCLSVM::OpenCLSVM(GraphicsManagerPtr dxMgr){
		// Get OpenCL platform count
		cl_uint NumPlatforms;
		clGetPlatformIDs(0, NULL, &NumPlatforms);

		// Get all OpenCL platform IDs
		cl_platform_id* PlatformIDs;
		PlatformIDs = new cl_platform_id[NumPlatforms];
		clGetPlatformIDs(NumPlatforms, PlatformIDs, NULL);

		// Select NVIDIA platform
		char cBuffer[1024];
		cl_uint NvPlatform;
		for(cl_uint i = 0; i < NumPlatforms; ++i){
			clGetPlatformInfo(PlatformIDs[i], CL_PLATFORM_NAME, 1024, cBuffer, NULL);
			if(strstr(cBuffer, "NVIDIA") != NULL){
				NvPlatform = i;
				break;
			}
		}

		// Get a GPU device on Platform
		cl_device_id cdDevice;
		clGetDeviceIDs(PlatformIDs[NvPlatform], CL_DEVICE_TYPE_GPU, 1, &cdDevice, NULL);
		delete[] PlatformIDs;

		// Create a context
		m_context = clCreateContext(0, 1, &cdDevice, NULL, NULL, NULL);

		// Create a command queue for the device in the context
		m_cQueue = clCreateCommandQueue(m_context, cdDevice, 0, NULL);

		// Create programs
		cl_int errorCode;
		compileProgram("SMO_SelfProd.cl",OCLP_SelfProd);
		errorCode = clBuildProgram(m_programs[OCLP_SelfProd], 0, 0, "-I ../Resources/Shaders", 0, 0);
		assert(errorCode == CL_SUCCESS);
		compileProgram("SMO_SVMOutput.cl",OCLP_SVMOutput);
		errorCode = clBuildProgram(m_programs[OCLP_SVMOutput], 0, 0, "-I ../Resources/Shaders", 0, 0);
		assert(errorCode == CL_SUCCESS);
		compileProgram("SMO_Test.cl",OCLP_Test);
		errorCode = clBuildProgram(m_programs[OCLP_Test], 0, 0, "-I ../Resources/Shaders", 0, 0);
		assert(errorCode == CL_SUCCESS);
		compileProgram("SMO_UpdateErrorCache.cl",OCLP_UpdateErrorCache);
		errorCode = clBuildProgram(m_programs[OCLP_UpdateErrorCache], 0, 0, "-I ../Resources/Shaders", 0, 0);
		assert(errorCode == CL_SUCCESS);

		// Create kernel instance
		m_kernels[OCLK_SelfProd] = clCreateKernel(m_programs[OCLP_SelfProd], "SelfProd", &errorCode);
		assert(errorCode == CL_SUCCESS);
		m_kernels[OCLK_SVMOutput] = clCreateKernel(m_programs[OCLP_SVMOutput], "SVMOutput", &errorCode);
		assert(errorCode == CL_SUCCESS);
		m_kernels[OCLK_Test] = clCreateKernel(m_programs[OCLP_Test], "Test", &errorCode);
		assert(errorCode == CL_SUCCESS);
		m_kernels[OCLK_UpdateErrorCache] = clCreateKernel(m_programs[OCLP_UpdateErrorCache], "UpdateErrorCache", &errorCode);
		assert(errorCode == CL_SUCCESS);

		clUnloadCompiler();

		m_inds.assign(1,std::vector<unsigned int>());
		m_copyBuffer.assign(1,std::vector<Value::v_precision>());
		m_numDevices = 1;
	}
	
	OpenCLSVM::~OpenCLSVM(){
		std::map<OpenCLEnums,cl_kernel>::iterator kItr = m_kernels.begin();
		while(kItr != m_kernels.end()){
			clReleaseKernel(kItr->second);
			kItr++;
		}
		std::map<OpenCLEnums,cl_program>::iterator pItr = m_programs.begin();
		while(pItr != m_programs.end()){
			clReleaseProgram(pItr->second);
			pItr++;
		}
		clReleaseCommandQueue(m_cQueue);
		clReleaseContext(m_context);
	}

	void OpenCLSVM::compileProgram(std::string filename, OpenCLEnums program){
		boost::filesystem::path path = ResourceManager::findFilePath(filename);
		std::ifstream input(path.generic_string(),std::ios_base::binary);
		size_t size = boost::filesystem::file_size(path);

		char* buffer = new char[size];
		input.read(buffer,size);

		cl_int errorCode;
		m_programs[program] = clCreateProgramWithSource(m_context, 1, (const char**)&buffer, (const size_t*)&size, &errorCode);
		assert(errorCode == CL_SUCCESS);

		input.close();
		delete[] buffer;
	}

	void OpenCLSVM::beginExecute(){
		m_buffers.clear();

		cl_int errorCode;
		m_buffers[OCLB_InputData] = clCreateBuffer(m_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, m_document->m_data.size() * sizeof(svm_precision), &m_document->m_data[0], &errorCode);
		assert(errorCode == CL_SUCCESS);
		m_buffers[OCLB_SelfProd] = clCreateBuffer(m_context,CL_MEM_READ_WRITE, m_document->getNumInstances() * sizeof(svm_precision), NULL, &errorCode);
		assert(errorCode == CL_SUCCESS);

		m_buffers[OCLB_ConstantBuffer] = clCreateBuffer(m_context,CL_MEM_READ_ONLY, sizeof(SharedBuffer), NULL, &errorCode);
		assert(errorCode == CL_SUCCESS);
		m_sharedBuffer[0].instanceCount = m_document->getNumInstances();
		m_sharedBuffer[0].instanceLength = m_document->getNumAttributes()+1;
		m_sharedBuffer[0].classIndex = m_document->m_classAttributeId;
		errorCode = clEnqueueWriteBuffer(m_cQueue,m_buffers[OCLB_ConstantBuffer],CL_TRUE,0,sizeof(SharedBuffer),&m_sharedBuffer[0],0,NULL,NULL);
		assert(errorCode == CL_SUCCESS);

		// Setup parameter values
		errorCode = clSetKernelArg(m_kernels[OCLK_SelfProd], 0, sizeof(cl_mem), (void *)&m_buffers[OCLB_InputData]);
		assert(errorCode == CL_SUCCESS);
		errorCode = clSetKernelArg(m_kernels[OCLK_SelfProd], 1, sizeof(cl_mem), (void *)&m_buffers[OCLB_SelfProd]);
		assert(errorCode == CL_SUCCESS);
		errorCode = clSetKernelArg(m_kernels[OCLK_SelfProd], 2, sizeof(SharedBuffer*), (void *)&m_buffers[OCLB_ConstantBuffer]);
		assert(errorCode == CL_SUCCESS);

		// Launch kernel
		size_t threadsInGroup = thread_group_size;
		size_t totalThreads = ((m_document->getNumInstances()/threadsInGroup)+1)*threadsInGroup;
		errorCode = clEnqueueNDRangeKernel(m_cQueue, m_kernels[OCLK_SelfProd], 1, 0, &totalThreads, &threadsInGroup, 0, 0, 0);
		assert(errorCode == CL_SUCCESS);
	}

	void OpenCLSVM::endExecute(){
		clReleaseMemObject(m_buffers[OCLB_ConstantBuffer]);
		clReleaseMemObject(m_buffers[OCLB_InputData]);
		clReleaseMemObject(m_buffers[OCLB_SelfProd]);
		m_buffers.clear();
	}

	void OpenCLSVM::lagrangeThresholdUpdate(svm_precision p1, svm_precision p2, int id, int i1, int i2){
		cl_int errorCode;

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
				errorCode = clEnqueueWriteBuffer(m_cQueue,m_buffers[OCLB_ConstantBuffer],CL_TRUE,0,sizeof(SharedBuffer),&m_sharedBuffer[0],0,NULL,NULL);
				assert(errorCode == CL_SUCCESS);
				errorCode = clEnqueueWriteBuffer(m_cQueue,m_buffers[OCLB_InputInds],CL_TRUE,0,sizeof(unsigned int)*m_inds[i].size(),&m_inds[i][0],0,NULL,NULL);
				assert(errorCode == CL_SUCCESS);

				size_t threadsInGroup = thread_group_size;
				size_t totalThreads = ((m_inds[i].size()/threadsInGroup)+1)*threadsInGroup;

				errorCode = clSetKernelArg(m_kernels[OCLK_UpdateErrorCache], 0, sizeof(cl_mem), (void *)&m_buffers[OCLB_InputData]);
				assert(errorCode == CL_SUCCESS);
				errorCode = clSetKernelArg(m_kernels[OCLK_UpdateErrorCache], 1, sizeof(cl_mem), (void *)&m_buffers[OCLB_InputInds]);
				assert(errorCode == CL_SUCCESS);
				errorCode = clSetKernelArg(m_kernels[OCLK_UpdateErrorCache], 2, sizeof(cl_mem), (void *)&m_buffers[OCLB_TrainingInds]);
				assert(errorCode == CL_SUCCESS);
				errorCode = clSetKernelArg(m_kernels[OCLK_UpdateErrorCache], 3, sizeof(cl_mem), (void *)&m_buffers[OCLB_Output]);
				assert(errorCode == CL_SUCCESS);
				errorCode = clSetKernelArg(m_kernels[OCLK_UpdateErrorCache], 4, sizeof(cl_mem), (void *)&m_buffers[OCLB_SelfProd]);
				assert(errorCode == CL_SUCCESS);
				errorCode = clSetKernelArg(m_kernels[OCLK_UpdateErrorCache], 5, sizeof(SharedBuffer*), (void *)&m_buffers[OCLB_ConstantBuffer]);
				assert(errorCode == CL_SUCCESS);

				errorCode = clEnqueueNDRangeKernel(m_cQueue, m_kernels[OCLK_UpdateErrorCache], 1, 0, &totalThreads, &threadsInGroup, 0, 0, 0);
				assert(errorCode == CL_SUCCESS);

			}
		}

		// Copy back results
		for(unsigned int i=0; i<m_numDevices; i++){
			m_copyBuffer[i].clear();
		}
		for(unsigned int i=0; i<m_inds.size(); i++){
			if(!m_inds[i].empty()){
				m_copyBuffer[i].assign(m_inds[i].size()*2,0);
				errorCode = clEnqueueReadBuffer(m_cQueue, m_buffers[OCLB_Output], CL_TRUE, 0, sizeof(svm_precision)*m_copyBuffer[i].size(), &m_copyBuffer[i][0], 0, 0, 0);
				assert(errorCode == CL_SUCCESS);
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

	void OpenCLSVM::updateErrorCache(svm_precision f, int i, int id){
		m_params[id].m_errors[i] = f;
	}

	ISVM::svm_precision OpenCLSVM::SVMOutput(int index, InstancePtr inst, int id){
		cl_int errorCode;

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
					errorCode = clEnqueueWriteBuffer(m_cQueue,m_buffers[OCLB_Alpha],CL_FALSE,0,sizeof(svm_precision)*m_params[id].m_alpha.size(),&m_params[id].m_alpha[0],0,NULL,NULL);
					assert(errorCode == CL_SUCCESS);
				}*/
				
				m_sharedBuffer[0].instanceCount = unsigned int(m_inds[i].size());

				errorCode = clEnqueueWriteBuffer(m_cQueue,m_buffers[OCLB_ConstantBuffer],CL_FALSE,0,sizeof(SharedBuffer),&m_sharedBuffer[0],0,NULL,NULL);
				assert(errorCode == CL_SUCCESS);
				errorCode = clEnqueueWriteBuffer(m_cQueue,m_buffers[OCLB_InputInds],CL_FALSE,0,sizeof(unsigned int)*m_inds[i].size(),&m_inds[i][0],0,NULL,NULL);
				assert(errorCode == CL_SUCCESS);

				size_t threadsInGroup = thread_group_size;
				size_t totalThreads = ((m_inds[i].size()/threadsInGroup)+1)*threadsInGroup;

				errorCode = clSetKernelArg(m_kernels[OCLK_SVMOutput], 0, sizeof(cl_mem), (void *)&m_buffers[OCLB_Alpha]);
				assert(errorCode == CL_SUCCESS);
				errorCode = clSetKernelArg(m_kernels[OCLK_SVMOutput], 1, sizeof(cl_mem), (void *)&m_buffers[OCLB_Class]);
				assert(errorCode == CL_SUCCESS);
				errorCode = clSetKernelArg(m_kernels[OCLK_SVMOutput], 2, sizeof(cl_mem), (void *)&m_buffers[OCLB_Output]);
				assert(errorCode == CL_SUCCESS);
				errorCode = clSetKernelArg(m_kernels[OCLK_SVMOutput], 3, sizeof(cl_mem), (void *)&m_buffers[OCLB_InputData]);
				assert(errorCode == CL_SUCCESS);
				errorCode = clSetKernelArg(m_kernels[OCLK_SVMOutput], 4, sizeof(cl_mem), (void *)&m_buffers[OCLB_TrainingInds]);
				assert(errorCode == CL_SUCCESS);
				errorCode = clSetKernelArg(m_kernels[OCLK_SVMOutput], 5, sizeof(cl_mem), (void *)&m_buffers[OCLB_InputInds]);
				assert(errorCode == CL_SUCCESS);
				errorCode = clSetKernelArg(m_kernels[OCLK_SVMOutput], 6, sizeof(cl_mem), (void *)&m_buffers[OCLB_SelfProd]);
				assert(errorCode == CL_SUCCESS);
				errorCode = clSetKernelArg(m_kernels[OCLK_SVMOutput], 7, sizeof(SharedBuffer*), (void *)&m_buffers[OCLB_ConstantBuffer]);
				assert(errorCode == CL_SUCCESS);

				errorCode = clEnqueueNDRangeKernel(m_cQueue, m_kernels[OCLK_SVMOutput], 1, 0, &totalThreads, &threadsInGroup, 0, 0, 0);
				assert(errorCode == CL_SUCCESS);
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
				errorCode = clEnqueueReadBuffer(m_cQueue, m_buffers[OCLB_Output], CL_TRUE, 0, sizeof(svm_precision)*m_copyBuffer[i].size(), &m_copyBuffer[i][0], 0, 0, 0);
				assert(errorCode == CL_SUCCESS);
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

	void OpenCLSVM::testInstances(std::vector<svm_precision> &finalResult, int id){
		cl_int errorCode;

		for(unsigned int i=0; i<m_numDevices; i++){
			errorCode = clEnqueueWriteBuffer(m_cQueue,m_buffers[OCLB_Alpha],CL_FALSE,0,sizeof(svm_precision)*m_params[id].m_alpha.size(),&m_params[id].m_alpha[0],0,NULL,NULL);
			assert(errorCode == CL_SUCCESS);
		}

		m_inds[0].clear();
		m_inds[0].reserve(m_params[id].m_supportVectors->numElements());
		for (int i = m_params[id].m_supportVectors->getNext(-1); i != -1; i = m_params[id].m_supportVectors->getNext(i)) {
			m_inds[0].push_back(i);
		}
		
		for(unsigned int i=0; i<m_numDevices; i++){
			errorCode = clEnqueueWriteBuffer(m_cQueue,m_buffers[OCLB_InputInds],CL_FALSE,0,sizeof(unsigned int)*m_inds[0].size(),&m_inds[0][0],0,NULL,NULL);
			assert(errorCode == CL_SUCCESS);
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
		size_t threadsInGroup = thread_group_size;
		size_t totalThreads = ((m_sharedBuffer[0].instanceCount/threadsInGroup)+1)*threadsInGroup;
		for(unsigned int i=0; i<max; i++){
			for(unsigned int j=0; j<instanceMapping.size(); j++){
				if(i < instanceMapping[j].size()){
					m_sharedBuffer[0].cb_ind1 = instanceMapping[j][i];
					errorCode = clEnqueueWriteBuffer(m_cQueue,m_buffers[OCLB_ConstantBuffer],CL_FALSE,0,sizeof(SharedBuffer),&m_sharedBuffer[0],0,NULL,NULL);
					assert(errorCode == CL_SUCCESS);

					errorCode = clSetKernelArg(m_kernels[OCLK_Test], 0, sizeof(cl_mem), (void *)&m_buffers[OCLB_Output]);
					assert(errorCode == CL_SUCCESS);
					errorCode = clSetKernelArg(m_kernels[OCLK_Test], 1, sizeof(cl_mem), (void *)&m_buffers[OCLB_InputData]);
					assert(errorCode == CL_SUCCESS);
					errorCode = clSetKernelArg(m_kernels[OCLK_Test], 2, sizeof(cl_mem), (void *)&m_buffers[OCLB_TestingInds]);
					assert(errorCode == CL_SUCCESS);
					errorCode = clSetKernelArg(m_kernels[OCLK_Test], 3, sizeof(cl_mem), (void *)&m_buffers[OCLB_TrainingInds]);
					assert(errorCode == CL_SUCCESS);
					errorCode = clSetKernelArg(m_kernels[OCLK_Test], 4, sizeof(cl_mem), (void *)&m_buffers[OCLB_InputInds]);
					assert(errorCode == CL_SUCCESS);
					errorCode = clSetKernelArg(m_kernels[OCLK_Test], 5, sizeof(cl_mem), (void *)&m_buffers[OCLB_Class]);
					assert(errorCode == CL_SUCCESS);
					errorCode = clSetKernelArg(m_kernels[OCLK_Test], 6, sizeof(cl_mem), (void *)&m_buffers[OCLB_Alpha]);
					assert(errorCode == CL_SUCCESS);
					errorCode = clSetKernelArg(m_kernels[OCLK_Test], 7, sizeof(cl_mem), (void *)&m_buffers[OCLB_SelfProd]);
					assert(errorCode == CL_SUCCESS);
					errorCode = clSetKernelArg(m_kernels[OCLK_Test], 8, sizeof(SharedBuffer*), (void *)&m_buffers[OCLB_ConstantBuffer]);
					assert(errorCode == CL_SUCCESS);

					errorCode = clEnqueueNDRangeKernel(m_cQueue, m_kernels[OCLK_Test], 1, 0, &totalThreads, &threadsInGroup, 0, 0, 0);
					assert(errorCode == CL_SUCCESS);
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
					errorCode = clEnqueueReadBuffer(m_cQueue, m_buffers[OCLB_Output], CL_TRUE, 0, sizeof(svm_precision)*m_copyBuffer[j].size(), &m_copyBuffer[j][0], 0, 0, 0);
					assert(errorCode == CL_SUCCESS);
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

	void OpenCLSVM::kernelEvaluations(std::vector<int> &inds, std::vector<svm_precision> &result, int id){
		if(inds.size() < 2)
			return;

		for(unsigned int i=0; i<inds.size(); i+=2){
			result.push_back(m_params[id].m_kernel->eval(inds[i], inds[i+1], m_params[id].m_evaluation->getTrainingInstance(inds[i])));
		}
	}

	void OpenCLSVM::initializeFold(int id){
		cl_int errorCode;
		int trainingSetSize = m_params[id].m_evaluation->getNumTrainingInstances();

		m_buffers[OCLB_Output] = clCreateBuffer(m_context, CL_MEM_WRITE_ONLY, sizeof(svm_precision)*(trainingSetSize*2), NULL, &errorCode);
		assert(errorCode == CL_SUCCESS);
		
		m_buffers[OCLB_Alpha] = clCreateBuffer(m_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(svm_precision)*m_params[id].m_alpha.size(), &m_params[id].m_alpha[0], &errorCode);
		assert(errorCode == CL_SUCCESS);

		m_buffers[OCLB_InputInds] = clCreateBuffer(m_context, CL_MEM_READ_ONLY, sizeof(unsigned int)*trainingSetSize, NULL, &errorCode);
		assert(errorCode == CL_SUCCESS);

		m_buffers[OCLB_TrainingInds] = clCreateBuffer(m_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int)*trainingSetSize, &m_params[id].m_evaluation->getTrainingInds()[0], &errorCode);
		assert(errorCode == CL_SUCCESS);

		m_buffers[OCLB_TestingInds] = clCreateBuffer(m_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int)*m_params[id].m_evaluation->getTestingInds().size(), &m_params[id].m_evaluation->getTestingInds()[0], &errorCode);
		assert(errorCode == CL_SUCCESS);

		m_buffers[OCLB_Class] = clCreateBuffer(m_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(svm_precision)*trainingSetSize, &m_params[id].m_class[0], &errorCode);
		assert(errorCode == CL_SUCCESS);

		m_sharedBuffer[0].instanceCount = m_params[id].m_evaluation->getNumTrainingInstances();
		m_sharedBuffer[0].kernelParam1 = m_params[id].m_kernel->getParameter(0);
		m_sharedBuffer[0].kernelParam2 = m_params[id].m_kernel->getParameter(1);
		m_sharedBuffer[0].kernel = m_params[id].m_kernel->getId();
	}

	void OpenCLSVM::endFold(int id){
		clReleaseMemObject(m_buffers[OCLB_Output]);
		clReleaseMemObject(m_buffers[OCLB_Alpha]);
		clReleaseMemObject(m_buffers[OCLB_InputInds]);
		clReleaseMemObject(m_buffers[OCLB_TrainingInds]);
		clReleaseMemObject(m_buffers[OCLB_TestingInds]);
		clReleaseMemObject(m_buffers[OCLB_Class]);

		m_buffers.erase(OCLB_Output);
		m_buffers.erase(OCLB_Alpha);
		m_buffers.erase(OCLB_InputInds);
		m_buffers.erase(OCLB_TrainingInds);
		m_buffers.erase(OCLB_TestingInds);
		m_buffers.erase(OCLB_Class);
	}

	void OpenCLSVM::checkError(cl_int error){
		std::string errorDesc = "";
		switch(error){
		case CL_INVALID_PROGRAM_EXECUTABLE:
			errorDesc = "CL_INVALID_PROGRAM_EXECUTABLE";
			break;
		case CL_INVALID_COMMAND_QUEUE:
			errorDesc = "CL_INVALID_COMMAND_QUEUE";
			break;
		case CL_INVALID_KERNEL:
			errorDesc = "CL_INVALID_KERNEL";
			break;
		case CL_INVALID_CONTEXT:
			errorDesc = "CL_INVALID_CONTEXT";
			break;
		case CL_INVALID_KERNEL_ARGS:
			errorDesc = "CL_INVALID_KERNEL_ARGS";
			break;
		case CL_INVALID_WORK_DIMENSION:
			errorDesc = "CL_INVALID_WORK_DIMENSION";
			break;
		case CL_INVALID_GLOBAL_WORK_SIZE:
			errorDesc = "CL_INVALID_GLOBAL_WORK_SIZE";
			break;
		case CL_INVALID_GLOBAL_OFFSET:
			errorDesc = "CL_INVALID_GLOBAL_OFFSET";
			break;
		case CL_INVALID_WORK_GROUP_SIZE:
			errorDesc = "CL_INVALID_WORK_GROUP_SIZE";
			break;
		case CL_INVALID_WORK_ITEM_SIZE:
			errorDesc = "CL_INVALID_WORK_ITEM_SIZE";
			break;
		case CL_MISALIGNED_SUB_BUFFER_OFFSET:
			errorDesc = "CL_MISALIGNED_SUB_BUFFER_OFFSET";
			break;
		case CL_INVALID_IMAGE_SIZE:
			errorDesc = "CL_INVALID_IMAGE_SIZE";
			break;
		case CL_OUT_OF_RESOURCES:
			errorDesc = "CL_OUT_OF_RESOURCES";
			break;
		case CL_MEM_OBJECT_ALLOCATION_FAILURE:
			errorDesc = "CL_MEM_OBJECT_ALLOCATION_FAILURE";
			break;
		case CL_INVALID_EVENT_WAIT_LIST:
			errorDesc = "CL_INVALID_EVENT_WAIT_LIST";
			break;
		case CL_OUT_OF_HOST_MEMORY:
			errorDesc = "CL_OUT_OF_HOST_MEMORY";
			break;
		default:
			errorDesc = "UNKNOWN";
			break;
		}
	}
}