#include <AccelStepper.h>
#include <LiquidCrystal_I2C.h>  // Подключаем библиотеку для LCD
#include <Wire.h>
#include <IRremote.h>
#define PIN_RECEIVER 7   // Signal Pin of IR receiver
IRrecv receiver(PIN_RECEIVER);

// Определяем шаговый двигатель и используемые пины (по умолчанию для 4-проводного подключения)
AccelStepper stepper(AccelStepper::FULL4WIRE, 2, 3, 4, 5); // Пины для подключения мотора

// Пин для подключения потенциометра
const byte SpeedPot = A0;  // Потенциометр для регулировки скорости

// Инициализация LCD экрана с I2C адресом 0x3F и размером 16x2
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Адрес может быть другим, проверьте с помощью сканера I2C

// Переменные для хранения данных
const int sampleCount = 10;    // Количество измерений в секунду (1000ms / 100ms = 10)
int readings[sampleCount];     // Массив для хранения значений
int readIndex = 0;             // Индекс для записи значений в массив
long total = 0;                // Сумма значений для расчета среднего
long average = 0;              // Среднее значение
int sensorValue = 0;
long speed = 0;
int safevalue = 700;           // Ещё безопасное значение
int receiver_signal = 0;
int power_flag = 0;

unsigned long lastDisplayTime = 0; // Время последнего обновления дисплея

// Флаг для мониторинга текущего значения сигнала
volatile bool moni_tok = false;

// Настройка таймера 1 для прерываний каждые 100 мс
void setupTimer1() {
  cli(); // Отключение прерываний для настройки таймера

  TCCR1A = 0; // Очищаем регистр управления A
  TCCR1B = 0; // Очищаем регистр управления B

  // Настройка CTC Mode
  TCCR1B |= (1 << WGM12); 
  TCCR1B |= (1 << CS12);  // Устанавливаем делитель тактовой частоты на 256

  OCR1A = 16000000 / 256 * 0.1 - 1;    // Настройка таймера на интервал 100 мс (0.1 секунды)

  TIMSK1 |= (1 << OCIE1A); // Разрешаем прерывание по совпадению

  sei(); // Включение глобальных прерываний
}

// Обработчик прерывания Timer1
ISR(TIMER1_COMPA_vect) {
  moni_tok = true; // Устанавливаем флаг мониторинга тока
}

void setup() {
  // Настройка максимальной скорости
  stepper.setMaxSpeed(1000);  // Максимальная скорость (можно настроить)
  receiver.enableIRIn(); // Старт ресивера
  // Инициализация LCD экрана
  lcd.init();
  lcd.begin(16, 2);  // Устанавливаем размер экрана (16 колонок, 2 строки)
  lcd.noBacklight();   // Включаем подсветку экрана
  pinMode(13, OUTPUT);
  Serial.begin(9600);

  // Инициализируем массив значений как 0
  for (int i = 0; i < sampleCount; i++) {
    readings[i] = 0;
  }

  // Настроим таймер
  setupTimer1();
}

void loop() {
  // Проверяем сигнал на ресивер
  if (receiver.decode()) {
    Serial.println(receiver.decodedIRData.command);
    receiver_signal = receiver.decodedIRData.command;
    receiver.resume();  // Receive the next value
  }

  if (receiver_signal ==162)
  {
    if(power_flag ==0)
    { 
    power_flag = 1;
    receiver_signal = 0;
    lcd.backlight();
    }
    else
    {
        power_flag = 0;
        receiver_signal = 0;
        lcd.noBacklight();
    }
  }
  
  if(power_flag==1)
  { 


  if (receiver_signal ==2)
  {
    speed+=100;
    Serial.println("Speed upped!");
    receiver_signal = 0;
  }
  if (receiver_signal ==152)
  {
    speed-=100;
    Serial.println("Speed slowed!");
    receiver_signal = 0;
  }
  // Проверяем флаг мониторинга сигнала
  if (moni_tok) {
    
    moni_tok = false; // Сбрасываем флаг

    // Чтение значения с потенциометра
    sensorValue = analogRead(SpeedPot);
    if(speed<=safevalue)
    {
    // Преобразуем значение с потенциометра в скорость (от 0 до 1000)
    //speed = map(sensorValue, 0, 1023, 0, 1023); // Значения от 0 до 1000 (скорость вращения)

    // Отнимаем старое значение (которое больше не будет использоваться)
    total -= readings[readIndex];

    // Добавляем новое значение в массив
    readings[readIndex] = speed;

    // Добавляем новое значение к общей сумме
    total += speed;

    // Перемещаем индекс для следующего измерения
    readIndex++;

    // Если мы собрали 10 значений, начинаем снова с нулевого индекса
    if (readIndex >= sampleCount) {
      readIndex = 0;
    }

    // Если прошло 1 секунда (1000ms), рассчитываем среднее и обновляем экран
    unsigned long currentMillis = millis();
    if (currentMillis - lastDisplayTime >= 1000) {
      lastDisplayTime = currentMillis;  // Обновляем время последнего обновления дисплея

      // Рассчитываем среднее значение
      average = total / sampleCount;

      // Обновляем только если значение изменилось
      static long lastAverage = -1;
      if (average != lastAverage) {
        lcd.clear();

        lcd.setCursor(0, 0);
        lcd.print("Speed: ");
        lcd.setCursor(7, 0);
        lcd.print(speed); 

        lcd.setCursor(0, 1);
        lcd.print("Avg Speed: ");
        lcd.setCursor(11, 1);
        lcd.print(average);  // Отображаем среднее значение
        lastAverage = average;
      }
    }

    // Постоянное движение мотора с текущей скоростью
    stepper.setSpeed(speed);  // Используем среднее значение как скорость
    stepper.runSpeed();         // Вращаем мотор с заданной скоростью
    }
    else
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("!!!!Crash!!!!");
      lcd.setCursor(0, 1);
      lcd.print("Motor stopped");
      stepper.setSpeed(0);
      digitalWrite(13, HIGH);
        
        
    }
  }
  if(speed<=safevalue)
  { 
    digitalWrite(13,LOW);
  stepper.setSpeed(speed);  
    stepper.runSpeed();         
  }
  }
}
