/* 
Codigo Tesis Modularizado.ino
* Asignatura: Trabajo de Grado II
* Archivo: Codigo Tesis Modularizado.ino
* Fecha creacion: 11 DE OCTUBRE DE 2017
* Fecha ultima modificacion: 11 DE JUNIO DE 2018
* Autores: Kevin Santiago Angulo Angulo y Miguel Angel Cardenas Diaz
* Responsabilidad: Este programa tiene la funcion de comunicarse con el modulo SIM 808 por comunicación serial,
                   Encender el GPS, Extraer y Guardar en variables los elementos más importantes para una aplicacion
                   Geográfica de Rastreo y Monitoreo, y enviarlas a un servidor web por peticion Http en Internet
                   Enviar un Mensaje de Texto SMS al usuario y realizar una Llamada de Voz al Usuario todo programado
                   de acuerdo a los tiempos de envio requeridos por el cliente
  ESCUELA DE INGENIERIA CIVIL Y GEOMATICA --LABORATORIO DE GEOPOSICIONAMIENTO-- PROGRAMA DE INGENIERIA TOPOGRAFICA
*/

#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

SoftwareSerial SIM808(7,8);
LiquidCrystal LCD(12,11,5,4,3,2);
//Definición de Variables Globales
char placa[8] = {'A','I','X','-','6','5','E'};
char gpschar[15]; char latichar[9]; char longichar[11]; char altichar[9]; char velchar[7];char cantsat[3];
char aniochr[5]; char meschr[3]; char diachr[3]; char horachr[3];char minchr[3];char segchr[3];
const unsigned long numnegativo = 4294936100; const unsigned long tiempoparqueo = 180000;
unsigned long inicio = 0; unsigned long foto1 = 0; unsigned long foto2 = 0; unsigned long foto3 = 0;unsigned long foto4 = 0;
unsigned long tiemposms2 = 300000; unsigned long tiempointernet = 15000;
unsigned int cont1 = 0; unsigned int cont2 = 0; unsigned int cont3 = 0;
const int LED = 6; const int BOTON = 13;
int val = 0; int state = 0; int old_val = 0; int cont_llamada = 0; int cont_llamada2 = 0;

void setup()                    //Configuraciones Iniciales
{
  //////////////////////////////Configuracion Inicial Modulo SIM808///////////////////////
  Serial.begin(9600);
  SIM808.begin(9600);
  pinMode(LED,OUTPUT); 
  pinMode(BOTON,INPUT);
  //Configuracion Inicial para Comunicacion con el Modulo SIM 808
  enviarAT("AT+IPR=9600",250);
  enviarAT("AT+CMGR=?",250);
  //Configuracion Inicial para GPS
  enviarAT("AT+CGNSPWR=1",250);
  enviarAT("AT+CGNSSEQ=RMC",250);
  LCD.begin(16,2);
  LCD.print("");
}

void loop()
{
  inicio = millis();
  escucharBoton();
  consultarGps("AT+CGNSINF",1000);
  arreglarFecha();
  if(consultarEstado() == true) //Vehiculo Estatico
  {
    if(cont1 == 1)
    {
      foto1 = millis();
    }
    if(inicio - foto1 >= numnegativo)
    {
      //
    }
    else
    {
      if(inicio - foto1 >= tiempoparqueo)
      {
        cont2++;
        if(cont2 == 1)
        {
          foto2 = millis();
          enviarSMS(1);
        }
        if(inicio - foto2 >= numnegativo)
        {
          //
        }
        else
        {
          if(inicio - foto2 >= tiemposms2)
          {
            enviarSMS(2);
            tiemposms2 = tiemposms2 + 300000;
          }
        }
      }
    }
  }
  else                        //Vehiculo en Movimiento
  {
    cont3++;
    if(cont3 == 1)
    {
      foto3 = millis();
    }
    cont_llamada++;
    if(cont_llamada == 5)
    {
      llamarUsuario();
      foto4 = millis();
      cont_llamada2 = 1;
    }
    if(inicio - foto3 >= numnegativo)
    {
      //
    }
    else
    {
      if(inicio - foto3 >= tiempointernet)
      {
        enviarDatoInternet();
        tiempointernet = tiempointernet + 15000;
      }
    }
  }
  if(state != 1)
  {
    cont_llamada = 0;
  }
  if(inicio - foto4 >= 300000 && cont_llamada2 == 1)
  {
    if(inicio - foto4 >= 4294916238)
    {
      //
    }
    else
    {
      llamarUsuario();
      cont_llamada2 = 0;
    }
  }
  LCD.setCursor (0,0);
  for (int i = 0; i < 16;i++)
  {
    LCD.write(' ');
  }
  if(consultarEstado() == true)
  {
    LCD.setCursor(0,0);
    LCD.print(latichar);LCD.print(horachr);LCD.print(":");LCD.print(minchr);LCD.print(":");LCD.print(segchr);
    LCD.setCursor(0,1);
    LCD.print(longichar);LCD.print(",");LCD.print("E=e");LCD.print(cantsat);
  }
  else
  {
    LCD.setCursor(0,0);
    LCD.print(latichar);LCD.print(horachr);LCD.print(":");LCD.print(minchr);LCD.print(":");LCD.print(segchr);
    LCD.setCursor(0,1);
    LCD.print(longichar);LCD.print(",");LCD.print("E=m");LCD.print(cantsat);   
  }
}

void enviarAT(const char *toSend, unsigned long milliseconds)
{
  SIM808.println(toSend);
  unsigned long startTime = millis();
  while (millis() - startTime < milliseconds)
  {
    if (SIM808.available())
    {
      char c = SIM808.read();
      Serial.write(c);
    }
  }
Serial.println();
}

void consultarGps(String comandoAT, const int timeout)
{
  String estado=""; String fechagps="";String latitud ="";String longitud =""; String altitud =""; String velocidad ="";String cantsatelites ="";
  String variables[15];
  for(int i=0;i<18;i++)
  {
    variables[i] = "";
  }
  SIM808.println(comandoAT);
  long int time = millis();
  int i = 0;
  while( (time+timeout) > millis())
  {
    while(SIM808.available())
    {
      char c = SIM808.read();
      if(c != ',')
      {
        variables[i]+=c;
      }
      else
      {
        i++;
      }
    }
  }
  estado = variables[1];
  fechagps = variables[2];
  latitud = variables[3];
  longitud = variables[4];
  altitud = variables[5];
  velocidad = variables[6];
  cantsatelites = variables[15];
  fechagps.toCharArray(gpschar,15);
  latitud.toCharArray(latichar,9);
  longitud.toCharArray(longichar,11);
  altitud.toCharArray(altichar,9);
  velocidad.toCharArray(velchar,7);
  cantsatelites.toCharArray(cantsat,3);
}

void enviarDatoInternet()
{
  //Cambiar por nueva URL
  char tramachr[72]= {'A','T','+','H','T','T','P','P','A','R','A','=','\"','U','R','L','\"',',','\"','g','e','o','p','r','o','c','e','s','s','.','c','o','m','.','c','o','/','a','r','d','u','i','n','o','t','e','s','t','/','i','n','s','e','r','t','a','r','.','p','h','p','?','m','y','_','c','o','o','r','d','='};
  //char tramachr[72]= "AT+HTTPPARA=\"URL\",\"geoprocess.com.co/arduinotest/insertar.php?my_coord=";
  enviarAT("AT+SAPBR=3,1,\"Contype\",\"GPRS\"",500);
  enviarAT("AT+SAPBR=3,1,\"APN\",\"internet.comcel.com.co\"",500);
  enviarAT("AT+SAPBR=1,1",500);
  enviarAT("AT+SAPBR=2,1",500);
  enviarAT("AT+HTTPINIT",500);
  enviarAT("AT+HTTPPARA=\"CID\",1",500);
  SIM808.print(tramachr); //Cambiar por nueva URL
  SIM808.print("placa_datos="); //Agregar &
  SIM808.print(placa);
  SIM808.print("lat="); //Agregar &
  SIM808.print(latichar);
  SIM808.print("lng="); //Agregar &
  SIM808.print(longichar);
  SIM808.print("h=");  //Agregar &
  SIM808.print(altichar);
  SIM808.print("vel=");  //Agregar &
  SIM808.print(velchar);
  SIM808.print("fecha="); //Agregar &
  SIM808.print(aniochr);
  SIM808.print("/");
  SIM808.print(meschr);
  SIM808.print("/");
  SIM808.print(diachr);
  SIM808.print("hora_local=");
  SIM808.print(horachr);
  SIM808.print(":");
  SIM808.print(minchr);
  SIM808.print(":");
  SIM808.print(segchr);
  SIM808.print("\"");
  enviarAT("",500);
  enviarAT("AT+HTTPACTION=1",5000);
  enviarAT("AT+HTTPTERM",500);
  enviarAT("AT+SAPBR=0,1",500);
}

void enviarSMS(int cond)
{
  char sms_1[63] = {'V','e','h','i','c','u','l','o',' ','E','s','t','a','c','i','o','n','a','d','o',' ','H','a','c','e',' ','1','2','m','i','n',' ','a','q','u','i',' ','h','t','t','p',':','/','/','m','a','p','s','.','g','o','o','g','l','e','.','e','s','/','?','q','='};
  char sms_2[62] = {'V','e','h','i','c','u','l','o',' ','E','s','t','a','c','i','o','n','a','d','o',' ','H','a','c','e',' ','3','H','r','s',' ','a','q','u','i',' ','h','t','t','p',':','/','/','m','a','p','s','.','g','o','o','g','l','e','.','e','s','/','?','q','='};
  enviarAT("AT+CMGF=1",1000);
  enviarAT("AT+CMGS=\"3015252223\"",1000);
  if(cond == 1)
  {
    SIM808.print(sms_1);
    SIM808.print(latichar);
    SIM808.print("%20");
    SIM808.print(longichar);
    SIM808.print(char(26));
    SIM808.println();
  }
  else
  {
    SIM808.print(sms_2);
    SIM808.print(latichar);
    SIM808.print("%20");
    SIM808.print(longichar);
    SIM808.print(char(26));
    SIM808.println();
  }
}

void llamarUsuario()
{
  if (state==1)
  {
   enviarAT("ATD3015252223;",1000);
   delay(23000);
   enviarAT("ATH",1000);
   enviarAT("ATD3172547377;",1000);
   delay(23000);
   enviarAT("ATH",1000);
  }
  else
  {
   // 
  }
}

bool consultarEstado()
{
  float latifloat = atof(latichar);
  float longifloat = atof(longichar);
  float velfloat = atof(velchar);
  if(latifloat <= latifloat+0.0001 && latifloat >= latifloat-0.0001 && longifloat <= longifloat +0.0001 && longifloat >= longifloat -0.0001 && velfloat <= 1.5 && velfloat >= 0.00)
  {
    cont1++;
    cont3 = 0;
    tiempointernet = 30000;
    return true;
  }
  else
  {
    cont1 = 0;
    cont2 = 0;
    tiemposms2 = 10800000;
    return false; 
  }
}

void arreglarFecha()
{
  int dias[5] = {1,28,29,30,31};
  int meses[12] = {1,2,3,4,5,6,7,8,9,10,11,12};
  int anio,mes,dia,hora;

  for(int i=0;i<4;i++)
  {
   aniochr[i] = gpschar[i];
   if(i>=0 && i<2)
   {
     meschr[i] = gpschar[i+4];
     diachr[i] = gpschar[i+6];
     horachr[i] = gpschar[i+8];
     minchr[i] = gpschar[i+10];
     segchr[i] = gpschar[i+12];
   }
  }
  anio = atoi(aniochr);
  mes = atoi(meschr);
  dia = atoi(diachr);
  hora = atoi(horachr);
  if(dias[0] == dia && hora <= 4 && (meses[1] == mes || meses[3] == mes || meses[5] == mes || meses[7] == mes || meses[8] == mes || meses[10] == mes))
  {
    hora = (hora - 5) + 24;
    mes = mes - 1;
    dia = dias[4];
    anio = anio;
  }
  else if(dias[0] == dia && hora <= 4 && meses[0] == mes)
  {
    hora = (hora - 5) + 24;
    mes = meses[11];
    dia = dias[4];
    anio = anio - 1;
  }
  else if(dias[0] == dia && hora <= 4 && (meses[4] == mes || meses[6] == mes || meses[9] == mes || meses[11] == mes))
  {
    hora = (hora - 5) + 24;
    mes = mes - 1;
    dia = dias[3];
    anio = anio;
  }
  else if(dias[0] == dia && hora <= 4 && meses[2] == mes && anio % 4 == 0)
  {
    hora = (hora - 5) + 24;
    mes = mes - 1;
    dia = dias[2];
    anio = anio;
  }
  else if (dias[0] == dia && hora <= 4 && meses[2] == mes)
  {
    hora = (hora - 5) + 24;
    mes = mes - 1;
    dia = dias[1];
    anio = anio;
  }
  else
  {
    if(hora <= 4)
    {
      hora = (hora - 5) + 24;
      dia = dia - 1;
      mes = mes;
      anio = anio;
    }
    else
    {
      hora = hora - 5;
      dia = dia;
      mes = mes;
      anio = anio;
    }
  }
  itoa(anio,aniochr,10);
  itoa(mes,meschr,10);
  itoa(dia,diachr,10);
  itoa(hora,horachr,10);
}

void escucharBoton()
{
  val = digitalRead(BOTON); 
  if ((val == HIGH) && (old_val == LOW))
  {
    state = 1-state;
    delay(1000);
  }
  if (state == 1)
  {
     digitalWrite(LED, HIGH);
  }
  else
  {
    digitalWrite(LED,LOW);
  }
  old_val = val;
}

