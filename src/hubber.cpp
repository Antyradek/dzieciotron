#include "hubber.hpp"
#include "logger.hpp"
#include "exceptions.hpp"

using namespace externals;
using namespace logger;

Hubber::Hubber(const defines::HubParams& params):
params(params),
isResetting(false),
usbContext(nullptr, libusb_exit)
{
	this->pause(true);
	
	this->usbContext.reset([](){
		libusb_context* context;
		const int err = libusb_init(&context);
		if(err != 0)
		{
			throw UsbError("Błąd inicjalizacji libusb");
		}
		return(context);
	}());
	
	/*
	struct libusb_device **usb_devs = nullptr;
	//kontekst
	int devs = libusb_get_device_list(nullptr, &usb_devs);
	assert(devs > 0);
	struct libusb_device* hub = nullptr;
	struct libusb_device* unixhub = nullptr;
	for(int i = 0; i < devs; i++)
	{
		struct libusb_device_descriptor desc;
		err = libusb_get_device_descriptor(usb_devs[i], &desc);
		assert(err == 0); 
		if(desc.idVendor == 0x2109 && desc.idProduct == 0x3431)
		{
			hub = usb_devs[i];
		}
		if(desc.idVendor == 0x1D6B && desc.idProduct == 0x0003)
		{
			unixhub = usb_devs[i];
		}
	}
	assert(hub != nullptr);
	assert(unixhub != nullptr);
	
	setOn(unixhub, {1}, false);
	setOn(hub, {1}, false);
	std::cout << "Wyłączono" << std::endl;
	// 	std::this_thread::sleep_for (std::chrono::seconds(1));
	setOn(unixhub, {1}, true);
	setOn(hub, {1}, true);
	std::cout << "Włączono" << std::endl;
	
	
	//czy zwolnić poprawnie
	libusb_free_device_list(usb_devs, true);
	*/
	
}

void Hubber::reset()
{
	if(!this->isResetting)
	{
		this->isResetting = true;
		this->pause(false);
	}
}

void Hubber::runLoop()
{
	//rozpoczynamy reset
	Logger::debug() << "Restartowanie huba USB...";
	
}

