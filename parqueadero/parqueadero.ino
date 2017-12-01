/* Motor Paso a Paso ajustado a grados
  by: www.elprofegarcia.com

  Motor X             Motor Y
  Arduino    Driver   Arduino      Driver
  8          IN4    2             IN4
  9          IN3    3             IN3
  10         IN2    4             IN2
  11         IN1    5             IN1

  Servo
  Arduino
  6
*/

// LIBRARIES.
#include <SPI.h>
#include <MFRC522.h>

// ARDUINO PINS.
constexpr uint8_t RST_PIN = 6;     // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 7;     // Configurable, see typical pin layout above

// RFID class.
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

// Key used for rfid tags.
MFRC522::MIFARE_Key key;

// CONSTANTS.
// Number of bytes in a UID.
const byte UID_BYTE_COUNT = 4;
// Numbers for ownership.
enum title {
  TITLE_NONE = 0
    , TITLE_HOST = 1
    , TITLE_GUEST = 2
};
// Number of existent hosts.
const byte HOST_COUNT = 2;
// Number of existent guests.
const byte GUEST_COUNT = 2;
// Number of existent clients.
const byte CLIENT_COUNT = HOST_COUNT + GUEST_COUNT;

struct uid {
  const title ownership;
  const byte key[UID_BYTE_COUNT];
  const byte assignedSpace;
  bool inLot;
};

// Collections to store unique rfid identifiers.
uid CLIENT_UID[CLIENT_COUNT] = {
  {TITLE_HOST, {54, 228, 179, 121}, 1, false}
  , {TITLE_HOST, {144, 231, 136, 73}, 2, false}
  , {TITLE_GUEST, {38, 248, 82, 165}, 3, false}
  , {TITLE_GUEST, {22, 16, 175, 121}, 4, false}
};

// Init array that will store new NUID.
byte nuidPICC[4];

#include <Servo.h>
Servo servo;
int pos = 0;
int retardo = 2;
int i = 0;

void setup() {
  Serial.begin(9600);     // inicializamos el puerto serie a 9600 baudios
  servo.attach(6);
  pinMode(1, INPUT);
  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(11, OUTPUT);    // Pin 11 conectar a IN4
  pinMode(10, OUTPUT);    // Pin 10 conectar a IN3
  pinMode(9, OUTPUT);     // Pin 9 conectar a IN2
  pinMode(8, OUTPUT);     // Pin 8 conectar a IN1

  
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522

  // Fill key with empty values.
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  // Display basic information.
  Serial.println(F("This code scans the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  Serial.println("Please enter valid RFID tag.");
  Serial.println();
}

void loop() {
  
  // Look for new cards.
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed.
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  // Check if the PICC is of Classic MIFARE type.
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  // Store NUID into nuidPICC array.
  for (byte i = 0; i < UID_BYTE_COUNT; i++) {
    nuidPICC[i] = rfid.uid.uidByte[i];
  }

  // Display RFID UID.
  Serial.println(F("The NUID tag is:"));
  Serial.print(F("In dec: "));
  printDec(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();

  // Display the ownership status of the RFID tag.
  int rfidNumber = GetUidNumber(rfid.uid.uidByte);
  uid rfidTag = CLIENT_UID[rfidNumber];
  DisplayOwnership(rfidTag);

  // Change lot status.
  CLIENT_UID[rfidNumber].inLot = ! rfidTag.inLot;
  Serial.println();
  Serial.println();

  // Halt PICC.
  rfid.PICC_HaltA();

  // Stop encryption on PCD.
  rfid.PCD_StopCrypto1();
  
  x_der();
  apagado_x();
  x_izq();
  apagado_x();
  /*y_der();
    apagado_y();
    y_izq();
    apagado_y();    */
  // Apagado del Motor para que no se caliente
}  ///////////////////// Fin del Loop ///////////////////////////

void x_der() {        // Pasos a la derecha
  for (i = 0; i < 512; i++) {
    digitalWrite(11, HIGH);
    digitalWrite(10, HIGH);
    digitalWrite(9, LOW);
    digitalWrite(8, LOW);
    delay(retardo);
    digitalWrite(11, LOW);
    digitalWrite(10, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(8, LOW);
    delay(retardo);
    digitalWrite(11, LOW);
    digitalWrite(10, LOW);
    digitalWrite(9, HIGH);
    digitalWrite(8, HIGH);
    delay(retardo);
    digitalWrite(11, HIGH);
    digitalWrite(10, LOW);
    digitalWrite(9, LOW);
    digitalWrite(8, HIGH);
    delay(retardo);
  }
}

void x_izq() {        // Pasos a la izquierda
  for (i = 0; i < 512; i++) {
    digitalWrite(11, LOW);
    digitalWrite(10, LOW);
    digitalWrite(9, HIGH);
    digitalWrite(8, HIGH);
    delay(retardo);
    digitalWrite(11, LOW);
    digitalWrite(10, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(8, LOW);
    delay(retardo);
    digitalWrite(11, HIGH);
    digitalWrite(10, HIGH);
    digitalWrite(9, LOW);
    digitalWrite(8, LOW);
    delay(retardo);
    digitalWrite(11, HIGH);
    digitalWrite(10, LOW);
    digitalWrite(9, LOW);
    digitalWrite(8, HIGH);
    delay(retardo);
  }
}

void apagado_x() {         // Apagado del Motor
  digitalWrite(11, LOW);
  digitalWrite(10, LOW);
  digitalWrite(9, LOW);
  digitalWrite(8, LOW);
}

void y_der() {        // Pasos a la derecha
  for (i = 0; i < 512; i++) {
    digitalWrite(5, HIGH);
    digitalWrite(4, HIGH);
    digitalWrite(3, LOW);
    digitalWrite(2, LOW);
    delay(retardo);
    digitalWrite(5, LOW);
    digitalWrite(4, HIGH);
    digitalWrite(3, HIGH);
    digitalWrite(2, LOW);
    delay(retardo);
    digitalWrite(5, LOW);
    digitalWrite(4, LOW);
    digitalWrite(3, HIGH);
    digitalWrite(2, HIGH);
    delay(retardo);
    digitalWrite(5, HIGH);
    digitalWrite(4, LOW);
    digitalWrite(3, LOW);
    digitalWrite(2, HIGH);
    delay(retardo);
  }
}

void y_izq() {        // Pasos a la izquierda
  for (i = 0; i < 512; i++) {
    digitalWrite(5, LOW);
    digitalWrite(4, LOW);
    digitalWrite(3, HIGH);
    digitalWrite(2, HIGH);
    delay(retardo);
    digitalWrite(5, LOW);
    digitalWrite(4, HIGH);
    digitalWrite(3, HIGH);
    digitalWrite(2, LOW);
    delay(retardo);
    digitalWrite(5, HIGH);
    digitalWrite(4, HIGH);
    digitalWrite(3, LOW);
    digitalWrite(2, LOW);
    delay(retardo);
    digitalWrite(5, HIGH);
    digitalWrite(4, LOW);
    digitalWrite(3, LOW);
    digitalWrite(2, HIGH);
    delay(retardo);
  }
}

void apagado_y() {         // Apagado del Motor
  digitalWrite(5, LOW);
  digitalWrite(4, LOW);
  digitalWrite(3, LOW);
  digitalWrite(2, LOW);
}

// FUNCTIONS.
// Get uid number o uid tag.
int GetUidNumber(byte uidTag[UID_BYTE_COUNT])
{
  // RFID values to return.
  int uidNumber = -1;

  // Verify if UID exists.
  for (byte client = 0; client < CLIENT_COUNT; client++) {
    bool currentClientIsUid = true;

    // Loop trough all bytes of the current client ID.
    for (byte currentByte = 0; currentByte < UID_BYTE_COUNT; currentByte++) {
      byte currentClientUidByte = CLIENT_UID[client].key[currentByte];
      byte currentUidByte = uidTag[currentByte];
      if (currentClientUidByte != currentUidByte) {
        currentClientIsUid = false;
        currentByte = UID_BYTE_COUNT;
      }
    }

    // If the current client ID is the same as the UID,
    // define UID as a client and end the loop.
    if (currentClientIsUid) {
      uidNumber = client;
      client = CLIENT_COUNT;
    }
  }

  // Return values for rfid tag.
  return uidNumber;
}

// Verify if RFID is valid for a client.
void DisplayOwnership(uid uidTag)
{
  // Define ownership to display.
  String ownership = "does not exist.";

  // Define text if UID is a host or guest.
  if (uidTag.ownership == TITLE_HOST)
    ownership = "is for a host.";
  else if (uidTag.ownership == TITLE_GUEST)
    ownership = "is for a guest.";

  Serial.print("The NUID tag ");
  Serial.print(ownership);
  Serial.println();
}

/**
  Helper routine to dump a byte array as hex values to Serial.
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
  Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
