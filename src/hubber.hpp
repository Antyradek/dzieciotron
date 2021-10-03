#pragma once
#include <atomic>
#include <mutex>
#include <memory>
#include <functional>
#include <libusb-1.0/libusb.h>
#include "task.hpp"
#include "defines.hpp"

namespace externals
{
/// Pozwala kontrolować białe oświetlenie w łagodny sposób
class Hubber: public dzieciotron::AsyncTask
{
private:
	/// Parametry
	const defines::HubParams& params;
	
	/// Flaga czy resetuje
	std::atomic_bool isResetting;
	
	/// Kontekst libusb
	std::unique_ptr<libusb_context, std::function<void(libusb_context*)>> usbContext;
	
	/// Lista urządzeń USB
	std::unique_ptr<libusb_device*[], std::function<void(libusb_device*[])>> usbDevices;
	
	/// Ilość urządzeń USB
	size_t usbDevicesCount;
	
	/// Wskaźnik na sprzętowy hub USB
	libusb_device* hardwareHub;
	
	/// Wskaźnik na systemowy hub USB
	libusb_device* softwareHub;
	
	/// Ustaw zasilanie podanego huba
	void setPower(struct libusb_device* hub, bool turnOn);
	
	/// Główna pętla
	void runLoop() override;
	
public:
	/// Startuje w wyłączonym stanie
	Hubber(const defines::HubParams& params);
	
	/// Czeka na zakończenie
	virtual ~Hubber() {};
	
	/// Resetuj wszystkie kamery, może być wołane z wielu wątków
	void reset();
};
}

