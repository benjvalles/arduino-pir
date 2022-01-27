#ifndef _ajustes_
#define _ajustes_

/*#define setup_btn 6
#define plus_btn 7
#define menos_btn 8*/
#define led 5
#define rele 6
#define fotoresistencia A0
#define pir A1

#define VERANO true
#define INVIERNO false

typedef struct
{
  char hora;
  char minuto;
} HORA;

typedef struct
{
  unsigned short noche;
  unsigned short dia;
  unsigned char tiempo;
} CREPUSCULO;


// Botones analogicos
bool setupBtn();
bool plusBtn();
bool menosBtn();

void setupFunc(void);
void nextPantalla(const unsigned char index);
void setupHora(void);
void setupFecha(void);
void setupInvierno(const bool);
void setupCrep(void);
void setupManual(void);
void setupPir(void);
void setupSensor(void);




#endif
