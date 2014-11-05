// we turn on on game start, off on game end
// #define START_TYPE "GameStart"
// #define END_TYPE "GameOver"
#define BEAT_TYPE "BEAT"
#define DEATH_TYPE "3on"

const unsigned int whistle_pin = 12;
const unsigned int puffer_pin = 13;
const unsigned int foo_pin = 11;
unsigned int whistle_len = 500;

// #define START_COMMAND 1
// #define END_COMMAND 2
#define BEAT_COMMAND 3
#define DEATH_COMMAND 4
// String startPreamble = START_TYPE;
// String endPreamble = END_TYPE;
String beatPreamble = BEAT_TYPE;
String DeathPreamble = DEATH_TYPE;
const unsigned int baud = 9600;
// must be outside of function, http://arduino.cc/forum/index.php?topic=73177.0
static String command = "";
static boolean on = false;
static unsigned long lastBeat;        // milli of last beat received
const int beatInterval = 10000;       // die if no beat in 10 secs

// return the data part of the command
int parseCommand(const String &cmd)
{
  if (cmd.startsWith(DeathPreamble)) {
    return DEATH_COMMAND;
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

// set up io, set up first effects, to be run once on start
void setup()
{
  Serial.begin(baud);	// USB, communication to PC or Mac
  for (int i=9; i<=17; i++) {
    pinMode(i, OUTPUT);
    }   
  digitalWrite(whistle_pin, LOW);
  digitalWrite(puffer_pin, LOW);
  digitalWrite(foo_pin, LOW);
}

// main loop, to be run continuously
void loop()
{
  int cmd = parseCommand(getCommand());
  if (cmd == DEATH_COMMAND) {
    Serial.println("death start");
    digitalWrite(whistle_pin, HIGH);
    digitalWrite(puffer_pin, HIGH);
    digitalWrite(foo_pin, HIGH);
    delay(whistle_len);
    digitalWrite(whistle_pin, LOW);
    digitalWrite(puffer_pin, LOW);
    digitalWrite(foo_pin, LOW);
    Serial.println("death end");
  } else if (cmd == BEAT_COMMAND) {
    Serial.println("beat");
    lastBeat = millis();
  }
  if (millis() - lastBeat > beatInterval) {
    // we didn't get a beat, did the game die?
    Serial.println("no beat");
    digitalWrite(whistle_pin, LOW);
    digitalWrite(puffer_pin, LOW);
    digitalWrite(foo_pin, LOW);
  }
}
