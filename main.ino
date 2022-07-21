/*

  header       Byte1         byte2       byte3
  ---------------------------------------------------
  2=MTC       240=STOP,    clocktick
          241=clocktick    counted
  --------------------------------------------------
  8=noteOff   Channel         note       velocity
              (-127)
  ---------------------------------------------------
  9=noteOn    Channel         note       velocity
              (-143)
  ---------------------------------------------------
  11=CC       Channel         CC#        CC Value
              (-175)
  ---------------------------------------------------
  15=clock   tick=248
            Start=250
            Stop=252
*/

#include <Adafruit_NeoTrellisM4.h>
Adafruit_NeoTrellisM4 trellis = Adafruit_NeoTrellisM4();
#define MIDI_CHANNEL 0 // default channel # is 0

#define TICKS_PER_STEP 6

int tickcount = 0;
int beatcount = 0;
int STEP;                 // variable for "step" selection
int INST;                 // variable for "instrument" selection
const int inst = 8;       // number of instruments
const int steps = 16;     // number of steps
bool pattern[inst][steps]{// Array for the "inst" step sequencers
                          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
int pixelcolor[inst]{3093151, 16616016, 3263577, 3249497, 16776990}; // color of the instruments
int instcolor;
int inst0 = 1;

void setup()
{
  Serial.begin(115200);
  trellis.begin();
  trellis.setBrightness(80);
  trellis.enableUSBMIDI(true);
  trellis.setUSBMIDIchannel(MIDI_CHANNEL);
  for (int i = 0; i <= 15; i++)
  {
    trellis.setPixelColor(i, pixelcolor[0]); // set pixels 0-16 to first instrument color
  }
}

void loop()
{
  trellis.tick();
  while (trellis.available())
  {                                 // while we hit buttons..
    keypadEvent e = trellis.read(); // standard stuff, not really used
    int key = e.bit.KEY;

    for (int i = 16; i <= 23; i++)
    { // this counts for all the instrument selections
      if (trellis.isPressed(i))
      {                // if we choose a instrument
        INST = i - 16; // set the instrument-selector to "pressed button"-16
        for (STEP = 0; STEP < steps; STEP++)
        { // this counts for all choosen steps
          if (pattern[INST][STEP] == HIGH)
          {                                       // if any of the choosen instrument steps is high
            trellis.setPixelColor(STEP, 7340032); // set step led color to red
          }
          else
          {
            pattern[INST][STEP] = LOW;                     // if any other step is low
            trellis.setPixelColor(STEP, pixelcolor[INST]); // make it the instrument color.
          }
        }
        if (e.bit.EVENT == KEY_JUST_PRESSED)
        { // now to look if any of the step buttons is hit (with the instrument-button,(we´re still in the first for-loop!))
          if (e.bit.KEY < steps)
          {                   // check that we only hit on of the step buttons
            STEP = e.bit.KEY; // set the clicked step-button as the Step-selector
            if (pattern[INST][STEP] == LOW)
            {                                       // if any of the choosen instrument steps is low
              pattern[INST][STEP] = HIGH;           // make it high
              trellis.setPixelColor(STEP, 7340032); // make the selected step red
            }
            else
            {                                                // if it was already high
              pattern[INST][STEP] = LOW;                     // make it low
              trellis.setPixelColor(STEP, pixelcolor[INST]); // make the selected step to instrument color
            }
          } // boom
        }   // bamm
      }     // chack
    }       // pow-wee
  }         //  we are done with checking what is pressed and filling the array with active (high) steps

  midiEventPacket_t rx; // now we look at midi
  rx = MidiUSB.read();  // read incoming midi

  if (rx.header == 15 && rx.byte1 == 252)
  {                // clock stop
    tickcount = 0; // reset the tick counter
    beatcount = 0; // reset the beatcounter
  }

  if (rx.header == 15 && rx.byte1 == 250)
  { // clock start
    // yeah
  }

  if (rx.header == 15 && rx.byte1 == 248)
  { // clock tick
    tickcount++;
  }

  if (tickcount % TICKS_PER_STEP == 0) // every 6th clock ticker
  {                
    beatcount = (tickcount/TICKS_PER_STEP) % steps;  // find the loop position from the overall tickcount

    // loop over each instrument
    for (int i = 0 ; i < inst ; i++ ) {
      if (pattern[i][beatcount] == HIGH) { // if the instrument is set to high at this point
        trellis.noteOn(i + 36, 100); // play the corresponding midi-note
      } else {
        trellis.noteOff(i + 36, 0); // if its low, shut up
        // todo: you probably want to replace this with an array to track how long the instrument has been playing and only send note off when both
        // the note is already on AND a duration has elapsed 
        // otherwise you are sending note offs immediately, and every step, unnecessarily
      }                                
      trellis.setPixelColor(beatcount - 1, pixelcolor[i]); // set the last played positionmarker to instrument color , here i tried to replace with different if statements with no luck
      trellis.setPixelColor(beatcount, 24577);             // set positionmarker to green
      if (beatcount == 0) {                                // additional line to handle the annoying 16th step color
        trellis.setPixelColor(15, pixelcolor[i]); // here you have you stupid 16th step
      }
    }

  }
} // end of code. yehaaw
