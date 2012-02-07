#pragma once

namespace SVM_Framework{
	class SMOset{
	public:
		SMOset(unsigned int size){
			m_indicators.assign(size,false);
			m_next.assign(size,0);
			m_previous.assign(size,0);
			m_number = 0;
			m_first = -1;
		}

		bool contains(unsigned int index){
			return m_indicators[index];
		}

		void remove(int index) {
			if (m_indicators[index]) {
				if (m_first == index) {
					m_first = m_next[index];
				} 
				else {
					m_next[m_previous[index]] = m_next[index];
				}
				if (m_next[index] != -1) {
					m_previous[m_next[index]] = m_previous[index];
				}
				m_indicators[index] = false;
				m_number--;
			}
		}

		void insert(int index) {
			if (!m_indicators[index]) {
				if (m_number == 0) {
					m_first = index;
					m_next[index] = -1;
					m_previous[index] = -1;
				}
				else{
					m_previous[m_first] = index;
					m_next[index] = m_first;
					m_previous[index] = -1;
					m_first = index;
				}
				m_indicators[index] = true;
				m_number++;
			}
		}

		int getNext(int index) {
			if (index == -1) {
				return m_first;
			} 
			else {
				return m_next[index];
			}
		}

		int numElements() {
			return m_number;
		}
	private:
		/** The current number of elements in the set */
		int m_number;
		/** The first element in the set */
		int m_first;
		/** Indicators */
		std::vector<bool> m_indicators;
		/** The next element for each element */
		std::vector<int> m_next;
		/** The previous element for each element */
		std::vector<int> m_previous;
	};
}

typedef boost::shared_ptr<SVM_Framework::SMOset> SMOSetPtr;