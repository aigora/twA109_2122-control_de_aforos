const int alarma=9;
const int verde=10,rojo=12;
int duracion=5000;
//bool isAlarmOn=0; //almacena el estado de la alarma;
void setup() {
  // put your setup code here, to run once:
  pinMode(alarma,OUTPUT); // definir pin como salida 
  pinMode(verde,OUTPUT);
  pinMode(rojo,OUTPUT);
}
bool GetSystemState()
{
  return true; //cambiar en función del sensor usado
}

void loop() {
  // put your main code here, to run repeatedly:
  isAlarmOn= GetSystemState();
  while (opcion==3)
  {
  if(contador>aforo)
  {
    digitalWrite(verde,LOW);
    digitalWrite(rojo,HIGH);
    delay(duracion);
    digitalWrite(alarma,HIGH);
    delay(duracion);
  }
  else
  {
    digitalWrite(alarma,LOW);
    digitalWrite(verde,HIGH); 
    delay(duracion); 
    digitalWrite(rojo,LOW);
   
  }
}
}
