#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
// =================== RFID
#define RST_PIN 19
#define SS_PIN 10
MFRC522 mfrc522(SS_PIN, RST_PIN);
String idnum_str;
bool ready = false;

// put function declarations here:

void printbin8(uint8_t data);
void printbin64(uint64_t data);
uint64_t string_toUint64(String strint);
void doWrite(uint64_t dataToWrite);

void setup()
{
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  mfrc522.PCD_Init();
}

void loop()
{
  if (Serial.available() > 0)
  {
    idnum_str = Serial.readString();
    idnum_str.trim();
  }
  if (!ready)
  {
    Serial.println("ready");
    ready = true;
  }
  if (idnum_str == "")
  {
    return;
  }

  doWrite(string_toUint64(idnum_str));

  delay(300);
}

void printbin8(uint8_t data)
{
  for (int i = 0; i < 8; i++)
  {
    Serial.print((data >> (7 - i) & 0b1));
  }
}

void printbin64(uint64_t data)
{
  for (int i = 0; i < 64; i++)
  {
    Serial.print((int)(data >> (63 - i) & 0b1));
  }
}
uint64_t string_toUint64(String strint)
{
  uint64_t result = 0;
  for (unsigned int i = 0; i < strint.length(); i++)
  {
    if (!isDigit(strint[i]))
      break;
    result = result * 10 + (strint[i] - '0');
  }
  return result;
}
void doWrite(uint64_t dataToWrite)
{
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++)
    key.keyByte[i] = 0xFF;

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial())
  {
    return;
  }

  byte buffer[34];
  byte block;
  MFRC522::StatusCode status;

  for (int i = 0; i < 8; i++)
  {
    buffer[i] = dataToWrite >> (i * 8);
  }

  block = 1;
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK)
  {
    Serial.println("failed");
    return;
  }
  else
    status = mfrc522.MIFARE_Write(block, buffer, 16);

  if (status != MFRC522::STATUS_OK)
  {
    Serial.println("failed");
    return;
  }
  else
  {
    Serial.println("success");
    // cleanup
    ready = false;
    idnum_str = "";
  }

  mfrc522.PICC_HaltA();      // Halt PICC
  mfrc522.PCD_StopCrypto1(); // Stop encryption on PCD
}