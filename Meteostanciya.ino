/*
Это мой первый ардуино-проект который я сделал. Этот проект называется Метеостанция 2.2, 
эта метеостанция более доработаная, по сравнению с предыдущей, 
здесь метеостанция использует часовой модуль DS3231, датчик температуры и давления BMP280, 
датчик влажности и температуры DHT11(можно использовать DHT22), 
и здесь можно использовать аккумуляторы, чтобы метеостанция могла работать 
без сети электропитания. Убрано: настройка пина прерывания для кнопки, настройка 
включение/отключение по времени и настройки символов и строк дисплея. 
Добавлено: настройка максимальной температуры стала более универсальная: 
если поставить 0, то эта настройка отключается, настрока типа дисплея и 
настройка датчиков, либо DHT и BMP280 или BME280
Также в этой версии метеостанции используется более облегчённая и 
более функциональная библиотека кнопки EncButton. 
Этот проект создан для того чтобы смотреть время, число, день недели, температуру, влажность и 
давление.
С уважением Николай Жуков
Дата и время проекта: 13:50:37 12 января(воскресенье) 2025 года
*/

// - - - - - - НАCТРОЙКИ ДЛЯ DHT - - - - - -

#define DHT_BMP280_OR_BME280 0 // если 0 - DHT И BMP280, если 1 - BME280 и ниже настройки выкл.
#define DHT_PIN 2 // пин куда подключён DHT 
#define DHT_TIP DHT11 // тип DHT (лучше DHT22, он точнее и диапазон температуры больше)

// - - - - - НАСТРОЙКИ ДЛЯ ДИСПЛЕЯ - - - - -

#define ADRES 0x27 // адрес для дисплея
#define DISPLAY_TYPE 0 // тип дисплея: 1 - 2004, 0 - 1602

// - - - - - НАСТРОЙКИ МЕТЕОДАННЫХ - - - - - 

#define TEMPERATURA 28 // максимальная допустимая температура (если равно 0, то настройка выкл.)

// - - - - - НАСТРОЙКИ ДЛЯ КНОПКИ - - - - - 

#define EB_HOLD_TIME 1000 // время удержания кнопки
#define EB_DEB_TIME 1 // дебаунс кнопки, у меня 1 миллисекунда, потому что кнопка сенсорная
#define VKL_UDER 0 // 0 - включается по клику, 1 - по удержаниию
#define VREMYA_RABOTY 6000 // время работы подсветки по кнопке
#define BUTTON_PIN 3 // пин для сенсорной кнопки

// - - - - - НАСТРОЙКИ ДЛЯ ВРЕМЕНИ - - - - - 

#define VREMYA 0 // если 1 - устанавливаем время, 0 - не устанавливаем время
#define SECOND 30 // устанавливаем секунды
#define MINUTE 23 //  устанавливаем минуты
#define HOUR 16 // устанавливаем часы
#define DATE 1 //  устанавливаем день месяца
#define MONTH 7 //  устанавливаем месяц
#define YEAR 2025 // устанавливаем год

// - - - - - НАСТРОЙКИ ЗАКОНЧЕНЫ - - - - - 

// - - - - - - - БИБЛИОТЕКИ - - - - - - - - 

#include <LiquidCrystal_I2C.h> // библиотека для дисплея (LCD1602)
#include <Wire.h> // библиотека для I2C
#if(DHT_BMP280_OR_BME280 == 0)
#include <DHT.h> // библиотека для датчика влажности (в этом варианте DHT11)
#endif
#include <EncButton.h> // библиотека для кнопки
#include <GyverBME280.h> // библиотека для датчика температуры и давления (BMP280)
#include <microDS3231.h> // библиотека для часового модуля (DS3231)
#include <GTimer.h> // библиотека для включения по кнопке
//#include <GyverOS.h> // библиотека для распределения задач
#include <GyverPower.h> // библиотека для энергосбережения

// - - - - - - ИНИЦИАЛИЗАЦИЯ - - - - - - - -
#if(DISPLAY_TYPE == 0) 
LiquidCrystal_I2C l(ADRES, 16, 2); // подключаем дисплей 1602
#elif(DISPLAY_TYPE == 1)
LiquidCrystal_I2C l(ADRES, 20, 4); // подключаем дисплей 2004
#endif
#if(DHT_BMP280_OR_BME280 == 0)
DHT d(DHT_PIN, DHT_TIP); // подключаем датчик влажности
#endif
Button bt(BUTTON_PIN, INPUT, HIGH); // подключаем сенсорную кнопку
//GyverOS<3> os; // подключаем задачи
GyverBME280 b; // подключаем датчик температуры и давления
MicroDS3231 ds; // подключаем часы
GTimer<millis> tmr(VREMYA_RABOTY, 0, GTMode::Timeout, 0); // подключаем таймер

// - - - - - - - - - МАССИВЫ - - - - - - - - -

byte c [] { // создаём значок температуры в виде массива
    0b10000,
    0b01110,
    0b10001,
    0b10000,
    0b10000,
    0b10001,
    0b01110,
    0b00000
};

// - - - - - - - - - SETUP - - - - - - - - -

void setup(){
    #if(VREMYA == 1)
    DateTime now; // устанавливаем время
    now.second = SECOND; 
    now.minute = MINUTE; 
    now.hour = HOUR; 
    now.date = DATE; 
    now.month = MONTH; 
    now.year = YEAR; 
    ds.setTime(now);
    #endif
    l.init(); // иницилизация дисплея
    //l.backlight(); // включаем подсветки 
    power.autoCalibrate(); // идёт калибровка, выполняется примерно 2 секунды
    power.hardwareDisable(PWR_ALL); // отключаем всё
    power.hardwareEnable(PWR_TIMER0|PWR_I2C); // включаем только то, что надо
    Wire.begin(); // переинициализируем шину I2C
    l.createChar(0,c); // генерируем символ градуса по цельсию
    #if(DHT_BMP280_OR_BME280 == 0)
    d.begin(); // иницилизация датчика влажжности
    #endif
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), isr, CHANGE); // подключааем внешнее прерывание, чтобы точно не пропустить кнопку
    //os.attach(0, print, 1000); // иницилизируем задачи
    //os.attach(1, oversampling, 1000);
    //os.attach(2, backlight, 1000);
    //os.attach(3, clear, 1000);
    b.setMode(NORMAL_MODE); // настраиваем датчик температуры и давления
    b.setStandbyTime(STANDBY_1000MS); // делаем измерения каждую секунду
    #if(DHT_BMP280_OR_BME280 == 0)
    b.setHumOversampling(MODULE_DISABLE); // отключаем опрос влажности у датчика температуры
    #endif
    b.begin(); // делаем иницилизацию датчика температуры и давления
	   //pinMode(13, OUTPUT);
}

// - - - - - АППАРАТНОЕ ПРЕРЫВАНИЕ - - - - - - - -

void isr() {
  power.wakeUp(); // просыпаемся
  bt.tick(); // опрашиваем кнопку чтобы точно не пропустить
}

// - - - - - - - - - - LOOP - - - - - - - - - - - - 

void loop(){
    static bool flag; // подключаем локальную переменную флаг, для подсветки
    static uint32_t tmr1, tmr2;
    bt.tick(); // проверяем кнопку на удержание
    tmr.tick(); // проверяем таймер на срабатывание   
    #if(VKL_UDER == 0) // если 0,то включаемся когда кнопка была нажата
    if(bt.press()){ // если было прерывание
        tmr.start(); // включаем таймер
    } 
    #elif(VKL_UDER == 1) // если 1, то включаемся по удержанию
      if(bt.hold()){ // если было удержание
        tmr.start(); // включаем таймер
    } 
    #endif
    if(tmr.running()){ // если таймер активен
        flag = 1; // включаем дисплей
        l.setBacklight(flag);
        l.display();
        if(millis() - tmr1 >= 1000){ // если прошла 1 секунда
        tmr1 = millis();
        oversampling(); // проводим измерения и выводим информацию на дисплей 
     }
}
    else{ // в противном случае отключаем дисплей, не проводим измеренрия и не выводим на дисплей
        flag = 0;
        l.setBacklight(flag);
        l.noDisplay();
    }
    if(!tmr.running() && millis() - tmr2 >= 10000){ // если таймер отключен и прошло 10 секунд, спим
        tmr2 = millis();
        power.sleep(SLEEP_FOREVER);
  }
}

// - - - - - - - - - - ФУНКЦИИ  - - - - - - - - - - - 

void print(float p, byte t, byte v){
    l.setCursor(0,0); // выводим время с начала первой строки
	   l.print(ds.getTimeString()); // получить время как строку вида ЧЧ:ММ:CC
    l.setCursor(10,0);
    l.print(ds.getDate()); // выводим число 
    l.setCursor(13,0); 
	switch (ds.getDay()){ // парсим номера дней недель в названия(дни недели на английском языке)
		case 1:
		  l.print("MON");
		  break;
		case 2:
		  l.print("TUE");
		  break;
		case 3:
		  l.print("WED");
		  break;
		case 4:
		  l.print("THU");
		  break;
		case 5:
		  l.print("FRI");
		  break;
		case 6:
		  l.print("SAT");
		  break;
		case 7:
		  l.print("SUN");
		  break;
	}
    l.setCursor(5,1);
    l.print(v); // выводим влажность
    l.print("%");
    l.setCursor(10,1);
    l.print(p); // выводим давление в мм.рт.ст.
    #if(TEMPERATURA > 0)
    if(t > TEMPERATURA){ // если температура больше 28 °C
    l.setCursor(0,1); // выводим с начала второй строки
    l.print(t); // выводим температуру
    l.print("!");
    }
    else{             // в противнном случае
    l.setCursor(0,1); // выводим с начала второй строки
    l.print(t); // выводим температуру
    l.print(char(0)); // выводим знак температуры     
    }
    #elif(TEMPERATURA < 0)
    if(t < TEMPERATURA){ // если температура больше 28 °C
    l.setCursor(0,1); // выводим с начала второй строки
    l.print(t); // выводим температуру
    l.print("!");
    }
    else{             // в противнном случае
    l.setCursor(0,1); // выводим с начала второй строки
    l.print(t); // выводим температуру
    l.print(char(0)); // выводим знак температуры     
    }
    #else             // но если настройка равна нулю
    l.setCursor(0,1); // выводим с начала второй строки
    l.print(t); // выводим температуру
    l.print(char(0)); // просто выводим знак температуры
    #endif
}
void oversampling(){
    static float pressure; // переменная для считывания давления в Па
    static float pMmHg; // переменная для парсинга давления в мм.рт.ст.
    static int temperature; // переменная для температуры
    static int vlazhnost; // переменная для влажности
    pressure = b.readPressure();
    pMmHg = pressureToMmHg(pressure); // измеренное давление в Па преобразуем в мм.рт.ст.
    temperature = b.readTemperature(); // измеряем температуру в градусах по цельсию
    #if(DHT_BMP280_OR_BME280 == 0)
    vlazhnost = d.readHumidity(); // измеряем влажность в %
    #elif(DHT_BMP280_OR_BME280 == 1)
    vlazhnost = b.readHumidity();
    #endif
    print(pMmHg, temperature, vlazhnost);
}