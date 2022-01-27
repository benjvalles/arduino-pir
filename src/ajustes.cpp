#include <Arduino.h>
//#include <avr/wdt.h>
#include <LiquidCrystal_I2C.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <Ds1302.h>
#include "ajustes.h"

extern LiquidCrystal_I2C lcd;
extern Ds1302 rtc;
extern volatile bool setupMode;
extern volatile time_t setupTimeOut;
extern volatile bool LCD_brillo;
extern bool triger;
extern unsigned short foto_sensor;

HORA invierno_on;
HORA invierno_off;
HORA verano_on;
HORA verano_off;
// Ajuste crepuscular
CREPUSCULO crep;

char *formatTimeDigits (const int num);
void rele_on(void);
void rele_off(void);
void rele_auto_off();
void save();



bool setupBtn()
{
	uint16_t boton = analogRead(A7);
	return (boton > 500 && boton < 600);
}

bool plusBtn()
{
	uint16_t boton = analogRead(A7);
	return (boton > 300 && boton < 400);
}

bool menosBtn()
{
	uint16_t boton = analogRead(A7);
	return (boton > 200 && boton < 300);
}

void opcionActual(const unsigned char opcion)
{
	lcd.setCursor(0, opcion%2);
	lcd.print(">");
	//Alarm.delay(125);
}

void setupFunc(void)
{
	char opcion = 0;
	lcd.clear();
	//lcd.blink();
	lcd.setCursor(2, 0);
	lcd.print("Ajuste de hora");
	lcd.setCursor(2, 1);
	lcd.print("Ajuste fecha");
	//lcd.cursor();
	opcionActual(opcion);
	while (setupMode)
	{
		Alarm.delay(200);
		if (plusBtn() == HIGH) // Boton mas
		{
			setupTimeOut = now() + 20;
			lcd.setCursor(0, opcion%2);
			lcd.print(" ");
			opcion++;
			if (opcion == 2)
				nextPantalla(1);
			else if (opcion == 4)
				nextPantalla(2);
			else if (opcion == 6)
				nextPantalla(3);
			else if (opcion == 8)
				nextPantalla(4);
			else if (opcion > 8)
			{
				opcion = 0;
				nextPantalla(0);
			}
			opcionActual(opcion);
		}
		else if (menosBtn() == HIGH) // Boton menos
		{
			setupTimeOut = now() + 20;
			lcd.setCursor(0, opcion%2);
			lcd.print(" ");
			opcion--;
			if (opcion == 1)
				nextPantalla(0);
			else if (opcion == 3)
				nextPantalla(1);
			else if (opcion == 5)
				nextPantalla(2);
			else if (opcion == 7)
				nextPantalla(3);
			else if (opcion < 0)
			{
				opcion = 8;
				nextPantalla(4);
			}
			opcionActual(opcion);
		}
		else if (setupBtn() == HIGH)
		{
			Alarm.delay(250);
			setupTimeOut = now() + 20;
			if (opcion == 0) // Ajuste de  hora
			{
				setupHora();
				nextPantalla(0);
				opcionActual(opcion);
			}
			else if (opcion == 1) // Ajuste de fecha
			{
				setupFecha();
				nextPantalla(0);
				opcionActual(opcion);
			}
			else if (opcion == 2) // Ajuste de horario de invierno
			{
				setupInvierno(INVIERNO);
				nextPantalla(1);
				opcionActual(opcion);
			}
			else if (opcion == 3) // Ajuste de horario de verano
			{
				setupInvierno(VERANO);
				nextPantalla(1);
				opcionActual(opcion);
			}
			else if (opcion == 4) // Ajuste crepuscular
			{
				setupCrep();
				nextPantalla(2);
				opcionActual(opcion);
			}
			else if (opcion == 5) // Encender o apagar luces
			{
				setupManual();
				nextPantalla(2);
				opcionActual(opcion);
			}
			else if (opcion == 6) // Ajuste del tiempo del pir
			{
				setupPir();
				nextPantalla(3);
				opcionActual(opcion);
			}
			else if (opcion == 7)	// Volver a leer el sensor luz
			{
				setupSensor();
				nextPantalla(3);
				opcionActual(opcion);
			}
			else
			{
				lcd.noBacklight();
				LCD_brillo = false;
				setupMode = false;
			}
		}
	}
}

void nextPantalla(const unsigned char index)
{
	if (index == 0)
	{
		lcd.clear();
		lcd.setCursor(2, 0);
		lcd.print("Ajuste de hora");
		lcd.setCursor(2, 1);
		lcd.print("Ajuste fecha");
	}
	else if (index == 1)
	{
		lcd.clear();
		lcd.setCursor(2, 0);
		lcd.print("Horario invierno");
		lcd.setCursor(2, 1);
		lcd.print("Horario verano");
	}
	else if (index == 2)
	{
		lcd.clear();
		lcd.setCursor(2, 0);
		lcd.print("Ajuste crepuscular");
		lcd.setCursor(2, 1);
		lcd.print("Modo manual");
	}
	else if (index == 3)
	{
		lcd.clear();
		lcd.setCursor(2, 0);
		lcd.print("Tiempo sensor");
		lcd.setCursor(2, 1);
		lcd.print("Leer luz");
	}
	else
	{
		lcd.clear();
		lcd.setCursor(2, 0);
		lcd.print("Salir");
	}
}

void setupHora(void)
{
	char hora = (unsigned char)hour();
	char minuto = (unsigned char)minute();
	Ds1302::DateTime dt;

	lcd.clear();
	lcd.print(formatTimeDigits(hour()));
	//lcd.setCursor(2,0);
	lcd.print(":");
	lcd.print(formatTimeDigits(minute()));
	lcd.setCursor(1,0);
	lcd.cursor();

	while(setupBtn() == LOW && setupMode)
	{
		Alarm.delay(200);
		if (plusBtn() == HIGH)
		{
			setupTimeOut = now() + 20;
			hora ++;
			if (hora > 23)
			hora = 0;
			lcd.noCursor();
			lcd.setCursor(0,0);
			lcd.print(formatTimeDigits(hora));
			lcd.setCursor(1,0);
			lcd.cursor();
		} else if (menosBtn() == HIGH)
		{
			setupTimeOut = now() + 20;
			hora --;
			if (hora < 0)
			hora = 23;
			lcd.noCursor();
			lcd.setCursor(0,0);
			lcd.print(formatTimeDigits(hora));
			lcd.setCursor(1,0);
			lcd.cursor();
		}
	}
	setupTimeOut = now() + 20;
	lcd.setCursor(4,0);
	lcd.cursor();
	Alarm.delay(250);
	while(setupBtn() == LOW && setupMode)
	{
		Alarm.delay(150);
		if (plusBtn() == HIGH)
		{
			setupTimeOut = now() + 20;
			minuto ++;
			if (minuto > 59)
			minuto = 0;
			lcd.noCursor();
			lcd.setCursor(3,0);
			lcd.print(formatTimeDigits(minuto));
			lcd.setCursor(4,0);
			lcd.cursor();
		} else if (menosBtn() == HIGH)
		{
			setupTimeOut = now() + 20;
			minuto --;
			if (minuto < 0)
			minuto = 59;
			lcd.noCursor();
			lcd.setCursor(3,0);
			lcd.print(formatTimeDigits(minuto));
			lcd.setCursor(4,0);
			lcd.cursor();
		}
	}

	lcd.noCursor();
	if (!setupMode)
	{
		// poner pantalla de inicio
		return;
	}
	Alarm.delay(125);
	setTime(hora,minuto,second(),day(),month(),year());
	dt = {
		 .year = (uint8_t)(year() - 2000),
		 .month = (uint8_t)(month()),
		 .day = (uint8_t)(day()),
		 .hour = (uint8_t)(hour()),
		 .minute = (uint8_t)(minute()),
		 .second = (uint8_t)(second()),
		 .dow = Ds1302::DOW_THU
	};
	//wdt_disable();
	rtc.setDateTime(&dt);
	setupTimeOut = now() + 20;
	//wdt_enable(WDTO_2S);
}

void setupFecha()
{
	unsigned char dia = (unsigned char)day();
	unsigned char mes = (unsigned char)month();
	unsigned short int a = (unsigned short int)year();
	Ds1302::DateTime dt;

	lcd.clear();
	lcd.print(formatTimeDigits(dia));
	//lcd.setCursor(2,0);
	lcd.print("/");
	lcd.print(formatTimeDigits(mes));
	lcd.print("/");
	lcd.print(a);
	lcd.setCursor(1,0);
	lcd.cursor();

	while(setupBtn() == LOW)
	{
		Alarm.delay(200);
		if (plusBtn() == HIGH)
		{
			setupTimeOut = now() + 20;
			dia ++;
			if (dia > 30)
			dia = 1;
			lcd.noCursor();
			lcd.setCursor(0,0);
			lcd.print(formatTimeDigits(dia));
			lcd.setCursor(1,0);
			lcd.cursor();
		} else if (menosBtn() == HIGH)
		{
			setupTimeOut = now() + 20;
			dia --;
			if (dia < 1)
			dia = 31;
			lcd.noCursor();
			lcd.setCursor(0,0);
			lcd.print(formatTimeDigits(dia));
			lcd.setCursor(1,0);
			lcd.cursor();
		}
	}
	lcd.setCursor(4,0);
	lcd.cursor();
	Alarm.delay(250);
	setupTimeOut = now() + 20;
	while(setupBtn() == LOW) // Ajuste de meses
	{
		Alarm.delay(200);
		if (plusBtn() == HIGH)
		{
			setupTimeOut = now() + 20;
			mes ++;
			if (mes > 12)
			mes = 1;
			lcd.noCursor();
			lcd.setCursor(3,0);
			lcd.print(formatTimeDigits(mes));
			lcd.setCursor(4,0);
			lcd.cursor();
		} else if (menosBtn() == HIGH)
		{
			setupTimeOut = now() + 20;
			mes --;
			if (mes < 0)
			mes = 12;
			lcd.noCursor();
			lcd.setCursor(3,0);
			lcd.print(formatTimeDigits(mes));
			lcd.setCursor(4,0);
			lcd.cursor();
		}
	}
	setupTimeOut = now() + 30;
	lcd.setCursor(9,0);
	lcd.cursor();
	Alarm.delay(250);
	while(setupBtn() == LOW && setupMode) // Ajuste de año
	{
		Alarm.delay(150);
		if (plusBtn() == HIGH)
		{
			setupTimeOut = now() + 20;
			a ++;
			lcd.noCursor();
			lcd.setCursor(6,0);
			lcd.print(a);
			lcd.setCursor(9,0);
			lcd.cursor();
		} else if (menosBtn() == HIGH)
		{
			setupTimeOut = now() + 20;
			a --;
			if (a < 2017)
			a = 2017;
			lcd.noCursor();
			lcd.setCursor(6,0);
			lcd.print(a);
			lcd.setCursor(9,0);
			lcd.cursor();
		}
	}
	lcd.noCursor();
	if (!setupMode)
	{
		// poner pantalla de inicio
		return;
	}
	Alarm.delay(125);
	setTime(hour(),minute(),second(),dia,mes,a);
	dt = {
		 .year = (uint8_t)(year() - 2000),
		 .month = (uint8_t)(month()),
		 .day = (uint8_t)(day()),
		 .hour = (uint8_t)(hour()),
		 .minute = (uint8_t)(minute()),
		 .second = (uint8_t)(second()),
		 .dow = Ds1302::DOW_THU
	};
	//wdt_disable();
	rtc.setDateTime(&dt);
	setupTimeOut = now() + 20;
	//wdt_enable(WDTO_2S);
}

void setupInvierno(const bool modo)
{
	char hora_on;
	char minuto_on;
	char hora_off;
	char minuto_off;

	if (modo == INVIERNO)
	{
		hora_on = invierno_on.hora;
		minuto_on = invierno_on.minuto;
		hora_off = invierno_off.hora;
		minuto_off = invierno_off.minuto;
	}
	else
	{
		hora_on = verano_on.hora;
		minuto_on = verano_on.minuto;
		hora_off = verano_off.hora;
		minuto_off = verano_off.minuto;
	}

	lcd.clear();
	lcd.print("Encendido: ");
	lcd.print(formatTimeDigits(hora_on));
	lcd.print(":");
	lcd.print(formatTimeDigits(minuto_on));
	lcd.setCursor(0,1);
	lcd.print("Apagado: ");
	lcd.print(formatTimeDigits(hora_off));
	lcd.print(":");
	lcd.print(formatTimeDigits(minuto_off));
	lcd.setCursor(12, 0);
	lcd.cursor();

	while(setupBtn() == LOW && setupMode) // Hora de encendido
	{
		Alarm.delay(200);
		if (plusBtn() == HIGH)
		{
			setupTimeOut = now() + 20;
			hora_on ++;
			if (hora_on > 23)
			hora_on = 0;
			lcd.noCursor();
			lcd.setCursor(11,0);
			lcd.print(formatTimeDigits(hora_on));
			lcd.setCursor(12,0);
			lcd.cursor();
		} else if (menosBtn() == HIGH)
		{
			setupTimeOut = now() + 20;
			hora_on --;
			if (hora_on < 0)
			hora_on = 23;
			lcd.noCursor();
			lcd.setCursor(11,0);
			lcd.print(formatTimeDigits(hora_on));
			lcd.setCursor(12,0);
			lcd.cursor();
		}
	}
	lcd.setCursor(15,0);
	lcd.cursor();
	setupTimeOut = now() + 20;
	Alarm.delay(250);
	while(setupBtn() == LOW && setupMode) // Minuto de encendido de invierno
	{
		Alarm.delay(150);
		if (plusBtn() == HIGH)
		{
			setupTimeOut = now() + 20;
			minuto_on ++;
			if (minuto_on > 59)
			minuto_on = 0;
			lcd.noCursor();
			lcd.setCursor(14,0);
			lcd.print(formatTimeDigits(minuto_on));
			lcd.setCursor(15,0);
			lcd.cursor();
		} else if (menosBtn() == HIGH)
		{
			setupTimeOut = now() + 20;
			minuto_on --;
			if (minuto_on < 0)
			minuto_on = 59;
			lcd.noCursor();
			lcd.setCursor(14,0);
			lcd.print(formatTimeDigits(minuto_on));
			lcd.setCursor(15,0);
			lcd.cursor();
		}
	}
	//Horarios de apagado
	Alarm.delay(250);
	lcd.setCursor(10, 1);
	lcd.cursor();
	setupTimeOut = now() + 20;
	while(setupBtn() == LOW && setupMode) // Hora de Apagado
	{
		Alarm.delay(200);
		if (plusBtn() == HIGH)
		{
			setupTimeOut = now() + 20;
			hora_off ++;
			if (hora_off > 23)
			hora_off = 0;
			lcd.noCursor();
			lcd.setCursor(9,1);
			lcd.print(formatTimeDigits(hora_off));
			lcd.setCursor(10,1);
			lcd.cursor();
		} else if (menosBtn() == HIGH)
		{
			setupTimeOut = now() + 20;
			hora_off --;
			if (hora_off < 0)
			hora_off = 23;
			lcd.noCursor();
			lcd.setCursor(9,1);
			lcd.print(formatTimeDigits(hora_off));
			lcd.setCursor(10,1);
			lcd.cursor();
		}
	}
	lcd.setCursor(13,1);
	lcd.cursor();
	Alarm.delay(250);
	setupTimeOut = now() + 20;
	while(setupBtn() == LOW && setupMode) // Minuto de apagado
	{
		Alarm.delay(150);
		if (plusBtn() == HIGH)
		{
			setupTimeOut = now() + 20;
			minuto_off ++;
			if (minuto_off > 59)
			minuto_off = 0;
			lcd.noCursor();
			lcd.setCursor(12,1);
			lcd.print(formatTimeDigits(minuto_off));
			lcd.setCursor(13,1);
			lcd.cursor();
		} else if (menosBtn() == HIGH)
		{
			setupTimeOut = now() + 20;
			minuto_off --;
			if (minuto_off < 0)
			minuto_off = 59;
			lcd.noCursor();
			lcd.setCursor(12,1);
			lcd.print(formatTimeDigits(minuto_off));
			lcd.setCursor(13,1);
			lcd.cursor();
		}
	}
	lcd.noCursor();
	setupTimeOut = now() + 20;
	if (!setupMode)
	{
		// poner pantalla de inicio
		return;
	}
	invierno_on.hora = hora_on;
	invierno_on.minuto = minuto_on;
	invierno_off.hora = hora_off;
	invierno_off.minuto = minuto_off;
	save();
}

void setupCrep(void)
{
	short noche = crep.noche;
	short dia = crep.dia;

	lcd.clear();
	lcd.print("Val. noche: ");
	lcd.print(noche);
	lcd.setCursor(0,1);
	lcd.print("Val. dia: ");
	lcd.print(dia);
	lcd.setCursor(12, 0);
	lcd.cursor();

	while(setupBtn() == LOW && setupMode)
	{
		Alarm.delay(200);
		if (plusBtn() == HIGH)
		{
			setupTimeOut = now() + 20;
			noche ++;
			if (noche > 600)
			noche = 0;
			lcd.noCursor();
			lcd.setCursor(12,0);
			lcd.print(noche);
			lcd.setCursor(12,0);
			lcd.cursor();
		} else if (menosBtn() == HIGH)
		{
			setupTimeOut = now() + 20;
			noche --;
			if (noche < 0)
			noche = 600;
			lcd.noCursor();
			lcd.setCursor(12,0);
			lcd.print(noche);
			lcd.setCursor(12,0);
			lcd.cursor();
		}
	}
	setupTimeOut = now() + 20;
	lcd.setCursor(10,1);
	lcd.cursor();
	Alarm.delay(250);
	while(setupBtn() == LOW && setupMode)
	{
		Alarm.delay(150);
		if (plusBtn() == HIGH)
		{
			setupTimeOut = now() + 20;
			dia ++;
			if (dia > 1000)
			dia = 100;
			lcd.noCursor();
			lcd.setCursor(10,1);
			lcd.print(dia);
			lcd.setCursor(10,1);
			lcd.cursor();
		} else if (menosBtn() == HIGH)
		{
			setupTimeOut = now() + 20;
			dia --;
			if (dia < 100)
			dia = 1000;
			lcd.noCursor();
			lcd.setCursor(10,1);
			lcd.print(dia);
			lcd.setCursor(10,1);
			lcd.cursor();
		}
	}
	lcd.noCursor();
	setupTimeOut = now() + 20;
	if (!setupMode)
	{
		// poner pantalla de inicio
		return;
	}
	setupTimeOut = now() + 20;
	crep.noche = noche;
	crep.dia = dia;
	save();
	Alarm.delay(125);
}

void setupManual()
{
	const char *on = "Encendido";
	const char *off = "Apagado  ";
	bool estado = triger;

	lcd.clear();
	lcd.print("Estado: ");
	lcd.setCursor(0,1);
	if (triger)
		lcd.print(on);
	else
		lcd.print(off);
	lcd.setCursor(0,1);
	lcd.cursor();

	Alarm.delay(250);
	while(setupBtn() == LOW && setupMode)
	{
		Alarm.delay(250);
		if (plusBtn() == HIGH || menosBtn() == HIGH)
		{
			setupTimeOut = now() + 20;
			estado = !estado;
			if (estado)
				lcd.print(on);
			else
				lcd.print(off);
			lcd.setCursor(0,1);
			lcd.cursor();
		}
	}
	lcd.noCursor();
	setupTimeOut = now() + 20;
	if (!setupMode)
	{
		// poner pantalla de inicio
		return;
	}
	if (estado)
		rele_on();
	else
		rele_off();
	Alarm.delay(125);
}

void setupPir()
{
	unsigned char t = crep.tiempo;

	lcd.clear();
	lcd.print("Tiempo encendido pir: ");
	lcd.setCursor(0,1);
	lcd.print(formatTimeDigits(t));
	lcd.setCursor(0,1);
	lcd.cursor();

	Alarm.delay(250);
	while(setupBtn() == LOW && setupMode)
	{
		Alarm.delay(200);
		if (plusBtn() == HIGH)
		{
			setupTimeOut = now() + 20;
			t ++;
			lcd.print(formatTimeDigits(t));
			lcd.setCursor(0,1);
			lcd.cursor();
		}
		else if (menosBtn() == HIGH)
		{
			setupTimeOut = now() + 20;
			t --;
			lcd.print(formatTimeDigits(t));
			lcd.setCursor(0,1);
			lcd.cursor();
		}
	}
	lcd.noCursor();
	setupTimeOut = now() + 20;
	if (!setupMode)
	{
		// poner pantalla de inicio
		return;
	}
	crep.tiempo = t;
	save();
	Alarm.delay(125);
}

void setupSensor()
{
	lcd.noBacklight();
	LCD_brillo = false;
	foto_sensor = analogRead(fotoresistencia);
	Alarm.delay(250);
	lcd.backlight();
	setupTimeOut = now() + 20;
}
