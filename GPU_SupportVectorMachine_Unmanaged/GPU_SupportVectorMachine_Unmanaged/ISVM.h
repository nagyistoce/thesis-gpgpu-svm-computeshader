#pragma once
#include "IAlgorithm.h"
#include "IKernel.h"
#include "SMOSet.h"

namespace SVM_Framework{
	class ISVM : public IAlgorithm{
	public:
		ISVM();
		~ISVM(){}

		void run(AlgorithmDataPackPtr data);
	protected:
		virtual void execute() = 0;
		virtual void lagrangeThresholdUpdate(double p1, double p2, int id, int i1, int i2) = 0;
		virtual void initializeFold(int id) = 0;
		virtual void endFold(int id) = 0;
		virtual void updateErrorCache(float f, int i, int id) = 0;
		virtual double SVMOutput(int index, InstancePtr inst, int id) = 0;
		virtual void testInstances(int id) = 0;

		int examineExample(int i2, int id);
		int takeStep(int i1, int i2, double F2, int id);

		void initialize(unsigned int id);
		void executeFold(unsigned int id);

		double m_C;
		double m_eps;
		double m_tol;
		double m_Del;
		bool m_KernelIsLinear;

		struct AlgoParams{
			std::vector<float> m_errors;
			std::vector<float> m_class;

			std::vector<float> m_alpha;
			float m_b, m_bLow, m_bUp;
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

			unsigned int	cl1Correct,
							cl1Wrong,
							cl2Correct,
							cl2Wrong;

			IKernelPtr m_kernel;
			IEvaluationPtr m_evaluation;

			unsigned int iterations;
		};

		std::vector<AlgoParams> m_params;

		DataDocumentPtr m_document;
		AlgorithmDataPackPtr m_data;
	};
}