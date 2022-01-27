#include <Arduino.h>
#include <Ds1302.h>
#include <Wire.h>
#include <TimeAlarms.h>
#include <LiquidCrystal_I2C.h>
//#include <avr/wdt.h>
#include <EEPROM.h>
#include "ajustes.h"


// DS1302 RTC instance
//Ds1302 rtc(PIN_ENA, PIN_CLK, PIN_DAT);
Ds1302 rtc(2,4,3);
LiquidCrystal_I2C lcd(0x27,16, 2);


uint8_t clockDisplay; // Id del Alarm
uint8_t rele_auto_off_id = 0;
volatile bool setupMode = false;
volatile bool LCD_brillo = false;
volatile time_t setupTimeOut = 0;
time_t pirTimeOut = 0;
bool triger = false;
bool IoV; // Invierno o Verano
unsigned short foto_sensor;

// Exportados de ajustes
extern HORA invierno_on;
extern HORA invierno_off;
extern HORA verano_on;
extern HORA verano_off;
extern CREPUSCULO crep;
// ---------------------

void led_blink();
bool estacion();
char *formatTimeDigits (const int num);
bool rango(char on, char off, char c);
void rele_on();
void rele_off();
void rele_auto_off();
void digitalClockDisplay();
void brillo_off();
void pir_sensor(void);
void syncHora(void);


/*const static char* WeekDays[] =
{
    "Lunes",
    "Martes",
    "Miercoles",
    "Jueves",
    "Viernes",
    "Sabado",
    "Domingo"
};*/


void save()
{
	HORA hora;
	unsigned char Direccion = 0;
	struct {
		unsigned char dia = day();
		unsigned char mes = month();
		unsigned short int ano = year();
	} fecha;

	hora.hora = hour();
	hora.minuto = minute();
	EEPROM.write(Direccion, true);
	Direccion = Direccion + sizeof(bool);
	EEPROM.put(Direccion, hora);//Guardamos el valor en la dirección de la EEPROM elegida
	Direccion = Direccion + sizeof(hora);
	EEPROM.put(Direccion, fecha);
	Direccion = Direccion + sizeof(fecha);
	EEPROM.put(Direccion, invierno_on);
	Direccion = Direccion + sizeof(invierno_on);
	EEPROM.put(Direccion, invierno_off);
	Direccion = Direccion + sizeof(invierno_off);
	EEPROM.put(Direccion, verano_on);
	Direccion = Direccion + sizeof(verano_on);
	EEPROM.put(Direccion, verano_off);
	Direccion = Direccion + sizeof(verano_off);
	EEPROM.put(Direccion, crep);
	syncHora();
}

bool load()
{
	HORA hora;
	unsigned char Direccion = sizeof(bool);
	struct {
		unsigned char dia;
		unsigned char mes;
		unsigned short int ano;
	} fecha;

	if (!EEPROM.read(0))
		return false;

	EEPROM.get(Direccion, hora);//Guardamos el valor en la dirección de la EEPROM elegida
	//Serial.println((int)hora.hora);
	Direccion = Direccion + sizeof(hora);
	EEPROM.get(Direccion, fecha);
	Direccion = Direccion + sizeof(fecha);
	EEPROM.get(Direccion, invierno_on);
	Direccion = Direccion + sizeof(invierno_on);
	EEPROM.get(Direccion, invierno_off);
	Direccion = Direccion + sizeof(invierno_off);
	EEPROM.get(Direccion, verano_on);
	Direccion = Direccion + sizeof(verano_on);
	EEPROM.get(Direccion, verano_off);
	Direccion = Direccion + sizeof(verano_off);
	crep.noche = 0;
	crep.dia = 0;
	crep.tiempo = 0; // En minutos
	EEPROM.get(Direccion, crep);
	//Serial.println(crep.tiempo);
	//Serial.println("Dalos leidos");
	//setTime(hora.hora, hora.minuto, 00, fecha.dia, fecha.mes, fecha.ano);
	return true;
}

void syncHora(void)
{
	Ds1302::DateTime n;
	rtc.getDateTime(&n);
	setTime(n.hour, n.minute, n.second, n.day, n.month, (int)(n.year + 2000));
}

void setup()
{
    //Serial.begin(9600);

    // initialize the RTC
    rtc.init();

    // test if clock is halted and set a date-time (see example 2) to start it
    if (rtc.isHalted())
    {
        //Serial.println("RTC is halted. Setting time...");

        Ds1302::DateTime dt = {
            .year = 18,
            .month = 4,
            .day = 19,
            .hour = 10,
            .minute = 22,
            .second = 0,
            .dow = Ds1302::DOW_THU
        };
        rtc.setDateTime(&dt);

    }
	 /*else
	 	Serial.println("ESta en hora");*/

	syncHora();
	// watch dog
	//wdt_disable();    // No te olvides

	if (!load())
	{
		invierno_on.hora = 21;
		invierno_on.minuto = 0;
		invierno_off.hora = 7;
		invierno_off.minuto = 0;
		verano_on.hora = 22;
		verano_on.minuto = 0;
		verano_off.hora = 6;
		verano_off.minuto = 0;
		crep.noche = 70;
		crep.dia = 150;
		crep.tiempo = 10; // En minutos

		if (timeStatus() != timeSet)
			setTime(00,00,00,16,04,2018);
	}

	lcd.init();
	lcd.backlight();
	lcd.home ();                   // go home

	lcd.print("Programador de");
	lcd.setCursor(0,1);
	lcd.print("luces");

	// Pines
	pinMode(led,OUTPUT);
	pinMode(rele, OUTPUT);
	pinMode(pir, INPUT);


	// Inicamos el timer para el parpadeo de los leds
	Alarm.timerRepeat(1, led_blink);
	// Guardamos la hora cada dia
	Alarm.alarmRepeat(0, 0, 0, save);

	IoV = estacion();

	if (IoV == INVIERNO)
	{
		if (rango(invierno_on.hora, invierno_off.hora, hour()))
		rele_on();
	}
	else
	{
		if (rango(verano_on.hora, verano_off.hora, hour()))
		rele_on();
	}
	clockDisplay = Alarm.timerRepeat(10, digitalClockDisplay);
	Alarm.delay(5000);
	lcd.noBacklight();

	// watch dog;
	//wdt_enable(WDTO_2S);

	// Leemos el foto sensor por primera vez
	foto_sensor = analogRead(fotoresistencia);

}

bool estacion()
{
	bool iv; // Invierno o verano

	if (month() == 3)
	{
		if (day() >= 25)
		iv = VERANO;
		else
		iv = INVIERNO;
	}
	else if (month() == 10)
	{
		if (day() <= 25)
		iv = VERANO;
		else
		iv = INVIERNO;
	}
	else
	{
		iv = (month() > 3  && month() <10);
	}

	return iv;

	// Los meses de cambio de hora son marzo y octubre
	//return (month() >= 3 && day() >= 25 && month() <=10 && day() <= 25);
}

void digitalClockDisplay()
{
	lcd.setCursor(0,0);
	lcd.print("Prog. ");
	if (IoV == INVIERNO)
		lcd.print("invierno");
	else
		lcd.print("verano  ");
	lcd.setCursor(0,1);
	lcd.print(formatTimeDigits(hour()));
	lcd.print(":");
	lcd.print(formatTimeDigits(minute()));
}

void led_blink()
{
	static boolean output = HIGH;
	unsigned short sensor;
	int diferencia;

	// Leemos el foto sensor
	sensor = analogRead(fotoresistencia);
	diferencia = (int)(sensor - foto_sensor);
	//Serial.print("FOTO SENSOR:");
	//Serial.print(sensor);
	//Serial.print(" FOTO SENSOR ORIGINAL:");
	//Serial.println(foto_sensor);
	//Serial.println(diferencia < 200);
	if (diferencia < 200)
		foto_sensor = sensor;
	else if (sensor != foto_sensor)// Se ha encendido una luz
	{
		if (!LCD_brillo)
		{
			lcd.backlight();
			LCD_brillo = true;
			Alarm.timerOnce(5, brillo_off);
		}
		if (triger && !rele_auto_off_id)
		{
			rele_auto_off_id = Alarm.timerOnce((60*60), rele_auto_off);
		}
	}


	//Serial.print(" AJUSTE CREPUSCULAR: ");
	//Serial.print(crep.noche);
	//Serial.print(" - ");
	//Serial.println(crep.dia);
	if (crep.noche > foto_sensor && crep.dia > foto_sensor) // Es denoche
	{
		//Serial.print("Led Activo: ");
		//Serial.print(output);
		digitalWrite(led, output);
		output = !output;
	}
	else // es dedia
	{
		//Serial.print("es dedia. Rele: ");
		digitalWrite(led, LOW);
		output = HIGH;
		//Serial.println(rele_auto_off_id);
		if (rele_auto_off_id)
			Alarm.disable(rele_auto_off_id);
		if (triger && !rele_auto_off_id)
			rele_auto_off_id = Alarm.timerOnce((5*60), rele_auto_off);
		//rele_off();
	}

	if (setupTimeOut < now() && setupMode)
	{
		setupMode = false;
		lcd.noBacklight();
		LCD_brillo = false;
		lcd.noCursor();
	}
	// watch dog;
	//wdt_reset();
}

void loop()
{
	Alarm.delay(200);

	//Serial.print("BOTON: ");
	//Serial.println(analogRead(A7));
	if (setupBtn() == HIGH)
	{
		lcd.backlight();
		LCD_brillo = true;
		Alarm.disable(clockDisplay);
		Alarm.delay(250);
		setupMode = true;
		setupTimeOut = now() + 20;
		setupFunc();
		lcd.clear();
		Alarm.enable(clockDisplay);
	}
	if ((plusBtn() == HIGH || menosBtn() == HIGH) && !LCD_brillo)
	{
		lcd.backlight();
		LCD_brillo = true;
		Alarm.timerOnce(5, brillo_off);
	}

	// Comprobamos si es verano o invierno
	IoV = estacion();

	// comprovamos hora
	if (hour() == (long)invierno_on.hora && minute() == (long)invierno_on.minuto && IoV == INVIERNO)
	{
		rele_on();
	}
	else if (hour() == (long)verano_on.hora && minute() == (long)verano_on.minuto && IoV == VERANO)
		rele_on();
	else if (hour() == (long)invierno_off.hora && minute() == (long)invierno_off.minuto && IoV == INVIERNO)
		rele_off();
	else if (hour() == (long)verano_off.hora && minute() == (long)verano_off.minuto && IoV == VERANO)
		rele_off();

	if (!triger)
	{
		pir_sensor();
	}
	else
	{
		if (pirTimeOut && pirTimeOut <= now())
		{
			rele_off();
		}
	}

}

void brillo_off()
{
	if (LCD_brillo && !setupMode)
	{
		lcd.noBacklight();
		LCD_brillo = false;
	}
}

void rele_on()
{
	pirTimeOut = 0;
	if (triger) // Ya esta encendido
		return;
	if (crep.noche < foto_sensor && crep.dia < foto_sensor) // es dedia
		return;
	digitalWrite(rele, HIGH);
	lcd.backlight();
	LCD_brillo = true;
	Alarm.timerOnce(5, brillo_off);
	triger = true;
	//Serial.println("ENCENDIDO");
	//Serial.println(triger);
}

void rele_off()
{
	pirTimeOut = 0;
	if (!triger) // Ya esta apagado
		return;
	digitalWrite(rele, LOW);
	lcd.backlight();
	LCD_brillo = true;
	Alarm.timerOnce(5, brillo_off);
	triger = false;
	//Serial.println("APAGADO");
}

void rele_auto_off()
{
	foto_sensor = analogRead(fotoresistencia);
	if (crep.noche < foto_sensor && crep.dia < foto_sensor)
		rele_off();
	rele_auto_off_id = 0;
}

bool rango(char on, char off, char c)
{
	if (on>12 && off<12 && c>12)
	off = off +24;
	else if (on>12)// && off<12 && c<12)
	{
		off = off+24;
		c = c+24;
	}
	else if (on <12 && off < 12 && c>12)
	c= c-12;
	return (c>on && c<off);
}

char *formatTimeDigits (const int num)
{
	static char strOut[3];

	if (num >= 0 && num < 100) {
		sprintf(strOut, "%02d", num);
	} else {
		strcpy(strOut, "XX");
	}

	return strOut;
}
void printDigits(int digits)
{
	//Serial.print(formatTimeDigits(digits));
}

void pir_sensor(void)
{
	int pir_value;

	pir_value = analogRead(pir);
	//Serial.print("Pir value: ");
	//Serial.println(pir_value);
	if (pir_value > 400 && crep.noche > foto_sensor && crep.dia > foto_sensor)
	{
		rele_on();
		pirTimeOut = now() + (60*crep.tiempo);
	}
}
