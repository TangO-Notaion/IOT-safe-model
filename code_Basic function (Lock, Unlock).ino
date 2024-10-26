#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <TridentTD_LineNotify.h>
#include <WiFi.h>
#include "DHT.h"

#define DHTTYPE DHT22
#define LINE_TOKEN  "ZEqXoOMFnqVvJtERXQjJVMiDmIaaOSKOJRyUZ01LOFT"
#define SSID "------" // Change the name of the WiFi you want to connect to.
#define PASSWORD "------" // Change the WiFi password to connect.

int LED1 = 18; 
int LED2 = 5; 
int buzzer = 4;
int dhtpin = 15;
int relay = 13;

DHT dht(dhtpin, DHTTYPE);

const byte ROW_NUM = 4; //four rows
const byte COLUMN_NUM = 3; //three columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

  byte pin_rows[ROW_NUM] = {12, 14, 27, 26}; //connect to the row pinouts of the keypad
  byte pin_column[COLUMN_NUM] = {25, 33, 32}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

String password = "123456"; 
String lock = "654321";
String input_password = "";
String input_reset = "";
String message = "";

int count = 0, sens = 0;
bool con = 0, stt = 1, allow = 0, RP = 0, notice = 1, unlock = 0;
double h, t, hic;

LiquidCrystal_I2C lcd(0x3F, 16, 2);

void myDelay(unsigned long del) 
{
  unsigned long myPrevMillis = millis();
  unsigned long myCurrentMillis = myPrevMillis;
  while (myCurrentMillis - myPrevMillis <= del) 
  {
    myCurrentMillis = millis();
  }
  return;
}

void checkPassword(String str)
{
  if(str == password)
  {
    count = 0;
    unlock = 1;
    Serial.println("The password is correct, ACCESS GRANTED!");
    lcd.clear();
    lcd.print("Correct!");
    digitalWrite(relay, LOW);   
    digitalWrite(LED1, LOW); 
    digitalWrite(LED2, HIGH); 
    LINE.notify("Unlock");
    lcd.clear();
    lcd.print("Welcome");
  }

  else if(str == lock)
  {
    count = 0;
    unlock = 0;
    Serial.println("Lock");
    digitalWrite(relay, HIGH);
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, LOW); 
    LINE.notify("Lock");
    lcd.clear();
    lcd.print("Lock");
  }

  else
  {
    if(unlock)
    {
      lcd.clear();
      lcd.print("Incorrect");
      myDelay(700);
      lcd.clear();
      lcd.print("Can't lock");
      LINE.notify("Incorrect password. The safe cannot be locked");
      lcd.clear();
      lcd.print("Enter again");
    }
    else
    {
      count++;
      allow = 1;
      Serial.println("The password is incorrect, ACCESS DENIED!");
      lcd.clear();
      lcd.print("Incorrect");
      LINE.notify("Incorrect password");
      //alarm
      if(count > 3 && unlock == 0 && allow)
      {
        int cooldown = (count-2)*30;
        password = "-------";
        lock = "-------";
        myDelay(500);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("You can't access");
        lcd.setCursor(0, 1);
        lcd.print("for ");
        lcd.setCursor(4, 1);
        lcd.print(cooldown);
        lcd.setCursor(String(cooldown).length()+5, 1);
        lcd.print("sec.");
        myDelay(500);
        LINE.notify("Warning! The wrong password was entered more than three times.");
        lcd.clear();
        for(int i=cooldown;i>=0;i--)
        {
          lcd.setCursor(0, 0);
          lcd.print(i);
          lcd.setCursor(String(i).length()+1, 0);
          lcd.print("sec");
          Serial.println(i);
          digitalWrite(buzzer, HIGH);
          myDelay(500);
          lcd.clear();
          digitalWrite(buzzer, LOW);
          myDelay(500);
        }
        digitalWrite(buzzer, HIGH);
        password = "123456";
        lock = "654321";
        allow = 0;
        LINE.notify("The password can be entered again.");
      }
      myDelay(700);
      lcd.clear();
      lcd.print("Please try agian");
    }
  }
  return;
}

void setup() 
{
  Serial.begin(115200);

  input_password.reserve(7); // maximum input characters is 33, 

  lcd.begin();
  lcd.clear();

  pinMode(relay, OUTPUT);  
  digitalWrite(relay, HIGH);

  pinMode(LED1, OUTPUT); 
  pinMode(LED2, OUTPUT);
  digitalWrite(LED1, HIGH); 
  digitalWrite(LED2, LOW); 

  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, HIGH);

  dht.begin();
  
  Serial.println(LINE.getVersion());
  WiFi.begin(SSID, PASSWORD);
  Serial.printf("WiFi connecting ",  SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    myDelay(500);
  }
  Serial.printf("\nWiFi connected\nIP : ");
  Serial.println(WiFi.localIP());
  LINE.setToken(LINE_TOKEN);
  LINE.notify("Started");
  lcd.print("Enter password");
}

void loop() {
  
  
  t = dht.readTemperature();

  if(t>50)
  {
    //LINE.notify(t,2);
    LINE.notify("Danger! The surrounding area has a temperature above 50°C. There may be flames nearby.");
  }

  char key = keypad.getKey();
  
  if(RP)
  {    
    if(notice)
    {
      LINE.notify("Start password reset.");
      Serial.println("do reset");
      lcd.print("Enter a new");
      lcd.setCursor(0, 1);
      lcd.print("password");
      notice = 0;
    }
    if(key)
    {   
      lcd.clear();
      if(key == '*' && input_reset.length()>0)
      {
        input_reset = input_reset.substring(0,input_reset.length()-1);
        lcd.clear();
        lcd.print(input_reset);
        Serial.println(input_reset);
      } 
      //enter
      else if(key == '#' && input_reset.length()>0) 
      {
        password = input_reset;
        Serial.print("New password ");
        Serial.println(password);
        input_reset = ""; // clear input password
        RP = 0;
        lcd.clear();
        LINE.notify("The password has been changed successfully.");
        lcd.print("Password changed");    
      } 
      //append key
      else 
      {
        input_reset += key; // append new character to input password string
        lcd.clear();
        lcd.print(input_reset);
        Serial.println(input_reset);
      }
    }
  }

  else 
  {
    if(key)
    {
      if(con >= 1 || stt)     
      {
        lcd.clear();
        con = 0;
        stt = 0;
      } 
      if(key == '#' && input_password[0] == '#' && input_password.substring(1,input_password.length()) == password)
      {
        Serial.println(input_password.substring(1,input_password.length()));
        RP = 1;
        notice = 1;
        input_password = "";
        lcd.clear();
      }
      //delete
      else if(key == '*' && input_password.length()>=0)
      {
        con++;
        input_password = input_password.substring(0,input_password.length()-1);
        lcd.clear();
        lcd.print(input_password);
        Serial.println(input_password);
      } 
        //enter
      else if(key == '#' && input_password.length()>0) 
      {
        con++;
        checkPassword(input_password);
        input_password = ""; // clear input password
      } 
        //append key
      else 
      {
        input_password += key; // append new character to input password string
        lcd.clear();
        lcd.print(input_password);
        Serial.println(input_password);
      }
    }
  }
}
