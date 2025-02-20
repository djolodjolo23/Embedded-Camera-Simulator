#include <Arduino.h>
#include <test_augmented_inferencing.h>
#include <eloquent_esp32cam.h>
#include <eloquent_esp32cam/edgeimpulse/fomo.h>

#define COLLECT_IMG 1
#define RUN_INFERENCE 0

#if COLLECT_IMG 

#define WIFI_SSID "Tele2_108703"
#define WIFI_PASS "q2yymgzk"
#define HOSTNAME "esp32cam"
#include <eloquent_esp32cam.h>
#include <eloquent_esp32cam/extra/esp32/wifi/sta.h>
#include <eloquent_esp32cam/viz/image_collection.h>

using eloq::camera;
using eloq::wifi;
using eloq::viz::collectionServer;


void setup() {
    delay(3000);
    Serial.begin(9600);
    Serial.println("___IMAGE COLLECTION SERVER___");

    // camera settings
    // replace with your own model!
    camera.pinout.aithinker();
    camera.brownout.disable();
    // Edge Impulse models work on square images
    // face resolution is 240x240
    camera.resolution.face();
    camera.quality.high();

    // init camera
    while (!camera.begin().isOk())
        Serial.println(camera.exception.toString());

    // connect to WiFi
    while (!wifi.connect().isOk())
      Serial.println(wifi.exception.toString());

    // init face detection http server
    while (!collectionServer.begin().isOk())
        Serial.println(collectionServer.exception.toString());

    Serial.println("Camera OK");
    Serial.println("WiFi OK");
    Serial.println("Image Collection Server OK");
    Serial.println(collectionServer.address());
}


void loop() {
    // server runs in a separate thread, no need to do anything here
}
#endif


#if RUN_INFERENCE
using eloq::camera;
using eloq::ei::fomo;


/**
 * 
 */
void setup() {
    delay(3000);
    Serial.begin(9600);
    Serial.println("__EDGE IMPULSE FOMO (NO-PSRAM)__");

    // camera settings
    // replace with your own model!
    camera.pinout.aithinker();
    camera.brownout.disable();
    // NON-PSRAM FOMO only works on 96x96 (yolo) RGB565 images
    camera.resolution.yolo();
    camera.pixformat.rgb565();

    // init camera
    while (!camera.begin().isOk())
        Serial.println(camera.exception.toString());

    Serial.println("Camera OK");
    Serial.println("Put object in front of camera");
}


void loop() {
    // capture picture
    if (!camera.capture().isOk()) {
        Serial.println(camera.exception.toString());
        return;
    }

    // run FOMO
    if (!fomo.run().isOk()) {
      Serial.println(fomo.exception.toString());
      return;
    }

    // how many objects were found?
    Serial.printf(
      "Found %d object(s) in %dms\n", 
      fomo.count(),
      fomo.benchmark.millis()
    );

    // if no object is detected, return
    if (!fomo.foundAnyObject())
      return;

    // if you expect to find a single object, use fomo.first
    Serial.printf(
      "Found %s at (x = %d, y = %d) (size %d x %d). "
      "Proba is %.2f\n",
      fomo.first.label,
      fomo.first.x,
      fomo.first.y,
      fomo.first.width,
      fomo.first.height,
      fomo.first.proba
    );

    // if you expect to find many objects, use fomo.forEach
    if (fomo.count() > 1) {
      fomo.forEach([](int i, bbox_t bbox) {
        Serial.printf(
          "#%d) Found %s at (x = %d, y = %d) (size %d x %d). "
          "Proba is %.2f\n",
          i + 1,
          bbox.label,
          bbox.x,
          bbox.y,
          bbox.width,
          bbox.height,
          bbox.proba
        );
      });
    }
}
#endif