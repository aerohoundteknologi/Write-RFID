#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
// =================== RFID
#define RST_PIN         19
#define SS_PIN          10
MFRC522 mfrc522(SS_PIN, RST_PIN);
// put function declarations here:

void printbin8(uint8_t data);
void printbin64(uint64_t data);
uint64_t string_toUint64(String strint);
void doWrite(uint64_t dataToWrite);

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  SPI.begin();               // Init SPI bus
  mfrc522.PCD_Init(); 
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()>0){
    String idnum_str = Serial.readString();
    idnum_str.trim();
    doWrite(string_toUint64(idnum_str));
    // doWrite((uint64_t));
  }
  delay(300);
}

void printbin8(uint8_t data){
  for(int i = 0; i < 8; i++){
    Serial.print((data >> (7-i)&0b1));
  }
}

void printbin64(uint64_t data){
  for(int i = 0; i < 64; i++){
    Serial.print((int)(data >> (63-i)&0b1));
  }
}
uint64_t string_toUint64(String strint){
  uint64_t result = 0;
  for(unsigned int i = 0; i < strint.length(); i++){
    if(!isDigit(strint[i])) break;
    result = result*10 + (strint[i] - '0');
  }
  return result;
}
void doWrite(uint64_t dataToWrite){
// Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    Serial.println(F("{\"status\":\"failed\"}"));
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    Serial.println(F("{\"status\":\"failed\"}"));
    return;
  }

  // Serial.print(F("Card UID:"));    //Dump UID
  // for (byte i = 0; i < mfrc522.uid.size; i++) {
  //   Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
  //   Serial.print(mfrc522.uid.uidByte[i], HEX);
  // }
  // Serial.print(F(" PICC type: "));   // Dump PICC type
  // MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  // Serial.println(mfrc522.PICC_GetTypeName(piccType));

  byte buffer[34];
  byte block;
  MFRC522::StatusCode status;
  // byte len;

  // Serial.setTimeout(20000L) ;     // wait until 20 seconds for input from serial
  
  for(int i = 0; i < 8; i++){
    buffer[i] = dataToWrite >> (i*8);
    // printbin8(buffer[i]); Serial.print(" ");
  }//Serial.println();

  block = 1;
  //Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.println(F("{\"status\":\"failed\"}"));
    // Serial.print(F("PCD_Authenticate() failed: "));
    // Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else //Serial.println(F("PCD_Authenticate() success: "));

  // Write block
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.println(F("{\"status\":\"failed\"}"));
    // Serial.print(F("MIFARE_Write() failed: "));
    // Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("{\"status\":\"success\"}"));

  mfrc522.PICC_HaltA(); // Halt PICC
  mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
}