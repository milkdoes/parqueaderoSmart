/*
	 Instrucciones de autor original.
	 BYJ48 Stepper motor code
Connect:
IN1 >> D8
IN2 >> D9
IN3 >> D10
IN4 >> D11
VCC ... 5V Prefer to use external 5V Source
Gnd
written By :Mohannad Rawashdeh
https://www.instructables.com/member/Mohannad+Rawashdeh/
28/9/2013
 */

// CONSTANTES.
// Baudios a los cuales operar.
const unsigned long BAUDIOS = 115200;
// Pasos maximos del motor.
const unsigned int PASOS_MAXIMOS = 4095;
// Pines en arduino a usar para operar el motor.
enum MOTOR {
	IN1 = 8,
	IN2 = 9,
	IN3 = 10,
	IN4 = 11
};

// VARIABLES GLOBALES.
int pasos = 0;
boolean direccion = true; // gre.
unsigned long ultimoTiempo;
unsigned long milisegundosActuales;
int pasosFaltantes = PASOS_MAXIMOS;
long tiempo;

// INICIALIZACION.
void setup()
{
	Serial.begin(BAUDIOS);
	pinMode(IN1, OUTPUT);
	pinMode(IN2, OUTPUT);
	pinMode(IN3, OUTPUT);
	pinMode(IN4, OUTPUT);
}

// PRINCIPAL.
void loop()
{
	while (pasosFaltantes > 0) {
		milisegundosActuales = micros();
		if ((milisegundosActuales - ultimoTiempo) >= 1000) {
			stepper(1);
			tiempo = (tiempo + micros() - ultimoTiempo);
			ultimoTiempo = micros();
			pasosFaltantes--;
		}
	}

	Serial.println(tiempo);
	Serial.println("Espera...!");
	delay(500);
	direccion = !direccion;
	pasosFaltantes = PASOS_MAXIMOS;
}

void stepper(int xw)
{
	for (int x = 0; x < xw; x++) {
		switch (pasos) {
			case 0:
				digitalWrite(IN1, LOW);
				digitalWrite(IN2, LOW);
				digitalWrite(IN3, LOW);
				digitalWrite(IN4, HIGH);
				break;
			case 1:
				digitalWrite(IN1, LOW);
				digitalWrite(IN2, LOW);
				digitalWrite(IN3, HIGH);
				digitalWrite(IN4, HIGH);
				break;
			case 2:
				digitalWrite(IN1, LOW);
				digitalWrite(IN2, LOW);
				digitalWrite(IN3, HIGH);
				digitalWrite(IN4, LOW);
				break;
			case 3:
				digitalWrite(IN1, LOW);
				digitalWrite(IN2, HIGH);
				digitalWrite(IN3, HIGH);
				digitalWrite(IN4, LOW);
				break;
			case 4:
				digitalWrite(IN1, LOW);
				digitalWrite(IN2, HIGH);
				digitalWrite(IN3, LOW);
				digitalWrite(IN4, LOW);
				break;
			case 5:
				digitalWrite(IN1, HIGH);
				digitalWrite(IN2, HIGH);
				digitalWrite(IN3, LOW);
				digitalWrite(IN4, LOW);
				break;
			case 6:
				digitalWrite(IN1, HIGH);
				digitalWrite(IN2, LOW);
				digitalWrite(IN3, LOW);
				digitalWrite(IN4, LOW);
				break;
			case 7:
				digitalWrite(IN1, HIGH);
				digitalWrite(IN2, LOW);
				digitalWrite(IN3, LOW);
				digitalWrite(IN4, HIGH);
				break;
			default:
				digitalWrite(IN1, LOW);
				digitalWrite(IN2, LOW);
				digitalWrite(IN3, LOW);
				digitalWrite(IN4, LOW);
				break;
		}
		AvanzarEnDireccion();
	}
}

void AvanzarEnDireccion()
{
	if (direccion == 1)
		pasos++;
	if (direccion == 0)
		pasos--;
	if (pasos > 7)
		pasos = 0;
	if (pasos < 0)
		pasos = 7;
}
