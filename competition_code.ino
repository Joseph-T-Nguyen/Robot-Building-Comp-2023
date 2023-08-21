/* Color sensor */
#define S0 A0
#define S1 A1
#define S2 A2
#define S3 A3
#define COL A4

/* Speaker */
#define SPK A5

/* Distance sensor */
#define TRF 2
#define ECF 3
#define TRR 4
#define ECR 5
#define TRL 6
#define ECL 7

/* Wheel */
#define IN2 8
#define ENR 9
#define ENL 10
#define IN1 11
#define IN3 12
#define IN4 13

/* Calibrated color frequencies */
#define NRGB 7 /* NOTE: keep in sync with below */
int RGB[NRGB][3] = {
  /* Ordered from low to high priority */
  {  16,  30,  26 }, /* black */
  {   6,   7,   6 }, /* white */
  {  11,  45,  34 }, /* red */
  {  16,  12,  20 }, /* green */
  {  48,  29,  13 }, /* blue */
  {  19,  35,  17 }, /* purple */
  {   7,  13,  22 }, /* yellow */
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

#define EAST 0
#define NORTH 1
#define WEST 2
#define SOUTH 3

struct cell {
  int color;
  int dist;
  struct cell *east;
  struct cell *north;
  struct cell *west;
  struct cell *south;
};

#define MAZE_SIZE 4
struct cell maze[MAZE_SIZE][MAZE_SIZE];
int src[2] = {3, 0};
int dst[2] = {2, 0};
int pos[2] = {src[0], src[1]};
int dir = NORTH;

void setup() {
  /* Open serial port at 9600 bps. */
  Serial.begin(9600);

  /* Set pin directions. */
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SPK, OUTPUT);

  pinMode(TRF, OUTPUT);
  pinMode(TRL, OUTPUT);
  pinMode(TRR, OUTPUT);
  pinMode(ECF, INPUT);
  pinMode(ECL, INPUT);
  pinMode(ECR, INPUT);

  pinMode(ENR, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENL, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  /* Set color sensor frequency scaling to 100%. */
  //digitalWrite(S0, HIGH);
  //digitalWrite(S1, HIGH);

  /* Set pin 9 & pin 10 PWM frequency to be 3921.16 Hz.
  This allow the wheels to have more torque while keeping RPM low.
  src: https://arduinoinfo.mywikis.net/wiki/Arduino-PWM-Frequency */
  TCCR1B = TCCR1B & B11111000 | B00000010;


  // Initialize maze as a fully connected graph
  for (int i=0; i<MAZE_SIZE; i++) {
    for (int j=0; j<MAZE_SIZE; j++) {
      maze[i][j].east = NULL;
      maze[i][j].north = NULL;
      maze[i][j].west = NULL;
      maze[i][j].south = NULL;
    }
  }
  for (int j=1; j<MAZE_SIZE; j++) {
    maze[0][j].west = &maze[0][j-1];
  }
  for (int i=1; i<MAZE_SIZE; i++) {
    maze[i][0].north = &maze[i-1][0];
  }
  for (int i=1; i<MAZE_SIZE; i++) {
    for (int j=1; j<MAZE_SIZE; j++) {
      maze[i][j].west = &maze[i][j-1];
      maze[i][j].north = &maze[i-1][j];
    }
  }

  Serial.println(pos[0]);
  Serial.println(pos[1]);
  Serial.println(dir);
}

void loop() {
  // Test distance sensors
  int left = distance(TRL, ECL);
  int front = distance(TRF, ECF);
  int right = distance(TRR, ECR);
  Serial.print(left);
  Serial.print("\t");
  Serial.print(front);
  Serial.print("\t");
  Serial.print(right);
  Serial.print("\n");

  hugleft();
}

#define DIST 30
#define RUNTIME 0
#define POWER 1

void hugleft() {
  //  speed(0,0);
//  delay(1);
  int left  = distance(TRL, ECL);
  int front  = distance(TRF, ECF);
  if (front < 12) {
    speed(-255, -255);
    delay(RUNTIME);
    return;
  }
  if(left <= 10) {
    speed(255,-100);
    delay(RUNTIME);
    Serial.println("right-correct");
    return;
    }
  if (front>=40 && left >=10) {
    speed(255, 255);
    delay(RUNTIME);
    Serial.println("forward");
    return;
  }
  if (left>=DIST && front>=DIST) {
    Serial.println("left");
    speed(-100, 100);
  } else if (left>=DIST && front<DIST) {
    Serial.println("left");
    speed(-100, 100);
  } else if (left<DIST && front>=DIST) {
    Serial.println("forward");
    speed(255, 255);
  } else if (left<DIST && front<DIST) {
    Serial.println("right");
    speed(100, -100);
  }
    delay(RUNTIME);
}

void beepbeep() {
  digitalWrite(LED_BUILTIN, HIGH);
  tone(A5, 30000, 1000);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);
  tone(A5, 30000, 1000);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
}

/* distance returns the distance in cm. */
int distance(int trig, int echo) {
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
  int R = pulseIn(COL, LOW);
  delay(COL);

  /* Green filter */
  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  int G = pulseIn(COL, LOW);
  delay(COL);

  /* Blue filter */
  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  int B = pulseIn(COL, LOW);
  delay(COL);

  /* Find closest color */
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
  digitalWrite(IN3, left>0 ? HIGH : LOW);
  digitalWrite(IN4, left>0 ? LOW : HIGH);
  analogWrite(ENL, abs(round(POWER * left)));

  digitalWrite(IN1, right>0 ? LOW : HIGH);
  digitalWrite(IN2, right>0 ? HIGH : LOW);
  analogWrite(ENR, abs(round(POWER * right)));
}
