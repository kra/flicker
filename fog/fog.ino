#include <Metro.h>

// we turn on on game start, off on game end
#define START_TYPE "GameStart"
#define END_TYPE "GameOver"
#define BEAT_TYPE "BEAT"
#define START_COMMAND 1
#define END_COMMAND 2
#define BEAT_COMMAND 3
String startPreamble = START_TYPE; // MY_ADDR SEPARATOR;
String endPreamble = END_TYPE; // MY_ADDR SEPARATOR;
String beatPreamble = BEAT_TYPE; // MY_ADDR SEPARATOR;
const unsigned int baud = 9600;
// must be outside of function, http://arduino.cc/forum/index.php?topic=73177.0
static String command = "";
static boolean on = false;
static unsigned long lastBeat;        // milli of last beat received
const int beatInterval = 10000;       // die if no beat in 10 secs

const int gapMax = 15000;             // max length between fog events
const int gapMin = 5000;              // min length between fog events
const int eventsMin = 2;              // min times to fire during sequence
const int eventsMax = 6;              // max times to fire during sequence
const int onMin = 500;                // min event length
const int onMax = 7000;                // max event length
const int offMin = 100;               // min gap between event length
const int offMax = 1000;               // max gap between event length

static int pin = 11;
static Metro metro = Metro(gapMin);

// return the data part of the command
int parseCommand(const String &cmd)
{
  if (cmd.startsWith(startPreamble)) {
    return START_COMMAND;
  }
  if (cmd.startsWith(endPreamble)) {
    return END_COMMAND;
  }
  if (cmd.startsWith(beatPreamble)) {
    return BEAT_COMMAND;
  }
  // ignore the number part of the command  
  return NULL;
}

// read until we get a command, then return a command string
String getCommand()
{
  char c;
  while (Serial.available()) {
    c = Serial.read();
    if (c=='\n' || c=='\r') {
      String out = command;
      command="";
      return out;
    } else {
      command += c;
    }
  }
  return NULL;
}

// reset all effects
void setup_effects()
{
  lastBeat = millis();         // we received something
  digitalWrite(pin, LOW);
  metro.reset();              // is this necessary?
  metro.interval(gapMin);
}

// set up io, set up first effects, to be run once on start
void setup()
{
  Serial.begin(baud);	// USB, communication to PC or Mac
  pinMode(pin, OUTPUT);
  setup_effects();
}

// if metro has fired, frob pins and reload metro
void effect_loop()
{
  if (metro.check() == 1) {
    // frob pins, note we aren't checking for game events during this time
    for (int i = 0; i < random(eventsMin, eventsMax); i++) {
      digitalWrite(pin, HIGH);
      delay(random(onMin, onMax));
      digitalWrite(pin, LOW);
      delay(random(offMin, offMax));
    }
    // reload metro
    metro.reset();              // is this necessary?
    metro.interval(random(gapMin, gapMax));
  }
}

// main loop, to be run continuously
void loop()
{
  int cmd = parseCommand(getCommand());
  //Serial.println(cmd);
  if (cmd == START_COMMAND) {
    setup_effects();
    on = true;
    Serial.println("on");
  } else if (cmd == END_COMMAND) {
    setup_effects();
    on = false;
    Serial.println("off");
  } else if (cmd == BEAT_COMMAND) {
    lastBeat = millis();
    Serial.println("beat");
  }

  if (millis() - lastBeat > beatInterval) {
    // we didn't get a beat, did the game die?
    Serial.println("no beat");
    setup_effects();
    on = false;
  }

  if (on) {
    effect_loop();
  }
  delay(10);
}
