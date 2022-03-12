#pragma once
#include "task.hpp"

namespace externals
{
/// Bazowa klasa do kontroli białego oświetlenia
class BaseHubber: public dzieciotron::AsyncTask
{
public:
	/// Domyślny
	BaseHubber() {};
	
	/// Czeka na zakończenie
	virtual ~BaseHubber() {};
	
	/// Resetuj wszystkie kamery, może być wołane z wielu wątków
	virtual void reset() = 0;
};
}


