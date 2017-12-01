/*
	 ----------------------------------------------------------------------------
	 Example sketch/program showing how to read new NUID from a PICC to serial.
	 ----------------------------------------------------------------------------
	 This is a MFRC522 library example; for further details and other examples
see: https://github.com/miguelbalboa/rfid

Example sketch/program showing how to the read data from a PICC (that is: a
RFID Tag or Card) using a MFRC522 based RFID Reader on the Arduino SPI
interface.

When the Arduino and the MFRC522 module are connected (see the pin layout
below), load this sketch into Arduino IDE then verify/compile and upload it.
To see the output: use Tools, Serial Monitor of the IDE (hit Ctrl+Shft+M).
When you present a PICC (that is: a RFID Tag or Card) at reading distance of
the MFRC522 Reader/PCD, the serial output will show the type, and the NUID
if a new card has been detected. Note: you may see "Timeout in
communication" messages when removing the PICC from reading distance too
early.

@license Released into the public domain.

Typical pin layout used:
------------------------------------------------------------------------------
MFRC522     Arduino      Arduino  Arduino  Arduino        Arduino
Reader/PCD  Uno/101      Mega     Nano v3  Leonardo/Micro Pro Micro
Signal     Pin         Pin          Pin      Pin      Pin            Pin
------------------------------------------------------------------------------
RST/Reset  RST         9            5        D9       RESET/ICSP-5   RST
SPI SS     SDA(SS)     10           53       D10      10             10
SPI MOSI   MOSI        11 / ICSP-4  51       D11      ICSP-4         16
SPI MISO   MISO        12 / ICSP-1  50       D12      ICSP-1         14
SPI SCK    SCK         13 / ICSP-3  52       D13      ICSP-3         15
 */

// LIBRARIES.
#include <SPI.h>
#include <MFRC522.h>

// ARDUINO PINS.
constexpr uint8_t RST_PIN = 9;     // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 10;     // Configurable, see typical pin layout above
const byte IN_LOT_PIN = 8;
const byte LOT_NUMBER_PIN1 = 3;
const byte LOT_NUMBER_PIN2 = 4;
const byte LOT_NUMBER_PIN3 = 5;

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

void setup() {
	Serial.begin(9600);
	SPI.begin(); // Init SPI bus
	rfid.PCD_Init(); // Init MFRC522

	// Initialize output pins.
	pinMode(IN_LOT_PIN, OUTPUT);
	pinMode(LOT_NUMBER_PIN1, OUTPUT);
	pinMode(LOT_NUMBER_PIN2, OUTPUT);
	pinMode(LOT_NUMBER_PIN3, OUTPUT);

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
	SendLotData(rfidTag);
	CLIENT_UID[rfidNumber].inLot = ! rfidTag.inLot;
	Serial.println();
	Serial.println();

	// Halt PICC.
	rfid.PICC_HaltA();

	// Stop encryption on PCD.
	rfid.PCD_StopCrypto1();
}

// FUNCTIONS.
// Get ownership
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

//
void SendLotData(uid uidTag)
{
	// Write if item still in lot.
	digitalWrite(IN_LOT_PIN, uidTag.inLot);

	if (uidTag.inLot)
		Serial.println("Item is in lot. Recovering.");
	else
		Serial.println("Item is not in lot. Inserting.");

	// Define in binary the lot number.
	int lotNumber = uidTag.assignedSpace;

	// Send binary data for lot to access.
	switch(lotNumber) {
		case 1:
			digitalWrite(LOT_NUMBER_PIN1, HIGH);
			digitalWrite(LOT_NUMBER_PIN2, LOW);
			digitalWrite(LOT_NUMBER_PIN3, LOW);
			break;
		case 2:
			digitalWrite(LOT_NUMBER_PIN1, LOW);
			digitalWrite(LOT_NUMBER_PIN2, HIGH);
			digitalWrite(LOT_NUMBER_PIN3, LOW);
			break;
		case 3:
			digitalWrite(LOT_NUMBER_PIN1, HIGH);
			digitalWrite(LOT_NUMBER_PIN2, HIGH);
			digitalWrite(LOT_NUMBER_PIN3, LOW);
			break;
		case 4:
			digitalWrite(LOT_NUMBER_PIN1, LOW);
			digitalWrite(LOT_NUMBER_PIN2, LOW);
			digitalWrite(LOT_NUMBER_PIN3, HIGH);
			break;
		default:
			digitalWrite(LOT_NUMBER_PIN1, LOW);
			digitalWrite(LOT_NUMBER_PIN2, LOW);
			digitalWrite(LOT_NUMBER_PIN3, LOW);
			break;
	}
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
