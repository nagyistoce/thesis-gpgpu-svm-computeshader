#pragma once
#include "IAlgorithm.h"
#include "IKernel.h"
#include "SMOSet.h"
#include "Value.h"

namespace SVM_Framework{
	class ISVM : public IAlgorithm{
	public:
		typedef Value::v_precision svm_precision;

		ISVM();
		~ISVM(){}

		struct algorithmParams{
			svm_precision	s_bLow,
							s_bUp;
			int		s_iLow,
					s_iUp;
		};

		void run(AlgorithmDataPackPtr data);
	protected:
		struct SharedBuffer{
			svm_precision	kernelParam1;
			svm_precision	kernelParam2;
			unsigned int	instanceLength;
			unsigned int	instanceCount;
			unsigned int	classIndex;
			unsigned int	kernel;
			svm_precision	param1;
			svm_precision	param2;
			int				cb_ind1;
			int				cb_ind2;
		};

		void meassurements(std::vector<svm_precision> &results, int id);
		void execute();
		virtual void beginExecute() = 0;
		virtual void endExecute() = 0;

		virtual void lagrangeThresholdUpdate(svm_precision p1, svm_precision p2, int id, int i1, int i2) = 0;
		virtual void initializeFold(int id) = 0;
		virtual void endFold(int id) = 0;
		virtual void updateErrorCache(svm_precision f, int i, int id) = 0;
		virtual svm_precision SVMOutput(int index, InstancePtr inst, int id) = 0;
		virtual void testInstances(std::vector<svm_precision> &finalResult, int id) = 0;
		virtual void kernelEvaluations(std::vector<int> &inds, std::vector<svm_precision> &result, int id) = 0;

		int examineExample(int i2, int id);
		int takeStep(int i1, int i2, svm_precision F2, int id);

		void initialize(unsigned int id);
		void executeStage(unsigned int id);
		void resultOutput(bool final, unsigned int id);

		svm_precision m_C;
		svm_precision m_eps;
		svm_precision m_tol;
		svm_precision m_Del;
		bool m_multiThreaded;

		struct AlgoParams{
			std::vector<svm_precision> m_errors;
			std::vector<svm_precision> m_class;

			std::vector<svm_precision> m_alpha;
			svm_precision m_b, m_bLow, m_bUp;
			int m_iLow, m_iUp;

			/* The five different sets used by the algorithm. */
			/** {i: 0 < m_alpha[i] < C} */
			SMOSetPtr m_I0;
			/**  {i: m_class[i] = 1, m_alpha[i] = 0} */
			SMOSetPtr m_I1; 
			/**  {i: m_class[i] = -1, m_alpha[i] =C} */
			SMOSetPtr m_I2; 
			/** {i: m_class[i] = 1, m_alpha[i] = C} */
			SMOSetPtr m_I3;
			/**  {i: m_class[i] = -1, m_alpha[i] = 0} */
			SMOSetPtr m_I4; 

			/** The set of support vectors */
			SMOSetPtr m_supportVectors; // {i: 0 < m_alpha[i]}

			IKernelPtr m_kernel;
			IEvaluationPtr m_evaluation;

			ResultsPack m_resultPack,
						m_finalResultPack;
		};

		std::vector<AlgoParams> m_params;

		std::vector<SharedBuffer> m_sharedBuffer;

		DataDocumentPtr m_document;
		AlgorithmDataPackPtr m_data;

		std::vector<svm_precision> finalResult;
		std::vector<std::vector<Value::v_precision>>	m_copyBuffer;
		std::vector<std::vector<unsigned int>>			m_inds;

		std::vector<std::pair<int,double>> m_cachedResults;

		bool m_alphaTransferNeeded;
	};
}