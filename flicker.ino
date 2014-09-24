#include <Metro.h>
#include <Queue.h>

// communication stuff

// we turn on on game start, off on game end
#define START_TYPE "GameStart"
#define END_TYPE "GameOver"
#define BEAT_TYPE "BEAT"
//#define MY_ADDR "1"
//#define SEPARATOR ":"
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

// effect stuff

// hold timing information for an effect; pin times are randomly generated
// based on these values
struct effect {
  float onbase;    // maximum on time for action
  float onmin;     // minimum on time for action
  float onmax;     // maximum time onbase can be inceased to
  float lenbase;   // maximum number of actions per group
  float lenmax;    // maximum number of actions group can be increased to
  float gapbase;   // maximum off time between groups
  float gapmin;    // minumum time gapbase can be reduced to
  float offmin;    // minimum off time between action
  float offmax;    // maximum off time between action
};

// number of pins
static const int pinlen = 7;
// penlen pins, 9-10 are jacob's ladders
// pins 12-14 are arcers
// pins 16-17 are helmet
static const int pins[] = {9, 10, 12, 13, 14, 16, 17};
// pinlen states, everything starts off
static int states[] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW};

// pinlen effects to hold pin timings to be randomized
static effect effects[] = {
  effect(), effect(), effect(), effect(), effect(), effect(), effect()};
// pinlen metros to fire the next on/off event for each pin
// first 2 effects are jacob's ladders,
// next 4 are arcers,
// final 1 are helmet
static Metro metros[] = {
  Metro(12800), Metro(6400),
  Metro(1600), Metro(800), Metro(400), Metro(200),
  Metro(1600)};
// pinlen queues to hold the current sequence of events for each pin
// must be able to hold lenmax * 2 + 2
static Queue queues[] = {
  Queue(50), Queue(50), Queue(50), Queue(50), Queue(50), Queue(50), Queue(50)};

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

// push an even parity group of floats to the qth queue,
// based on the qth effect.
void loadQueue(int q)
{
  // push even parity group of ons and offs
  for (int i = 0;
       i < random((effects[q].lenbase / 2), effects[q].lenbase);
       i++) {
    queues[q].push(random(effects[q].onmin, effects[q].onbase));
    queues[q].push(random(effects[q].offmin, effects[q].offmax));
  }
  // push a final on to make us odd parity
  queues[q].push(random(effects[q].onmin, effects[q].onbase));
  // push final off to make us even parity; delay between next group
  queues[q].push(random(effects[q].gapbase / 2, effects[q].gapbase));
}

// toggle the given pins array value
void togglePin(int pin)
{
  if (states[pin] == HIGH)  { 
    states[pin] = LOW;
  } 
  else {
    states[pin] = HIGH;
  }
  digitalWrite(pins[pin], states[pin]);
}

// reset all effects
void setup_effects()
{
  lastBeat = millis();         // we received something
  for (int i = 0; i < pinlen; i++) {
    states[i] = LOW;
    digitalWrite(pins[i], states[i]);
  }
  long metro_interval = 128000;
  for (int i = 0; i < pinlen; i++) {
    metros[i].interval(metro_interval);
    metro_interval = metro_interval / 2;
  }
  for (int i = 0; i < 2; i++) {
    // first 2 effects are jacob's ladders
    effects[i].onbase = 4000;
    effects[i].onmin = 2000;
    effects[i].onmax = 4000;
    effects[i].lenbase = 0;     // XXX does a lenbase/lenmax 0 make nothing?
    effects[i].lenmax = 0;
    effects[i].gapbase = 20000;
    effects[i].gapmin = 750;
    effects[i].offmin = 0;      // should not be used
    effects[i].offmax = 0;      // should not be used
  }
  for (int i = 2; i < pinlen; i++) {
    // rest of the effects are arcers
    effects[i].onbase = 100;
    effects[i].onmin = 3;
    effects[i].onmax = 1250;
    effects[i].lenbase = 1;
    effects[i].lenmax = 10;
    effects[i].gapbase = 10000;
    effects[i].gapmin = 750;
    effects[i].offmin = 10;
    effects[i].offmax = 200;
  }
}

// set up io, set up first effects, to be run once on start
void setup()
{
  Serial.begin(baud);	// USB, communication to PC or Mac
  for (int i = 0; i < pinlen; i++) {
    pinMode(pins[i], OUTPUT);
  }
  setup_effects();
}

// print diagnostics
void diag(int i)
{
  Serial.println(i);
  if (i <= 5) {
      return;  
  }
  // Serial.println("diag");
  Serial.println("onbase");
  Serial.println(effects[i].onbase);
  Serial.println("onmin");
  Serial.println(effects[i].onmin);
  Serial.println("onmax");
  Serial.println(effects[i].onmax);
  Serial.println("lenbase");
  Serial.println(effects[i].lenbase);
  Serial.println("lenmax");
  Serial.println(effects[i].lenmax);
  Serial.println("gapbase");
  Serial.println(effects[i].gapbase);
  Serial.println("gapmin");
  Serial.println(effects[i].gapmin);
  Serial.println();
}

// return random float between min and max
float randomFloat(float min, float max)
{
  return (float)random((long)(min * 100), (long)(max * 100)) / 100.0;
}

// reload empty queues, fire pins and reload metros that have fired
void effect_loop()
{
  for (int i = 0; i < pinlen; i++) {
    if (queues[i].isEmpty()) {
      // We've used up this sequence of firings, speed up and load another.
      effects[i].onbase = min(
          randomFloat(effects[i].onbase * 1.05, effects[i].onbase * 1.1),
          effects[i].onmax);
      effects[i].lenbase = min(
          randomFloat(effects[i].lenbase * 1.05, effects[i].lenbase * 1.1),
          effects[i].lenmax);
      effects[i].gapbase = max(
          randomFloat(effects[i].gapbase * 0.9, effects[i].gapbase * 0.95),
          effects[i].gapmin);
      loadQueue(i);
      diag(i);
    }
    if (metros[i].check() == 1) {
      // The metro has fired.  Toggle its pin and tell it to fire when
      // the pin should be toggled again.
      togglePin(i);
      metros[i].reset();        // XXX necessary?
      metros[i].interval(queues[i].pop());
    }
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
