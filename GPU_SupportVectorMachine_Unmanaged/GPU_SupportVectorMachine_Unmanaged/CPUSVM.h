#pragma once
#include "ISVM.h"
#include "GraphicsManager.h"

namespace SVM_Framework{
	class CPUSVM : public ISVM{
	public:
		CPUSVM(GraphicsManagerPtr dxMgr);
		~CPUSVM();
	
	protected:
		void execute();
		void initialize(unsigned int id);

		int examineExample(int i2, unsigned int id);
		int takeStep(int i1, int i2, double F2, unsigned int id);
		double SVMOutput(int index, InstancePtr inst, unsigned int id);
	private:
		void executeFold(unsigned int id);

		/** The complexity parameter. */
		double m_C;
		/** Epsilon for rounding. */
		double m_eps;
		/** Tolerance for accuracy of result. */
		double m_tol;
		double m_Del;
		bool m_KernelIsLinear;

		struct AlgoParams{
			std::vector<double> m_errors;
			std::vector<double> m_class;
			std::vector<double> m_weights;

			/** The Lagrange multipliers. */
			std::vector<double> m_alpha;
			/** The thresholds. */
			double m_b, m_bLow, m_bUp;
			/** The indices for m_bLow and m_bUp */
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
		};

		std::vector<AlgoParams> m_params;
		std::vector<boost::shared_ptr<boost::thread>> m_threads;
	};
}