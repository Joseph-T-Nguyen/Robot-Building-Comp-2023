/* Color sensor pins */
#define S0 A0
#define S1 A1
#define S2 A2
#define S3 A3
#define COLOR A4

/* Left wheel pins */
#define ENL 11
#define IN3 12
#define IN4 13

/* Right wheel pins */
#define ENR 10
#define IN1 8
#define IN2 9

/* Distance sensor */
#define TRIGE 2
#define ECHOE 3
#define TRIGN 4
#define ECHON 5
#define TRIGW 6
#define ECHOW 7

#define SPEED 180 /* must be in [0,255] */
#define COLOR_DELAY 50 /* milliseconds */

/* Calibrated color frequencies */
#define NRGB 7 /* NOTE: keep in sync with below */
int RGB[NRGB][3] = {
	/* Ordered from low to high priority */
	{  16,  30,  26 }, /* black */
	{  6,  7,  6 }, /* white */
	{  11,  45,  34 }, /* red */
	{  16,  12,  20 }, /* green */
	{  48,  29,  13 }, /* blue */
	{  19,  35,  17 }, /* purple */
	{  7,  13,  22 }, /* yellow */
};
char *COLOR_NAME[NRGB] = {
	"BLACK",
	"WHITE",
	"RED",
	"GREEN",
	"BLUE",
	"PURPLE",
	"YELLOW",
};

void setup() {
	/* Open serial port at 9600 bps. */
	Serial.begin(9600);

	/* Set pin directions. */

	pinMode(ENR, OUTPUT);
	pinMode(IN1, OUTPUT);
	pinMode(IN2, OUTPUT);
	pinMode(ENL, OUTPUT);
	pinMode(IN3, OUTPUT);
	pinMode(IN4, OUTPUT);

	pinMode(S0, OUTPUT);
	pinMode(S1, OUTPUT);
	pinMode(S2, OUTPUT);
	pinMode(S3, OUTPUT);
	pinMode(COLOR, INPUT);

  pinMode(TRIGN, OUTPUT);
  pinMode(TRIGW, OUTPUT);
  pinMode(TRIGE, OUTPUT);
  pinMode(ECHON, INPUT);
  pinMode(ECHOW, INPUT);
  pinMode(ECHOE, INPUT);

	/* Set color sensor frequency scaling to 100%. */
	//digitalWrite(S0, HIGH);
	//digitalWrite(S1, HIGH);

	/* Set pin 9 & pin 10 PWM frequency to be 3921.16 Hz.
	This allow the wheels to have more torque while keeping RPM low.
	src: https://arduinoinfo.mywikis.net/wiki/Arduino-PWM-Frequency */
	//TCCR1B = TCCR1B & B11111000 | B00000010;

  delay(2000);
}

void loop() {
  speed(255, 255);
  Serial.println("^^^^^^");
  delay(3000);
  speed(-255, -255);
  Serial.println("vvvvvv");
  delay(3000);
/*
  int north = distance(TRIGN, ECHON);
  int west = distance(TRIGW, ECHOW);
  int east = distance(TRIGE, ECHOE);
  Serial.print(north);
  Serial.print("\t");
  Serial.print(west);
  Serial.print("\t");
  Serial.print(east);
  Serial.print("\n");
  */

  //color();
}

int distance(int trig, int echo) {
    digitalWrite(trig, LOW);
    delayMicroseconds(2);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);
    return pulseIn(echo, HIGH) * 0.034/2;
}

/* color returns the color index identified by the color sensor. */
int color() {
	/* Red filter */
	digitalWrite(S2, LOW);
	digitalWrite(S3, LOW);
	int R = pulseIn(COLOR, LOW);
	delay(COLOR_DELAY);

	/* Green filter */
	digitalWrite(S2, HIGH);
	digitalWrite(S3, HIGH);
	int G = pulseIn(COLOR, LOW);
	delay(COLOR_DELAY);

	/* Blue filter */
	digitalWrite(S2, LOW);
	digitalWrite(S3, HIGH);
	int B = pulseIn(COLOR, LOW);
	delay(COLOR_DELAY);

	/* Find closest Color */
  int out;
	int mindist = 999999;
	for (int col=0; col<NRGB; col++) {
		int dist =
			(R-RGB[col][0])*(R-RGB[col][0]) +
			(G-RGB[col][1])*(G-RGB[col][1]) +
			(B-RGB[col][2])*(B-RGB[col][2]);
		if (dist < mindist) {
			mindist = dist;
			out = col;
		}
	}

	/* Print frequencies and classified color */
	Serial.print(R);
	Serial.print("\t");
	Serial.print(G);
	Serial.print("\t");
	Serial.print(B);
	Serial.print("\t");
	Serial.print(COLOR_NAME[out]);
	Serial.print("\n");

  return out;
}

/* speed sets the speed of the wheels. left and right must be in [-255,255]. */
void speed(int left, int right) {
	digitalWrite(IN3, left>0 ? LOW : HIGH);
	digitalWrite(IN4, left>0 ? HIGH : LOW);
	analogWrite(ENL, abs(left));

	digitalWrite(IN1, right>0 ? HIGH : LOW);
	digitalWrite(IN2, right>0 ? LOW : HIGH);
	analogWrite(ENR, abs(right));
}