
#if ARDUINO_USB_MODE
#warning This sketch should be used when USB is in OTG mode
void setup(){}
void loop(){}
#else
#include "USB.h"
#include "SD.h"
#include "SPI.h"
#include "USBMSC.h"

#if ARDUINO_USB_CDC_ON_BOOT
#define HWSerial Serial0
#define USBSerial Serial
#else
#define HWSerial Serial
USBCDC USBSerial;
#endif

USBMSC MSC;

#define SD_MISO  13//37 io13
#define SD_MOSI  11//35 io11 
#define SD_SCK   12//36 gpio12
#define SD_CS    10//34 gpio10

static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize)
{
    //log("write");
    HWSerial.printf("MSC WRITE: lba: %u, offset: %u, bufsize: %u\n", lba, offset, bufsize);
    SD.writeRAW((uint8_t*)buffer, lba);
    return bufsize;
}

static int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
{
    //log("read");
    HWSerial.printf("MSC READ: lba: %u, offset: %u, bufsize: %u\n", lba, offset, bufsize);
    SD.readRAW((uint8_t*)buffer, lba);
    return bufsize;
}

static bool onStartStop(uint8_t power_condition, bool start, bool load_eject){
  HWSerial.printf("MSC START/STOP: power: %u, start: %u, eject: %u\n", power_condition, start, load_eject);
  return true;
}

void log(const char* str)
{
    Serial.println(str);
}

static void usbEventCallback(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
  if(event_base == ARDUINO_USB_EVENTS){
    arduino_usb_event_data_t * data = (arduino_usb_event_data_t*)event_data;
    switch (event_id){
      case ARDUINO_USB_STARTED_EVENT:
        HWSerial.println("USB PLUGGED");
        break;
      case ARDUINO_USB_STOPPED_EVENT:
        HWSerial.println("USB UNPLUGGED");
        break;
      case ARDUINO_USB_SUSPEND_EVENT:
        HWSerial.printf("USB SUSPENDED: remote_wakeup_en: %u\n", data->suspend.remote_wakeup_en);
        break;
      case ARDUINO_USB_RESUME_EVENT:
        HWSerial.println("USB RESUMED");
        break;
      
      default:
        break;
    }
  }
}

void setup()
{
    HWSerial.begin(115200);
    HWSerial.setDebugOutput(true);

    static SPIClass* spi = NULL;
    spi = new SPIClass(FSPI);
    spi->begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    if (SD.begin(SD_CS, *spi, 40000000))
    {
      HWSerial.println("SD begin");
    }

//    SPIClass spi(FSPI);
//    spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
//    if(SD.begin(SD_CS, spi, 40000000))
//    {
//        log("SD begin");
//    }

    USB.onEvent(usbEventCallback);
    
    MSC.vendorID("JTI");
    MSC.productID("MSC-ADS");
    MSC.productRevision("1.0");
    MSC.onStartStop(onStartStop);
    MSC.onRead(onRead);
    MSC.onWrite(onWrite);
    
    MSC.mediaPresent(true);
    
    MSC.begin(SD.cardSize(), 512);
    

    USBSerial.begin();
    USB.begin();
    
    char buff[50];
    sprintf(buff, "Storage Size: %d, Block: %d", SD.totalBytes() / 1024, SD.cardSize());
    HWSerial.println(buff);
}

void loop()
{
    // put your main code here, to run repeatedly:
    //delay(200);
}

#endif /* ARDUINO_USB_MODE */
