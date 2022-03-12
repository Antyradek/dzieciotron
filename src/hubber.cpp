#include "hubber.hpp"
#include <thread>
#include "logger.hpp"
#include "exceptions.hpp"

using namespace externals;
using namespace logger;

Hubber::Hubber(const defines::HubParams& params):
BaseHubber(),
params(params),
isResetting(false),
usbContext(nullptr, libusb_exit),
usbDevices(nullptr, std::bind(libusb_free_device_list, std::placeholders::_1, true)),
usbDevicesCount(0),
hardwareHub(nullptr),
softwareHub(nullptr)
{
	this->pause(true);
	
	//kontekst USB
	this->usbContext.reset([](){
		libusb_context* context;
		const int err = libusb_init(&context);
		if(err != 0)
		{
			throw(UsbError("Błąd inicjalizacji libusb"));
		}
		return(context);
	}());
	
	//lista urządzeń USB
	this->usbDevices.reset([this](){
		libusb_device** devices;
		const int devcount = libusb_get_device_list(this->usbContext.get(), &devices);
		if(devcount <= 0)
		{
			throw(UsbError("Błąd odnajdywania urządzeń USB"));
		}
		this->usbDevicesCount = devcount;
		return(devices);
	}());
	
	//szukaj urządzeń
	for(size_t i = 0; i < this->usbDevicesCount; i++)
	{
		libusb_device_descriptor descriptor;
		const int errcode = libusb_get_device_descriptor(this->usbDevices[i], &descriptor);
		if(errcode != 0)
		{
			throw(UsbError("Błąd pobierania deskryptora"));
		}
		if(descriptor.idVendor == this->params.hardwareVidPid.first && descriptor.idProduct == this->params.hardwareVidPid.second)
		{
			this->hardwareHub = this->usbDevices[i];
		}
		else if(descriptor.idVendor == this->params.softwareVidPid.first && descriptor.idProduct == this->params.softwareVidPid.second)
		{
			this->softwareHub = this->usbDevices[i];
		}
	}
}

void Hubber::setPower(struct libusb_device* hub, bool turnOn)
{
	std::unique_ptr<libusb_device_handle, std::function<void(libusb_device_handle*)>> device([&hub](){
		libusb_device_handle* handle;
		const int outerr = libusb_open(hub, &handle);
		if(outerr != 0)
		{
			throw(UsbError("Błąd uzyskiwania uchwytu urządzenia USB"));
		}
		return(handle);
	}(), libusb_close);
	
	//uchwyt, typ, żądanie, flaga_featura, port, dane, wielkość_danych, timeout_ms
	const libusb_standard_request requestType = turnOn ? LIBUSB_REQUEST_SET_FEATURE : LIBUSB_REQUEST_CLEAR_FEATURE;
	const int outerr = libusb_control_transfer(device.get(), LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_OTHER, requestType, this->params.powerFlag, this->params.port, nullptr, 0, std::chrono::milliseconds(defines::usbSetPowerTimeout).count());
	if(outerr < 0)
	{
		throw(UsbError("Błąd wysyłania komendy kontroli USB"));
	}
	libusb_reset_device(device.get());
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
	this->setPower(this->softwareHub, false);
	this->setPower(this->hardwareHub, false);
	Logger::debug() << "Odłączono zasilanie USB";
	this->setPower(this->softwareHub, true);
	this->setPower(this->hardwareHub, true);
	Logger::debug() << "Podłączono zasilanie USB";
	//czekaj na aktywację
	std::this_thread::sleep_for(defines::cameraBootTime);
	Logger::debug() << "Zrestartowano hub USB";
	
	//deaktywacja włączania
	this->pause(true);
	this->isResetting = false;
}

