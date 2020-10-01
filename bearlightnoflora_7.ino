#include <Wire.h>
#include <SPI.h>
#include <Adafruit_CircuitPlayground.h>
//#include <Adafruit_TCS34725.h>
#include <FlashStorage.h>
#include "irremotecodes.h"
#include "colourrgbs.h"
#include "sounds.h"

#define SCANDELAY 100
#define SHORTDELAY 100
#define LONGDELAY 500
#define HOLDDELAY 3000
#define SAVEDELAY 10000

// Create a structure The "valid" variable is set to "true" once
// the structure is filled with actual data for the first time.
typedef struct {
  boolean valid;
  unsigned long flashcolour1;
  unsigned long flashcolour2;
  unsigned long flashcolour3;
  unsigned long flashcolour4;
  int flashanim3 = 0;
  int flashanim4 = 0;
  int flashbrightness3 = 2;
  int flashbrightness4 = 2;
} Storage;

int anim3 = 0;
int anim4 = 0;
int brightness3 = 2;
int brightness4 = 2;
int animation = 0;
int animationstep = 0;
int animationdelay = HOLDDELAY;
int lastanimation = 0;
int i = 0;
int j = 0;
int doubletapped = 0;
int brightness = 0;
int animationbrightness = 0;
int scancode = 0;
int lastscancode = 0;
int readytostore = 0;
int secret = 0;
int ack = 0; //1: Wurr, 2: On, 3: Off, 4: Hmm, 5: Ok, 6: Oops
unsigned long targetcolour = CLRYELLOW;
unsigned long animationcolour = CLRYELLOW;
unsigned long colour1 = 0;
unsigned long colour2 = 0;
unsigned long colour3 = 0;
unsigned long colour4 = 0;
unsigned long pixelcolours[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned long lastanim = 0;
unsigned long lastscan = 0;
unsigned long lastir = 0;
unsigned long lastbutton = 0;
unsigned long lastready = 0;

//Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
FlashStorage(flashstorage, Storage);
Storage savedcolours;

void isrdoubletapped(void)
{
  doubletapped = 1;
}

void turnon(void)
{
  randomSeed(millis());
  brightness = 1;
  targetcolour = CLRYELLOW;
  animationstep = 0;
  if(CircuitPlayground.readCap(6) > 400)
  {
    ack=1;
    animation = 12;
    secret = 1;
    CircuitPlayground.redLED(secret);
  }
  else
  {
    animation = 0;
  }
  animate();
}

void turnoff(void)
{
  if(secret==1)
  {
    ack=1;
    secret = 0;
    CircuitPlayground.redLED(secret);
  }
  else
  {
    brightness = 0;
    targetcolour = CLRYELLOW;
    animation = 0;
    animationstep = 0;
  }
  animate();
}

void brightnessup(void)
{
  brightness++;
  if(brightness>5) { brightness = 5; ack = 6; }
  if(animation)
  {
    ack = 1;
    animationbrightness++;
    if(animationbrightness>5) { animationbrightness = 5; ack = 6; }
  }
  animate();
}

void brightnessdown(void)
{
  brightness--;
  if(brightness<1) { brightness = 1; ack = 6; }
  if(animation)
  {
    ack = 1;
    animationbrightness--;
    if(animationbrightness<1) { animationbrightness = 1; ack = 6; } 
  }
  animate();
}

void animate(void)
{
  if(animation!=lastanimation)
  {
    animationstep = 0;
    animationcolour = 0;
    animationbrightness = brightness;
    lastanimation = animation;
  }
  Serial.println("Animation: " + String(animation) + " | Step: " + String(animationstep) + " | Brightness: " + String(brightness) + " | Animation brightness: " + String(animationbrightness));
  switch(animation)
  {
    case 0: //No animation
      {
        for(i=0;i<10;i++) 
        {
          pixelcolours[i] = targetcolour;
        }
        
        if(CircuitPlayground.slideSwitch())
        {
          Serial.println("Cap  6: " + String(CircuitPlayground.readCap(6)));
          Serial.println("Cap  9: " + String(CircuitPlayground.readCap(9)));
          Serial.println("Cap 10: " + String(CircuitPlayground.readCap(10)));
          Serial.println("Cap  2: " + String(CircuitPlayground.readCap(2)));
          Serial.println("Cap 3: " + String(CircuitPlayground.readCap(3)));
          Serial.println("Cap  0: " + String(CircuitPlayground.readCap(0)));
          Serial.println("Cap  1: " + String(CircuitPlayground.readCap(1)));
        }
        
        animationdelay = HOLDDELAY;
      }
      break;
    case 1: //Flashing
      {
        if(animationstep==0)
        {
          animationstep=1;
          for(i=0;i<10;i++) 
          {
            pixelcolours[i] = 0;
          }
        }
        else
        {
          animationstep=0;
          for(i=0;i<10;i++) 
          {
            pixelcolours[i] = targetcolour;
          }
          
        }
        animationdelay = LONGDELAY;
      }
      break;
    case 2: //Alternate flashing
      {
        if(animationstep==0)
        {
          animationstep=1;
          for(i=0;i<5;i++) 
          {
            pixelcolours[i*2] = 0;
            pixelcolours[(i*2)+1] = targetcolour;
          }
        }
        else
        {
          animationstep=0;
          for(i=0;i<5;i++) 
          {
            pixelcolours[(i*2)+1] = 0;
            pixelcolours[i*2] = targetcolour;
          }
          
        }
        animationdelay = LONGDELAY;
      }
      break;
    case 3: //Flashing (quick)
      {
        if(animationstep==0)
        {
          animationstep=1;
          for(i=0;i<10;i++) 
          {
            pixelcolours[i] = 0;
          }
        }
        else
        {
          animationstep=0;
          for(i=0;i<10;i++) 
          {
            pixelcolours[i] = targetcolour;
          }
          
        }
        animationdelay = SHORTDELAY;
      }
      break;
    case 4: //Alternate flashing (quick)
      {
        if(animationstep==0)
        {
          animationstep=1;
          for(i=0;i<5;i++) 
          {
            pixelcolours[i*2] = 0;
            pixelcolours[(i*2)+1] = targetcolour;
          }
        }
        else
        {
          animationstep=0;
          for(i=0;i<5;i++) 
          {
            pixelcolours[(i*2)+1] = 0;
            pixelcolours[i*2] = targetcolour;
          }
        }
        animationdelay = SHORTDELAY;
      }
      break;
    case 5: //Random RGBW
      {
        for(i=0;i<10;i++) 
        {
          j = random(4);
          switch(j)
          {
            case 0:
              { 
                pixelcolours[i] = CLRRED;  
              }
              break;
            case 1:
              { 
                pixelcolours[i] = CLRGREEN;  
              }
              break;
            case 2:
              { 
                pixelcolours[i] = CLRBLUE;  
              }
              break;
            case 3:
              { 
                pixelcolours[i] = CLRWHITE;  
              }
              break;
          }
        }   
        animationdelay = SHORTDELAY;  
      }
      break;
    case 6: //Fade
      {
        for(i=0;i<10;i++) 
        {
          pixelcolours[i] = targetcolour;
        }
        if(animationstep<0)
        {
          if(animationbrightness<=1)
          {
            animationstep = 1;
          }
          else
          {
            animationbrightness--;
          }
        }
        else
        {
          if(animationbrightness>=5)
          {
            animationstep = -1;
          }
          else
          {
            animationbrightness++;
          }
        }
        animationdelay = LONGDELAY;
      } 
      break;
    case 7: //Fade (quick)
      {
        for(i=0;i<10;i++) 
        {
          pixelcolours[i] = targetcolour;
        }
        if(animationstep<0)
        {
          if(animationbrightness<=1)
          {
            animationstep = 1;
          }
          else
          {
            animationbrightness--;
          }
        }
        else
        {
          if(animationbrightness>=5)
          {
            animationstep = -1;
          }
          else
          {
            animationbrightness++;
          }
        }
        animationdelay = SHORTDELAY;
      } 
      break;
    case 8: //Random brightness (quick)
      {
        for(i=0;i<10;i++) 
        {
          pixelcolours[i] = targetcolour;
        }
        animationbrightness = random(5) + 1;
        animationdelay = SHORTDELAY;
      }
      break;
    case 9: //Rainbow wheel (quick)
      {
        // Loop through each pixel and set it to an incremental color wheel value.
        for(i=0; i<10; ++i) {
          pixelcolours[i] = CircuitPlayground.colorWheel(((i * 256 / 10) + animationstep) & 255);
        }
        animationdelay = SHORTDELAY;
        animationstep+=5;
        if(animationstep>255) { animationstep = 0; }
      }
      break;
    case 10: //Rainbow wheel all (quick)
      {
        animationcolour = CircuitPlayground.colorWheel(animationstep);
        for(i=0;i<10;i++) 
        {
          pixelcolours[i] = animationcolour;
        }
        animationstep+=5;
        if(animationstep>255) { animationstep = 0; }
        animationdelay = SHORTDELAY;
      }
      break;
    case 11: //Random colour
      {
        animationcolour = CircuitPlayground.colorWheel(random(256));
        for(i=0;i<10;i++) 
        {
          pixelcolours[i] = animationcolour;
        }
        animationdelay = HOLDDELAY;
      }
      break;
    case 12: //Psychadelic (quick)
      {
        for(i=0;i<10;i++) 
        {
          pixelcolours[i] = CircuitPlayground.colorWheel(random(256));
        }
        animationdelay = SHORTDELAY;
      }
      break;
    case 13: //Christmas chaser
      {
        unsigned int christmas[3] = { CLRRED, CLRGREEN, CLRWHITE };
        for(i=0;i<10;i++) 
        {
          pixelcolours[i] = christmas[(i+animationstep) % 3];  
        }     
        animationstep++;
        if(animationstep==3) { animationstep = 0; }
      }
      animationdelay = LONGDELAY;
      break;
    case 14: //Christmas all
      {
        switch(animationstep)
        {
          case 0:
            animationcolour=CLRRED;
            break;
          case 1:
            animationcolour=CLRGREEN;
            break;
          case 2:
            animationcolour=CLRWHITE;
            break;
        }
        animationstep++;
        if(animationstep==3) { animationstep = 0; }
        for(i=0;i<10;i++) 
        {
          pixelcolours[i] = animationcolour;
        }
        animationdelay = LONGDELAY;
      }
      break;
    case 15: //Super psychadelic (quick)
      {
        for(i=0;i<10;i++) 
        {
          pixelcolours[i] = CircuitPlayground.colorWheel(random(256));
        }
        animationbrightness = random(5) + 1;
        animationdelay = SHORTDELAY;
      }
      break;
    default:
      {
        animation=0;
        animationdelay = SHORTDELAY;
      }
      break;
  }
  if(animation>0)
  {
    CircuitPlayground.setBrightness(10*animationbrightness*animationbrightness);
  }
  else
  {
    CircuitPlayground.setBrightness(10*brightness*brightness);  
  }
  for(i=0;i<10;i++) 
  {
    CircuitPlayground.setPixelColor(i, pixelcolours[i]);  
  }
  lastanim = millis();

  if(ack>0) { playeffect(); }
}

void getreadytostore()
{
  readytostore = 1;
  lastready = millis();
}

void playeffect(void)
{
  switch(ack)
  {
    case 1:
      {
        j = random(3);
        switch(j)
        {
          case 0:
            CircuitPlayground.speaker.playSound(sndwurr1, sizeof(sndwurr1), 32000);
            break;
          case 1:
            CircuitPlayground.speaker.playSound(sndwurr2, sizeof(sndwurr2), 32000);
            break;
          case 2:
            CircuitPlayground.speaker.playSound(sndwurr3, sizeof(sndwurr3), 32000);
            break;
        }
      }
      break;
    case 2:
      {
        CircuitPlayground.speaker.playSound(sndmingen, sizeof(sndmingen), 32000);
      }
      break;
    case 3:
      {
        CircuitPlayground.speaker.playSound(sndbyebye, sizeof(sndbyebye), 32000);
      }
      break;
    case 4:
      {
        CircuitPlayground.speaker.playSound(sndhmm, sizeof(sndhmm), 32000);
      }
      break;
    case 5:
      {
        CircuitPlayground.speaker.playSound(sndok, sizeof(sndok), 32000);
      }
      break;
    case 6:
      {
        CircuitPlayground.speaker.playSound(sndoops, sizeof(sndoops), 32000);
      }
      break;
  }
  if(readytostore==1)
  {
    if(ack!=4)
    {
      readytostore = 0;
      Serial.println("Missed storing opportunity");
    }
  }
  ack = 0;
  CircuitPlayground.speaker.end();
}

void sensecolour(void)
{
  uint8_t red, green, blue;
  CircuitPlayground.clearPixels();
  delay(10);
  CircuitPlayground.senseColor(red, green, blue);
  
  // Figure out some basic hex code for visualization
  uint32_t sum = red;
  sum += green;
  sum += blue;
  //sum += clear; // clear contains RGB already so no need to re-add it
  
  float r, g, b;
  r = red; r /= sum;
  g = green; g /= sum;
  b = blue; b /= sum;
  r *= 256; g *= 256; b *= 256;
  
  Serial.print("Scanned: ");
  Serial.print((int)r, HEX); Serial.print((int)g, HEX); Serial.print((int)b, HEX);

  r = CircuitPlayground.gamma8(r);
  g = CircuitPlayground.gamma8(g);
  b = CircuitPlayground.gamma8(b);

  Serial.print(" | Corrected: ");
  Serial.print((int)r, HEX); Serial.print((int)g, HEX); Serial.print((int)b, HEX);
  Serial.println();

  targetcolour = r * 256 * 256 + g * 256 + b;
  
  //colorWipe(strip.Color(gammatable[(int)r], gammatable[(int)g], gammatable[(int)b]), 0);  
}

void writeflash(void)
{
  readytostore = 0;
  savedcolours.valid=true;
  savedcolours.flashcolour1=colour1;
  savedcolours.flashcolour2=colour2;
  savedcolours.flashcolour3=colour3;
  savedcolours.flashcolour4=colour4;
  savedcolours.flashanim3 = anim3;
  savedcolours.flashanim4 = anim4;
  savedcolours.flashbrightness3 = brightness3;
  savedcolours.flashbrightness4 = brightness4;
  flashstorage.write(savedcolours);
  Serial.println ("Saved to flash");
  Serial.print ("Colour 1: ");
  Serial.println (colour1,HEX);
  Serial.print ("Colour 2: ");
  Serial.println (colour2,HEX);
  Serial.print ("Colour 3: ");
  Serial.println (colour3,HEX);
  Serial.print ("Animation 3: ");
  Serial.println (anim3);
  Serial.print ("Brightness 3: ");
  Serial.println (brightness3);
  Serial.print ("Colour 4: ");
  Serial.println (colour4,HEX);
  Serial.print ("Animation 4: ");
  Serial.println (anim4);
  Serial.print ("Brightness 4: ");
  Serial.println (brightness4);
  playeffect();
}

void setup()
{
  CircuitPlayground.begin();
  Serial.begin(9600);

  if(CircuitPlayground.slideSwitch())
  {
    CircuitPlayground.redLED(HIGH);
    while(!Serial)
    {
      delay(1);
    }
  }
  
  /*if (tcs.begin())
  {
    Serial.println("Found sensor");
    tcs.setInterrupt(true);  // turn off LED
  }
  else
  {
    Serial.println("No TCS34725 found ... check your connections");
  }*/

  savedcolours = flashstorage.read();
  if(savedcolours.valid == false)
  {
    Serial.println("No stored settings - re-initialise");
    colour1 = CLRWHITE;
    colour2 = CLRNAVY;
    colour3 = CLRWHITE;
    colour4 = CLRWHITE;
    anim3 = 9;
    anim4 = 12;
    brightness3 = 2;
    brightness4 = 2;
  }
  else
  {
    Serial.println("Stored settings retrieved");
    colour1 = savedcolours.flashcolour1;
    colour2 = savedcolours.flashcolour2;
    colour3 = savedcolours.flashcolour3;
    colour4 = savedcolours.flashcolour4;
    anim3 = savedcolours.flashanim3;
    anim4 = savedcolours.flashanim4;
    brightness3 = savedcolours.flashbrightness3;
    brightness4 = savedcolours.flashbrightness4;
  }
  Serial.println ("Retreived from flash");
  Serial.print ("Colour 1: ");
  Serial.println (colour1,HEX);
  Serial.print ("Colour 2: ");
  Serial.println (colour2,HEX);
  Serial.print ("Colour 3: ");
  Serial.println (colour3,HEX);
  Serial.print ("Animation 3: ");
  Serial.println (anim3);
  Serial.print ("Brightness 3: ");
  Serial.println (brightness3);
  Serial.print ("Colour 4: ");
  Serial.println (colour4,HEX);
  Serial.print ("Animation 4: ");
  Serial.println (anim4);
  Serial.print ("Brightness 4: ");
  Serial.println (brightness4);
  
  CircuitPlayground.setAccelRange(LIS3DH_RANGE_2_G); 

  // 0 = turn off click detection & interrupt
  // 1 = single click only interrupt output
  // 2 = double click only interrupt output, detect single click
  // Adjust threshhold, higher numbers are less sensitive
  // Adjust second number for the sensitivity of the 'click' force
  // this strongly depend on the range! for 16G, try 5-10
  // for 8G, try 10-20. for 4G try 20-40. for 2G try 40-80
  CircuitPlayground.setAccelTap(2, 75);
  
  // have a procedure called when a tap is detected
  attachInterrupt(digitalPinToInterrupt(CPLAY_LIS3DH_INTERRUPT), isrdoubletapped, FALLING);

  ack = 2;
  turnon();
}

void loop() 
{
  //Doubletapped?
  if(doubletapped)
  {
    Serial.println("Double tap detected");
    if(brightness>0)
    {
      ack = 3;
      turnoff();
    }
    else
    {
        ack = 2;
        turnon();
    }
    doubletapped=0;
  }
  
  // Time to check for scan code?
  if(millis()>lastscan+SCANDELAY)
  {
    // Scan for capacitive touches
    scancode = 0;
    // A1
    delay(20);
    if (CircuitPlayground.readCap(6) > 600) { scancode+=1; }
    // A2
    delay(20);
    if (CircuitPlayground.readCap(9) > 600) { scancode+=2; }
    // A3
    delay(20);
    if (CircuitPlayground.readCap(10) > 600) { scancode+=4; }
    // A4
    delay(20);
    if (CircuitPlayground.readCap(2) > 600) { scancode+=32; }
    // A5
    delay(20);
    if (CircuitPlayground.readCap(3) > 600) { scancode+=64; }
    // A6
    delay(20);
    if (CircuitPlayground.readCap(0) > 600) { scancode+=8; }
    // A7
    delay(20);
    if (CircuitPlayground.readCap(1) > 400) { scancode+=16; }

    if(lastscancode==scancode)
    {
      lastscancode=0;
      if(scancode>0)
      {
        Serial.println("Scancode confirmed: " + String(scancode));
        if(brightness==0)
        {
          if(scancode==24)
          {
            ack = 2;
            turnon();  
          }
        }
        else
        {
          ack = 1;
          switch(scancode)
          {
            case 4:
              brightnessdown();
              break;
            case 2:
              brightnessup();
              break;
            /*case 24:
              ack = 3;
              turnoff();
              break;*/
            case 1:
              if(secret==0)
              {
                ack = 4;
                getreadytostore();
                animation = 0;
                sensecolour();
              }
              else
              {
                animation++;
              }
              animate();
              break;
            case 16:
              if(readytostore==1)
              {
                ack = 5;
                colour1 = targetcolour;
                writeflash();
              }
              else
              {
                targetcolour = colour1;
                animate();
              }
              break;     
            case 8:
              if(readytostore==1)
              {
                ack = 5;
                colour2 = targetcolour;
                writeflash();
              }
              else
              {
                targetcolour = colour2;
                animate();
              }
              break;
            case 32:
              if(readytostore==1)
              {
                ack = 5;
                colour3 = targetcolour;
                anim3 = animation;
                brightness3 = brightness;
                writeflash();
              }
              else
              {
                targetcolour = colour3;
                animation = anim3;
                brightness = brightness3;
                animate();
              }
              break;  
            case 64:
              if(readytostore==1)
              {
                ack = 5;
                colour4 = targetcolour;
                anim4 = animation;
                brightness4 = brightness;
                writeflash();
              }
              else
              {
                targetcolour = colour4;
                animation = anim4;
                brightness = brightness4;
                animate();
              }
              break;  
            /*case 6:
              if(secret==0)
              {
                secret = 1;
                animation = 7;
              }
              else
              {
                secret = 0;
              }
              CircuitPlayground.redLED(secret);
              animate();
              break;*/
            default:
              ack = 0;
          }
        }
      }
    }
    else
    {
      lastscancode=scancode;
    }
    lastscan=millis();
  }

  //Check about buttons (mainly re Christmas variations)
  if((millis()>lastbutton+SCANDELAY)&&(brightness>0))
  {
    if(CircuitPlayground.leftButton())
    {
      ack = 1;
      if(CircuitPlayground.rightButton())
      {
        animation = 12;
      }
      else
      {
        animation = 13;
      }
      lastbutton = millis();
      animate();
    }
    else if(CircuitPlayground.rightButton())
    {
      ack = 1;
      animation = 14;
      lastbutton = millis();
      animate();
    }
  }
  
  // Did we get any infrared signals?
  if(CircuitPlayground.irReceiver.getResults()) 
  {
    // Did we get any decodable messages?
    if(CircuitPlayground.irDecoder.decode()) 
    {
      // Did we get any NEC remote messages?
      if(CircuitPlayground.irDecoder.protocolNum == NEC) 
      {
        // We can print out the message if we want...
        CircuitPlayground.irDecoder.dumpResults(false);
        // What message did we get?
        if(brightness==0)
        {
          if(CircuitPlayground.irDecoder.value==IRON)
          {
            ack = 2;
            Serial.println("Turn on");
            turnon();           
          }
        }
        else
        {
          ack=1;  
          switch(CircuitPlayground.irDecoder.value) 
          {
            case IRBRIGHTNESSUP: 
              Serial.println("Increase brightness");
                brightnessup();
              break;
            case IRBRIGHTNESSDOWN: 
              Serial.println("Decrease brightness");
                brightnessdown();
              break;
            case IROFF: 
              ack = 3;
              Serial.println("Turn off");
              turnoff();
              break;
            case IRON: 
              ack = 2;
              Serial.println("Turn on");
              turnon();
              break;
            case IRFLASH: 
              Serial.println("Animation Flash");
                switch(animation)
                {
                  case 1:
                    animation++;
                    break;
                  case 2:
                    animation = 0;
                    break;
                  default:
                    animation = 1;
                    break;
                }
                animate();
              break;
            case IRSTROBE: 
              Serial.println("Animation Strobe");
                switch(animation)
                {
                  case 3:
                  case 4:
                    animation++;
                    break;
                  case 5:
                    animation = 0;
                    break;
                  default:
                    animation = 3;
                    break;
                }  
                animate();
              break;
            case IRFADE: 
              Serial.println("Animation Fade");
                switch(animation)
                {
                  case 6:
                  case 7:
                    animation++;
                    break;
                  case 8:
                    animation = 0;
                    break;
                  default:
                    animation = 6;
                    break;
                }   
                animate();
              break;
            case IRSMOOTH: 
              Serial.println("Animation Smooth");
                switch(animation)
                {
                  case 9:
                  case 10:
                    animation++;
                    break;
                  case 11:
                    animation = 0;
                    break;
                  default:
                    animation = 9;
                    break;
                }  
                animate();
              break; 
            case IRRED: 
              Serial.println("Red");
              targetcolour = CLRRED;
              animate();
              getreadytostore();
              break;
            case IRGREEN: 
              Serial.println("Green");
              targetcolour = CLRGREEN;
              animate();
              getreadytostore();
              break;
            case IRBLUE: 
              Serial.println("Blue");
              targetcolour = CLRBLUE;
              animate();
              getreadytostore();
              break;
            case IRWHITE: 
              Serial.println("White");
              targetcolour = CLRWHITE;
              animate();
              getreadytostore();
              break;
            case IRROSE: 
              Serial.println("Rose");
              targetcolour = CLRROSE;
              animate();
              getreadytostore();
              break;
            case IRFOREST: 
              Serial.println("Forest");
              targetcolour = CLRFOREST;
              animate();
              getreadytostore();
              break;
            case IRNAVY: 
              Serial.println("Navy");
              targetcolour = CLRNAVY;
              animate();
              getreadytostore();
              break;
            case IRORANGE: 
              Serial.println("Orange");
              targetcolour = CLRORANGE;
              animate();
              getreadytostore();
              break;
            case IRTEAL: 
              Serial.println("Teal");
              targetcolour = CLRTEAL;
              animate();
              getreadytostore();
              break;
            case IRPURPLE: 
              Serial.println("Purple");
              targetcolour = CLRPURPLE;
              animate();
              getreadytostore();
              break;
            case IRMUSTARD: 
              Serial.println("Mustard");
              targetcolour = CLRMUSTARD;
              animate();
              getreadytostore();
              break;
            case IRLAKE: 
              Serial.println("Lake");
              targetcolour = CLRLAKE;
              animate();
              getreadytostore();
              break;
            case IRMAUVE: 
              Serial.println("Mauve");
              targetcolour = CLRMAUVE;
              animate();
              getreadytostore();
              break;
            case IRYELLOW: 
              Serial.println("Yellow");
              targetcolour = CLRYELLOW;
              animate();
              getreadytostore();
              break;
            case IROCEAN: 
              Serial.println("Ocean");
              targetcolour = CLROCEAN;
              animate();
              getreadytostore();
              break;
            case IRPINK: 
              Serial.println("Pink");
              targetcolour = CLRPINK;
              animate();
              getreadytostore();
              break;
            default:
              ack=0;
              break;
          } 
        }
      }
    }
  }
  CircuitPlayground.irReceiver.enableIRIn(); // Start the receiver
  if(readytostore) { if(millis()>lastready+SAVEDELAY) { readytostore = 0;  Serial.println("Missed storing opportunity"); } }
  if(brightness) { if(millis()>lastanim+animationdelay) { animate(); } }
}
