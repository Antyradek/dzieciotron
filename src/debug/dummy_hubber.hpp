#pragma once
#include "task.hpp"
#include "base_hubber.hpp"

namespace debug
{
	/// Bazowa klasa do kontroli białego oświetlenia
	class DummyHubber: public externals::BaseHubber
	{
	private:
		void runLoop() override {};
		
	public:
		/// Domyślny
		DummyHubber(): externals::BaseHubber() {};
		
		/// Czeka na zakończenie
		virtual ~DummyHubber() {};
		
		/// Nic nie robi
		void reset() override {};
	};
}
