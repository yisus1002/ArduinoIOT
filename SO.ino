#include "WiFi.h"
#include <WifiUDP.h>
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>
#include <ArduinoJson.h>
#include <Arduino_JSON.h>
#include <DNSServer.h>
#include <HTTPClient.h>

#include <stdlib.h>

const char* ssid = "JDGH";
const char* password = "1066508343@";

// Definir propiedades NTP
#define NTP_OFFSET   60 * 60                                                                                               // En segundos
#define NTP_INTERVAL 60 * 1000                                                                                             // En milisegundos
#define NTP_ADDRESS  "co.pool.ntp.org"                                                                                        // URL NTP

WiFiClient cliente;
WiFiUDP ntpUDP;                                                                                                            // Configura el cliente NTP UDP 

int pin_dos = 2;
int pin_cuatro = 4;

NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);
TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -300};  // Eastern Daylight Time = UTC - 4 hours
TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -360};   // Eastern Standard Time = UTC - 5 hours
Timezone usET(usEDT, usEST);

time_t local, utc;

const char * days[] = {"Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"} ;                        // Configurar Fecha y hora
const char * months[] = {"Ene", "Feb", "Mar", "Abr", "May", "Jun", "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"} ;            // Configurar Fecha y hora

JSONVar horario;
JSONVar activo;
JSONVar encender;

String horaLocal;
String diaLocal;

int rec=0;
int tocar=0;
int verificar=0;
int sonar=0;
int power=0;
int repetir=0;
int rep=0;



void setup() {

  pinMode(pin_dos, OUTPUT);
  pinMode(pin_cuatro, OUTPUT);

  Serial.begin(115200);

  WiFi.begin(ssid, password);
  digitalWrite(pin_cuatro, HIGH);
  delay(3000);
  digitalWrite(pin_cuatro, LOW);

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(pin_cuatro, HIGH);
    delay(500);
  WiFi.begin(ssid, password);
    rec=rec+1;
    Serial.println("Connecting to WiFi..");
    // Serial.println(rec);
    if(rec=120){
      rec=0;
  WiFi.begin(ssid, password);
  return;
    }
  }
  digitalWrite(pin_cuatro, LOW);
  Serial.println("Connected to the WiFi network");

}

void loop() {
 timeClient.update();                                                                                                // Actualizar el cliente NTP y obtener la marca de tiempo UNIX UTC
    unsigned long utc =  timeClient.getEpochTime();
    local = usET.toLocal(utc);                                                                                            // Convertir marca de tiempo UTC UNIX a hora local
    printTime(local);  
    HTTPClient https;
    HTTPClient httpss;
  digitalWrite(pin_cuatro, LOW);

  if((diaLocal!="Domingo") || (diaLocal!="Sabado") ){

      Serial.print("[HTTPS] begin...\n");
      if (https.begin(cliente, "http://62ccc2ba8042b16aa7d36742.mockapi.io/Horario")) {

            Serial.print("[HTTPS] GET...\n");
            int httpCode = https.GET();

            String payload = https.getString();
            if (httpCode < 0) {
                       Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
                        digitalWrite(pin_cuatro, HIGH);
                        https.end();
                        if (WiFi.status() != WL_CONNECTED){
                        WiFi.begin(ssid, password);
                        }
                        return;
            }
            
            JSONVar myObject = JSON.parse(payload.c_str()); 
           
              activo   = myObject[0]["activo"];
                    //  Serial.println(activo);
              horario  = myObject[0]["schedules"];
              encender = myObject[0]["tocar"]; 
              int tam =horario.length(); 
              JSONVar horasI[tam];
            if(!activo){


              for (int i = 0; i < horario.length(); i++){
                    String hI="";
                    horasI[i]= (horario[i]["start_time"]);
                    hI=JSON.stringify(horasI[i]);
                    hI=hI.substring(1,6);

                    if((hI==horaLocal) ){
                      String son = "";
                      son = JSON.stringify(horario[i]["sonara"]);
                      repetir = son.toInt();
                      if(repetir>2) repetir=repetir+1;
                      Serial.println("Son iguales");
                      verificar=1;
                      break;
                    }else{
                      verificar=0;
                    }
                }
              if(verificar==1){
                tocar= tocar+1;
                rep=rep +1;
              }else{
                tocar=0;
                rep=0;
              }
              
              if((tocar>0) && (tocar<=1)){
                Serial.println("Tocando timbre");
                digitalWrite(pin_dos, HIGH);
                digitalWrite(pin_dos, HIGH);
                digitalWrite(pin_dos, HIGH);
              }else if((tocar>1)|| (tocar==0)){
                Serial.println("Timbre desactivado");
                                
                digitalWrite(pin_dos, LOW);
                if(rep>repetir){
                    Serial.println("--------------------------");
                    digitalWrite(pin_dos, LOW);
                }else{
                  tocar=0;                  
                }


              }

              if(encender){
                digitalWrite(pin_dos, HIGH);
                
                apagar_timbre(httpss);
              }
            }else{
              Serial.println("Desactivado");
              digitalWrite(pin_dos, LOW);
              if(encender){
                apagar_timbre(httpss);
              }
            }

          
        if (httpCode > 0) {
            Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
                digitalWrite(pin_cuatro, LOW);
            // if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            //   String payload = https.getString();
            //   Serial.println(payload);
            // }
          } else {
            Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
                            digitalWrite(pin_cuatro, HIGH);
                            https.end();
        // Serial.printf("[HTTPS] ", https.errorToString(httpCode));
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }else{
  Serial.println("Es festivo");
  }



  Serial.println("Wait before next round...");
  delay(4000);  
}

void apagar_timbre(HTTPClient &httpss) {
  if (httpss.begin(cliente, "http://62ccc2ba8042b16aa7d36742.mockapi.io/Horario/1")){
    httpss.addHeader("Content-Type", "application/json");
    const char *modificar = "{\r\n    \"tocar\":false\r\n}";
    int httpco = httpss.PUT(modificar);
       httpss.end();
    Serial.println("Modificado");

  }
}

void printTime(time_t t)                                                                                              // Funcion para mandar hora por puerto serie    
{
  Serial.print("Hora local: ");
  Serial.println(convertirTimeATextoHora(t));
  Serial.println("Dia:");
  Serial.println(obtenerDia(t));

}

String convertirTimeATextoFecha(time_t t)                                                                               // Funcion para formatear en texto la fecha  
{
  String date = "";
  date += days[weekday(t)-1];
  date += ", ";
  date += day(t);
  date += " ";
  date += months[month(t)-1];
  date += ", ";
  date += year(t);
  return date;
}
String obtenerDia(time_t t){
  String date = "";
  date += days[weekday(t)-1];
  diaLocal= date;
  return date;
}

String convertirTimeATextoFechaSinSemana(time_t t)                                                                    // Funcion para formatear en texto la fecha sin dia de la semana
{
  String date = "";
  date += months[month(t)-1];
  date += "   ";
  date += year(t);
  return date;
}

String convertirTimeATextoHora(time_t t)                                                                              // Funcion para formatear en texto la hora                                                                          
{
  String hora ="";                                                                                                    // Funcion para formatear en texto la hora  
  if(hour(t) < 10)
  hora += "0";
  hora += hour(t);
  hora += ":";
  if(minute(t) < 10)                                                                                                  // Agregar un cero si el minuto es menor de 10
    hora += "0";
  hora += minute(t);
  horaLocal= hora;
  return hora;
}