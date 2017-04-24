/*
 * Copyright (c) 2017 Evan Kale
 * Email: EvanKale91@gmail.com
 * Web: www.youtube.com/EvanKale
 * Social: @EvanKale91
 *
 * This file is part of ArduinoGlowingLEDs.
 *
 * ArduinoGlowingLEDs is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include "Colors.h"

//Pin where potentiometer for hue offset is connected
#define HUE_POT_PIN A3

//Tweak these max color values for white balance (0-255)
#define RED_MAX 255
#define GREEN_MAX 255
#define BLUE_MAX 255

void setup()
{
	pinMode(9, OUTPUT);
	pinMode(10, OUTPUT);
	pinMode(11, OUTPUT);

  // Set up PWM on pins 9, 10, 11
	// For more info on PWM registers, see Atmega328 datasheet,
	// Section 16.11 (TCCR1) and 18.11 (TCCR2)

	// Timer/Counter1 Control Register A
	// - Set WGM to phase correct PWM, with 0xFF TOP (Set WGM13:0 to 0001 (note: WGM13:2 is in TCCR1B))
	// - Set OC1A to PWM (Set COM1A1:0 to 10)
	// - Set OC1B to PWM (Set COM1B1:0 to 10)
	TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM10);

	// Timer/Counter1 Control Register B
	// - Set clock source to CLK/64 (Set CS12:0 to 011)
	TCCR1B = _BV(CS11) | _BV(CS10);

	// Timer/Counter2 Control Register A
	// - Set WGM to phase correct PWM, with 0xFF TOP (Set WGM22:0 to 001 (note: WGM22 is in TCCR2B))
	// - Set OC2A to PWM (Set COM2A1:0 to 10)
	TCCR2A = _BV(COM2A1) | _BV(WGM20);

	// Timer/Counter2 Control Register B
	// - Set clock source to CLK/64 (Set CS22:0 to 100)
	TCCR2B = _BV(CS22);

	OCR1A = 0; //(pin 9, green)
	OCR1B = 0; //(pin 10, red)
	OCR2A = 0; //(pin 11, blue)
}

void loop()
{
	uint8_t rgb[3];
	uint8_t h, s, l;
	float hf, sf, lf;

	for (int i = 0; i < sizeof(colors) / 3; ++i)
	{
    //Read the the next color from progmem
		h = pgm_read_byte_near(colors + (i * 3));
		s = pgm_read_byte_near(colors + (i * 3) + 1);
		l = pgm_read_byte_near(colors + (i * 3) + 2);

		hf = (float)h / 255;
		sf = (float)s / 255;
		lf = (float)l / 255;

    //Convert HSL color to RGB color
		hslToRgb(hf, sf, lf, rgb);

		OCR1A = (rgb[1]); //(pin 9, green)
		OCR1B = (rgb[0]); //(pin 10, red)
		OCR2A = (rgb[2]); //(pin 11, blue)

		//Delay 33 ms for roughly 30FPS
		delay(33);
	}
}

void hslToRgb(float h, float s, float l, uint8_t * rgbOut)
{
  //Read hue offset from potentiometer
	int huePotValue = analogRead(HUE_POT_PIN);
	float huePotPercent = (float)huePotValue / 1023;

  //Offset hue value
	h += huePotPercent;
	h -= (int)h;

  //Convert HSL to RGB
	float r, g, b;

	if (s == 0.0f)
	{
		r = g = b = l; // achromatic
	}
	else
	{
		float q = (l < 0.5f) ? (l * (1 + s)) : (l + s - l * s);
		float p = 2 * l - q;
		r = hueToRgb(p, q, h + 1.0f / 3.0f);
		g = hueToRgb(p, q, h);
		b = hueToRgb(p, q, h - 1.0f / 3.0f);
	}

  //Apply exponential curve to RGB values for better LED response
	r = r * r * r;
	g = g * g * g;
	b = b * b * b;

  //Apply white balance scales
	rgbOut[0] = (int)(r * RED_MAX);
	rgbOut[1] = (int)(g * GREEN_MAX);
	rgbOut[2] = (int)(b * BLUE_MAX);
}

float hueToRgb(float p, float q, float t)
{
	if (t < 0.0f)
		t += 1.0f;
	if (t > 1.0f)
		t -= 1.0f;
	if (t < 1.0f / 6.0f)
		return p + (q - p) * 6.0f * t;
	if (t < 1.0f / 2.0f)
		return q;
	if (t < 2.0f / 3.0f)
		return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
	return p;
}
