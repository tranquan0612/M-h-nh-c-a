#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#include <SoftwareSerial.h>
#define Signal A0
#define RF D5
#define R0 D7
#define R1 D8
#define Coi D6

int cout=0;
String location = "---";
String receivedData = ""; // Biến để lưu trữ dữ liệu tạm thời
String phoneNumber = "+84971063280"; // Điện thoại nhận thông tin GPS

SoftwareSerial A9gSerial(D4,D3); // RX , TX 
void tich(int x=1, int y=100)
{
  while(x--)
  {
    digitalWrite(Coi,1); delay(y);
    digitalWrite(Coi,0); delay(y);
  }
}
void setup() {
    pinMode(Signal,INPUT);
    pinMode(RF,INPUT);
    pinMode(D8,INPUT);
    pinMode(R0,OUTPUT);
    pinMode(R1,OUTPUT);
    pinMode(Coi,OUTPUT); 
    tich();
    
    Serial.begin(9600);
    A9gSerial.begin(9600);
    lcd.init();
    lcd.backlight();
    lcd.print("Dang khoi tao...");
    delay(2000);
    A9gSerial.println("AT"); //Kiểm tra đáp ứng của A9gSerial A9g, nếu trả về OK thì Module hoạt động
    delay(500);
    UpdateSerial();
    delay(12000);
    A9gSerial.println("AT+CMGF=1");  // chế độ văn bản 
    delay(500);
    UpdateSerial();
    A9gSerial.println((char)26); 
    delay(1000); 
    // Gửi lệnh lấy thông tin GPS
    A9gSerial.println("AT+GPS=1");
    tich(3,100);
    lcd.setCursor(0,0); lcd.print("Th.bi chong trom");
    delay(2000); // Chờ cho module A9G xử lý lệnh
    lcd.clear();
}
void loop() { 
    while(A9gSerial.available()){
      parseData(A9gSerial.readString());
      }
    while (Serial.available()){
      A9gSerial.write(Serial.read());
      }
    if (digitalRead(RF)==1){
      tich();
      cout++;  
      while (digitalRead(RF)==1) delay(10);
    }
    khoa_dien();
    if(cout%2==1 && analogRead(Signal)>250){
      lcd.setCursor(0,1); lcd.print("Canh bao co TROM");
      digitalWrite(Coi,1);
      GPS();
      bao_dong();
      lcd.clear();
    }
 }
void khoa_dien(){
    if(cout%2 ==1){
      digitalWrite(R0,HIGH);
      lcd.setCursor(0,0); lcd.print(" Khoa:     ON   ");
    }else{
      digitalWrite(R0,LOW);
      lcd.setCursor(0,0); lcd.print(" Khoa:    OFF   ");
    }
}
void bao_dong(){
    digitalWrite(R1,HIGH);
    digitalWrite(Coi,HIGH);
//    delay(10000);
 
    for (int i =0 ;i < 200; i++) {
      if (digitalRead(RF) == 1) {
          break;
        }
       delay(100);
       digitalWrite(Coi,!digitalRead(Coi));
    }
    digitalWrite(R1,LOW);
    digitalWrite(Coi,LOW);
}
void parseData(String buff){
    Serial.println(buff);
}
void UpdateSerial(){
    while (A9gSerial.available()) 
    {
        Serial.write(A9gSerial.read());
    }
    
    while (Serial.available()) 
    {
      A9gSerial.write(Serial.read());
    }
}
// Hàm lấy tọa độ
void GPS(){

  // Gửi lệnh lấy vị trí GPS
  A9gSerial.println("AT+LOCATION=2\r");
  delay(3000); // Chờ cho module A9G xử lý lệnh

  // Đọc dữ liệu từ module A9G
while (A9gSerial.available()) {
    char c = A9gSerial.read();
    receivedData += c;
  }

  // Kiểm tra xem dữ liệu có chứa thông tin GPS
  if (receivedData.indexOf("AT+LOCATION=2") != -1) {
    
//    receivedData = receivedData.substring(start);
    int start = receivedData.indexOf("AT+LOCATION=2") + 14; // Bắt đầu từ vị trí +LOCATION: và bỏ 11 ký tự
    int end = receivedData.indexOf("OK", start); // Tìm vị trí kết thúc của dòng
    if (end != -1) {
      location = receivedData.substring(start, end); // Trích xuất dữ liệu GPS
      Serial.print("Location: ");
      Serial.println(location);
    }
  }
  lcd.setCursor(0,1); lcd.print("Dang gui tinnhan");
  // Gửi thông tin GPS qua tin nhắn
  sendSMS(phoneNumber, "www.google.com/maps?q=" + location);

  // Đặt lại biến receivedData để sẵn sàng cho lần đọc tiếp theo
  receivedData = "";

  // Delay trước khi thực hiện lệnh tiếp theo (có thể thay đổi tùy theo tần số cần lấy dữ liệu GPS)
  delay(1000);
}
// Hàm gửi tin nhắn SMS
void sendSMS(String phoneNumber, String message) {
  A9gSerial.println("AT+CMGF=1"); // Chọn chế độ văn bản
  delay(1000);
  A9gSerial.print("AT+CMGS=\"");
  A9gSerial.print(phoneNumber);
  A9gSerial.println("\""); // Gửi đến số điện thoại cụ thể
  delay(1000);
  A9gSerial.print(message);
  A9gSerial.write(26); // Gửi ký tự End Of Text (EOT) để hoàn thành tin nhắn
  delay(1000);
}