// SimpleRx - the slave or the receiver

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN  D2
#define CSN_PIN D4


typedef struct{
  char address[6];
} NRF24_address;

const NRF24_address TX_addresses[6] = {"0STUD", "1STUD", "2STUD", "3STUD", "4STUD", "5STUD"};
// uint64_t address[6] = { 0x7878787878LL,
//                         0xB3B4B5B6F1LL,
//                         0xB3B4B5B6CDLL,
//                         0xB3B4B5B6A3LL,
//                         0xB3B4B5B60FLL,
//                         0xB3B4B5B605LL };

RF24 radio(CE_PIN, CSN_PIN);
uint8_t pipe;

char dataReceived[10]; // this must match dataToSend in the TX
bool newData = false;
struct aht21Data {
  float temperature;
  float humidity;
} aht21Data;

void setup() {

    Serial.begin(9600);

    Serial.println("SimpleRx Starting");
    radio.begin();
    radio.setDataRate( RF24_250KBPS );
    radio.setPALevel(RF24_PA_HIGH);
    radio.setAutoAck(true);
    radio.setPayloadSize(sizeof(aht21Data));
    // radio.openReadingPipe(1, thisSlaveAddress);

    // radio.openReadingPipe(0, address[0]);
    // radio.openReadingPipe(1, address[1]);

    // radio.openReadingPipe(0, (byte*)(TX_addresses[0].address));
    // radio.openReadingPipe(1, (byte*)(TX_addresses[1].address));
    for (uint8_t i = 0; i < 6; ++i) {
      radio.openReadingPipe(i, (byte*)(TX_addresses[i].address));
    }
    radio.startListening();
}

void loop() {
    getData();
    showData();
}

void getData() {
    if (radio.available(&pipe)) {              // is there a payload? get the pipe number that recieved it
      Serial.println("Got something");
      uint8_t bytes = radio.getPayloadSize();  // get the size of the payload
      radio.read(&aht21Data, bytes);             // fetch payload from FIFO
      newData = true;
    }
    // if ( radio.available() ) {
    //     // radio.read( &dataReceived, sizeof(dataReceived) );
    //     radio.read( &aht21Data, sizeof(aht21Data) );
    //     newData = true;
    // }
}

void getDataMultiplePipes() {

}

void showData() {
    if (newData == true) {
        Serial.print("Data received from pipe: ");
        Serial.println(pipe);
        Serial.print("  Temperature: ");
        Serial.println(aht21Data.temperature);
        Serial.print("  Humidity: ");
        Serial.println(aht21Data.humidity);
        newData = false;
    }
}
