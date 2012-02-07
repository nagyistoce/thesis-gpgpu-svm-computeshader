#pragma once

namespace SVM_Framework{
	class ConfigManager{
	public:
		static void initialize();
		static unsigned int startTimer();
		static void removeTimer(unsigned int timer);
		static void resetTimer(unsigned int timer);
		static double getTime(unsigned int timer);

		static std::string getSetting(std::string setting);
	private:
		ConfigManager();

		static std::map<unsigned int,boost::shared_ptr<boost::timer>> m_timers;
		static std::map<std::string,std::string> m_configValues;
		static unsigned int m_timerId;
	};
}