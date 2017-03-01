/*
 * Strings.hpp
 *
 *  Created on: 27 Feb 2017
 *      Author: David
 */

#ifndef SRC_STRINGS_HPP_
#define SRC_STRINGS_HPP_

#include "ecv.h"

#define CSTRING const char * const array
#define Newline			"\n"
#define DegreeSymbol	"\u00B0"

struct StringTable
{
	// Language name
	CSTRING languageName;

	// Main page strings
	CSTRING control;
	CSTRING print;
	CSTRING console;
	CSTRING setup;
	CSTRING current;
	CSTRING active;
	CSTRING standby;
	CSTRING move;
	CSTRING extrusion;
	CSTRING macro;
	CSTRING stop;

	// Print page
	CSTRING extruderPercent;
	CSTRING speed;
	CSTRING fan;
	CSTRING timeRemaining;
	CSTRING file;
	CSTRING filament;
	CSTRING layer;
	CSTRING notAvailable;
	CSTRING pause;
	CSTRING babystep;
	CSTRING resume;
	CSTRING cancel;

	// Setup page
	CSTRING volume;
	CSTRING calibrateTouch;
	CSTRING mirrorDisplay;
	CSTRING invertDisplay;
	CSTRING theme;
	CSTRING brightnessDown;
	CSTRING brightnessUp;
	CSTRING saveSettings;
	CSTRING clearSettings;
	CSTRING saveAndRestart;

	// Theme names
	CSTRING lightTheme;
	CSTRING darkTheme;

	// Misc
	CSTRING confirmFactoryReset;
	CSTRING confirmRestart;
	CSTRING confirmFileDelete;
	CSTRING areYouSure;
	CSTRING touchTheSpot;
	CSTRING settingsNotSavedText;
	CSTRING restartNeededText;
	CSTRING moveHead;
	CSTRING extrusionAmount;
	CSTRING extrusionSpeed;
	CSTRING extrude;
	CSTRING retract;
	CSTRING babyStepping;
	CSTRING currentZoffset;
	CSTRING message;
	CSTRING messages;
	CSTRING firmwareVersion;

	// File popup
	CSTRING filesOnCard;
	CSTRING macros;
	CSTRING error;
	CSTRING accessingSdCard;
	CSTRING fileName;
	CSTRING fileSize;
	CSTRING layerHeight;
	CSTRING objectHeight;
	CSTRING filamentNeeded;
	CSTRING generatedBy;

	// Printer status strings
	CSTRING statusValues[11];
};

const StringTable LanguageTables[4] =
{
	// English
	{
		// Language name
		"English",

		// Main page strings
		"Control",
		"Print",
		"Console",
		"Setup",
		"Current" THIN_SPACE DEGREE_SYMBOL "C",
		"Active" THIN_SPACE DEGREE_SYMBOL "C",
		"Standby" THIN_SPACE DEGREE_SYMBOL "C",
		"Move",
		"Extrusion",
		"Macro",
		"STOP",

		// Print page
		"Extruder" THIN_SPACE "%",
		"Speed ",							// note space at end
		"Fan ",								// note space at end
		"Time left: ",
		"file ",							// note space at end
		", filament ",						// note space at end
		", layer ",							// note space at end
		"n/a",
		"Pause",
		"Baby step",
		"Resume",
		"Cancel",

		// Setup page
		"Volume ",							// note space at end
		"Calibrate touch",
		"Mirror display",
		"Invert display",
		"Theme",
		"Brightness -",
		"Brightness +",
		"Save settings",
		"Clear settings",
		"Save & Restart",

		// Theme names
		"Light",
		"Dark",

		// Misc
		"Confirm factory reset",
		"Confirm restart",
		"Confirm file delete",
		"Are you sure?",
		"Touch the spot",
		"Some settings are not saved!",
		"Touch Save & Restart to use new settings",
		"Move head",
		"Extrusion amount (mm)",
		"Speed (mm/s)",
		"Extrude",
		"Retract",
		"Baby stepping",
		"Current Z offset: ",
		"Message",
		"Messages",
		"Panel Due firmware version ",	// note space at end

		// File popup
		"Files on card ",				// note the space on the end
		"Macros",
		"Error ",						// note the space at the end
		" accessing SD card",			// note the space at the start
		"Filename: ",
		"Size: ",
		"Layer height: ",
		"Object height: ",
		"Filament needed: ",
		"Sliced by: ",

		// Printer status strings
		{
			"Connecting",
			"Idle",
			"Printing",
			"Halted",
			"Starting up",
			"Paused",
			"Busy",
			"Pausing",
			"Resuming",
			"Firmware upload",
			"Changing tool"
		}
	},

	// Spanish
	{
		// Language name
		"Espanol",

		// Main page strings
		"Control",
		"Print",
		"Console",
		"Setup",
		"Current" THIN_SPACE DEGREE_SYMBOL "C",
		"Active" THIN_SPACE DEGREE_SYMBOL "C",
		"Standby" THIN_SPACE DEGREE_SYMBOL "C",
		"Move",
		"Extrusion",
		"Macro",
		"STOP",

		// Print page
		"Extruder" THIN_SPACE "%",
		"Speed ",							// note space at end
		"Fan ",								// note space at end
		"Time left: ",
		"file ",							// note space at end
		", filament ",						// note space at end
		", layer ",							// note space at end
		"n/a",
		"Pause",
		"Baby step",
		"Resume",
		"Cancel",

		// Setup page
		"Volume ",							// note space at end
		"Calibrate touch",
		"Mirror display",
		"Invert display",
		"Theme",
		"Brightness -",
		"Brightness +",
		"Save settings",
		"Clear settings",
		"Save & Restart",

		// Theme names
		"Light",
		"Dark",

		// Misc
		"Confirm factory reset",
		"Confirm restart",
		"Confirm file delete",
		"Are you sure?",
		"Touch the spot",
		"Some settings are not saved!",
		"Touch Save & Restart to use new settings",
		"Move head",
		"Extrusion amount (mm)",
		"Speed (mm/s)",
		"Extrude",
		"Retract",
		"Baby stepping",
		"Current Z offset: ",
		"Message",
		"Messages",
		"Panel Due firmware version ",	// note space at end

		// File popup
		"Files on card ",				// note the space on the end
		"Macros",
		"Error ",						// note the space at the end
		" accessing SD card",			// note the space at the start
		"Filename: ",
		"Size: ",
		"Layer height: ",
		"Object height: ",
		"Filament needed: ",
		"Sliced by: ",

		// Printer status strings
		{
			"conexión",
			"ocioso",
			"imprimiendo",
			"detuvo",
			"empezando",
			"pausado",
			"ocupado",
			"pausando",
			"reanudando",
			"carga del firmware",
			"herramienta de cambio"
		}
	},

	// German
	{
		// Language name
		"Deutsch",

		// Main page strings
		"Control",
		"Print",
		"Console",
		"Setup",
		"Current" THIN_SPACE DEGREE_SYMBOL "C",
		"Active" THIN_SPACE DEGREE_SYMBOL "C",
		"Standby" THIN_SPACE DEGREE_SYMBOL "C",
		"Move",
		"Extrusion",
		"Macro",
		"HALT",

		// Print page
		"Extruder" THIN_SPACE "%",
		"Speed ",							// note space at end
		"Fan ",								// note space at end
		"Time left: ",
		"file ",							// note space at end
		", filament ",						// note space at end
		", layer ",							// note space at end
		"n/a",
		"Pause",
		"Baby step",
		"Resume",
		"Cancel",

		// Setup page
		"Lautstärke ",						// note space at end
		"Calibrate touch",
		"Mirror display",
		"Invert display",
		"Theme",
		"Helligkeit -",
		"Helligkeit +",
		"Save settings",
		"Clear settings",
		"Save & Restart",

		// Theme names
		"Light",
		"Dark",

		// Misc
		"Confirm factory reset",
		"Confirm restart",
		"Confirm file delete",
		"Are you sure?",
		"Berühren sie den punkt",
		"Some settings are not saved!",
		"Touch Save & Restart to use new settings",
		"Move head",
		"Extrusion amount (mm)",
		"Speed (mm/s)",
		"Extrude",
		"Retract",
		"Baby stepping",
		"Current Z offset: ",
		"Message",
		"Messages",
		"Panel Due firmware-version ",	// note space at end

		// File popup
		"Files on card ",				// note the space on the end
		"Macros",
		"Error ",						// note the space at the end
		" accessing SD card",			// note the space at the start
		"Filename: ",
		"Size: ",
		"Layer height: ",
		"Object height: ",
		"Filament needed: ",
		"Sliced by: ",

#if 0
		// Main page strings
		"bettsensor",
		"druck fortsetzen",
		"druck pausieren",
		"druck abbrechen",
		"lüftergeschwindigkeit (%)",
		"druckgeschwindigkeit (%)",
		"extrusion (%)",
		"fertigstellung (%)",
		"druckstatus",
		"aktuell",
		"gesetzt",

		// Setup page strings
		"sprache",
		"firmware-version anzeigen",
		"werkseinstellungen" newline "wiederherstellen",
		"rücksetzen auf werkseinstellungen bestätigen",
		"ja",
		"nein",
#endif

		// Printer status strings
		{
			"Verbinde",
			"Leerlauf",
			"Drucken",
			"Angehalten",
			"Starte",
			"Pausiert",
			"Beschäftigt",
			"Pausiere",
			"Fortsetzen",
			"Firmware-Upload",
			"Wechsle Tool"
		}
	},

	// French
	{
		// Language name
		"Français",

		// Main page strings
		"Control",
		"Print",
		"Console",
		"Setup",
		"Actuel" THIN_SPACE DEGREE_SYMBOL "C",
		"Active" THIN_SPACE DEGREE_SYMBOL "C",
		"Standby" THIN_SPACE DEGREE_SYMBOL "C",
		"Move",
		"Extrusion",
		"Macro",
		"ARRÊT",

		// Print page
		"Extruder" THIN_SPACE "%",
		"Vitesse ",							// note space at end
		"Fan ",								// note space at end
		"Time left: ",
		"file ",							// note space at end
		", filament ",						// note space at end
		", layer ",							// note space at end
		"n/a",
		"Pause",
		"Baby step",
		"Reprise",
		"Annuler",

		// Setup page
		"Volume ",							// note space at end
		"Calibrate touch",
		"Mirror display",
		"Invert display",
		"Theme",
		"Luminosité -",
		"Luminosité +",
		"Save settings",
		"Clear settings",
		"Save & Restart",

		// Theme names
		"Light",
		"Dark",

		// Misc
		"Confirmer le réinitialisation de l'usine",
		"Confirm restart",
		"Confirm file delete",
		"Are you sure?",
		"Toucher le place",
		"Some settings are not saved!",
		"Touch Save & Restart to use new settings",
		"Move head",
		"Extrusion amount (mm)",
		"Vitesse (mm/s)",
		"Extrude",
		"Retract",
		"Baby stepping",
		"Current Z offset: ",
		"Message",
		"Messages",
		"Panel Due firmware version ",	// note space at end

		// File popup
		"Files on card ",				// note the space on the end
		"Macros",
		"Error ",						// note the space at the end
		" accessing SD card",			// note the space at the start
		"Filename: ",
		"Size: ",
		"Layer height: ",
		"Object height: ",
		"Filament needed: ",
		"Sliced by: ",

		// Printer status strings
		{
			"De liason",
			"Au repos",
			"Impression",
			"Halte",
			"Démerrage",
			"Pause",
			"Occupé"
			"Pause",
			"Reprise",
			"Envoyer le firmware",
			"Outil de changement"
		}
	}
};

#endif /* SRC_STRINGS_HPP_ */
