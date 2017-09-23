// Open Pixel Control "video" example -- loops an animation
// (MOV, MP4, etc.) for display on an LED matrix (16x16 as
// written here, but easily changed).  Seems to work OK in
// Processing 2.2.1, other versions may put up a struggle.

import processing.video.*;

OPC    opc         = new OPC(this, 25, "bmg.opc");
int    arrayWidth  = 21, // Width of LED matrix
       arrayHeight = 9, // Height of LED matrix
       scale       = 26, // Preview window pixel size
       numPixels   = 140; // Total # of pixels
PImage img         = new PImage(arrayWidth, arrayHeight, RGB);
Movie  movie;

void setup() {
  size(273, 180, JAVA2D);
  opc.setPixel(numPixels-1, 0); // Alloc pixel array ASAP
  this.registerMethod("dispose", this);
  noSmooth();
  frameRate(29.97);

  // Set up OPC pixel grid.  Arguments are: 1st pixel index,
  // row length, # of rows, center x, y, horizontal & vertical
  // pixel spacing, angle (radians), 'zigzag' flag (true/false):
  //opc.ledGrid(0, arrayWidth, arrayHeight, (width - 1) / 2,
    //(height - 1) / 2, scale, scale, 0, true);
  
  /*for (int x = 0; x < numPixels; x++) {
    opc.led(x, 0, 0);
  }*/
  
  opc.ledStrip(2, 11, 136, 9, 26, 0, true);
  opc.ledStrip(17, 10, 136, 29, 26, 0, false);
  opc.ledStrip(33, 11, 136, 49, 26, 0, true);
  opc.ledStrip(49, 10, 136, 69, 26, 0, false);
  opc.ledStrip(65, 11, 136, 89, 26, 0, true);
  opc.ledStrip(81, 10, 136, 109, 26, 0, false);
  opc.ledStrip(97, 11, 136, 129, 26, 0, true);
  opc.ledStrip(113, 10, 136, 149, 26, 0, false);
  opc.ledStrip(129, 11, 136, 169, 26, 0, true);
  
  /*opc.led(0, 0, 0);
  opc.led(1, 0, 0);
  for (int x = 13; x < 17; x++) {
    opc.led(x, 0, 0);
  }
  for (int x = 27; x < 33; x++) {
    opc.led(x, 0, 0);
  }
  for (int x = 44; x < 49; x++) {
    opc.led(x, 0, 0);
  }
  for (int x = 59; x < 65; x++) {
    opc.led(x, 0, 0);
  }
  for (int x = 76; x < 81; x++) {
    opc.led(x, 0, 0);
  }
  for (int x = 91; x < 97; x++) {
    opc.led(x, 0, 0);
  }
  for (int x = 108; x < 113; x++) {
    opc.led(x, 0, 0);
  }
  for (int x = 123; x < 129; x++) {
    opc.led(x, 0, 0);
  }*/
  
  
  selectInput("Select a file to process:", "fileSelected");
}

void draw() {
  image(img, 0, 0, width, height);
}

void fileSelected(File selection) {
  if(selection != null) {
    movie = new Movie(this, selection.getAbsolutePath());
    movie.play();
  } else {
    println("Cancelled");
    exit();
  }
}

void movieEvent(Movie m) {
  m.read();
  opc.enable();       // File write enabled only when movie's playing
  PImage t = m.get(); // Movie frame to temporary PImage
  // First, scale image to a smaller size that maintains
  // aspect ratio, while minor axis fills LED array.
  float  xScale = (float)t.width  / (float)arrayWidth,
         yScale = (float)t.height / (float)arrayHeight;
  if(xScale >= yScale) t.resize(0, arrayHeight);
  else                 t.resize(arrayWidth, 0);
  // Then clip out the center section (whatever fits the LED array)
  // into a new image.  Using a temporary interim image (above) and
  // then copying to global 'img' ensures the scaled-down image
  // is used in draw().  Else a race condition occurs where an
  // unscaled instance would occasionally make it through and cause
  // a visual glitch.
  img.copy(t, (t.width-arrayWidth)/2, (t.height-arrayHeight)/2,
    arrayWidth, arrayHeight, 0, 0, arrayWidth, arrayHeight);
  // Other reason it's done in two steps (instead of using
  // resize capability of img.copy()) is that resize() provides
  // a higher-quality scaling function.
}

// Issue LEDs-off packet when certain exit conditions are caught
void dispose() {
  for(int i=0; i < numPixels; i++) opc.setPixel(i, 0);
  opc.writePixels();
}