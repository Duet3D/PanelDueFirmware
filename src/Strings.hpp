/*
 * Strings.hpp
 *
 *  Created on: 27 Feb 2017
 *      Author: David
 *
 * The encoding used for this file must be UTF-8 to ensure that accented characters are displayed correctly.
 */

#ifndef SRC_STRINGS_HPP_
#define SRC_STRINGS_HPP_

#include "ecv.h"
#include "UserInterfaceConstants.hpp"

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
	CSTRING set;

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

	// Misc
	CSTRING confirmFactoryReset;
	CSTRING confirmRestart;
	CSTRING confirmFileDelete;
	CSTRING areYouSure;
	CSTRING touchTheSpot;
	CSTRING settingsNotSavedText;
	CSTRING restartNeededText;
	CSTRING restartRequired;
	CSTRING restartNow;
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
	CSTRING simulate;

	// Printer status strings
	CSTRING statusValues[12];

	// Colour theme names
	CSTRING colourSchemeNames[NumColourSchemes];

	// display dimmer types
	CSTRING displayDimmingNames[DISPLAYDIMMER_MAX+1];
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
		"Set",

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

		// Misc
		"Confirm factory reset",
		"Confirm restart",
		"Confirm file delete",
		"Are you sure?",
		"Touch the spot",
		"Some settings are not saved!",
		"Touch Save & Restart to use new settings",
		"Restart required",
		"Restart now?",
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
		"Simulate",

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
			"Changing tool",
			"Simulating"
		},

		// Theme names
		{
			"Light",
			"Dark"
		},

		// display dimming types
		{
			"Always Dim",
			"Never Dim",
			"Idle Dim"
		}
	},

	// German
	{
		// Language name
		"Deutsch",

		// Main page strings
		"Steuerung",
		"Druck",
		"Konsole",
		"Einrichtung",
		"Aktuell" THIN_SPACE DEGREE_SYMBOL "C",
		"Aktiv" THIN_SPACE DEGREE_SYMBOL "C",
		"Standby" THIN_SPACE DEGREE_SYMBOL "C",
		"Bewegung",
		"Extrusion",
		"Makro",
		"NOT-AUS",

		// Print page
		"Extruder" THIN_SPACE "%",
		"Speed ",							// note space at end. Was "Geschwindigkeit " but that is too long to fit in the space available.
		"LÃ¼fter ",							// note space at end
		"Zeit Ã¼brig: ",
		"Datei ",							// note space at end
		", Filament ",						// note space at end
		", Schicht ",						// note space at end
		"n/v",
		"Pause",
		"Baby Step",
		"Fortsetzen",
		"Abbrechen",
		"Set",

		// Setup page
		"LautstÃ¤rke ",						// note space at end
		"Touch kalibrieren",
		"Anzeige spiegeln",
		"Anzeige invertieren",
		"Thema",
		"Helligkeit -",
		"Helligkeit +",
		"Einstellungen speichern",
		"Einstellungen lÃ¶schen",
		"Speichern & Neustarten",

		// Misc
		"RÃ¼cksetzen bestÃ¤tigen",
		"Neustart bestÃ¤tigen",
		"LÃ¶schen bestÃ¤tigen",
		"Sind sie sicher?",
		"BerÃ¼hren sie den Punkt",
		"Einige Einstellungen sind nicht gespeichert!",
		"BerÃ¼hren sie Speichern & Neustarten um die neuen Einstellungen anzuwenden",
		"Neustarten erforderlich",
		"Neustarten jetzt?",
		"Kopf bewegen",
		"Extrusionsmenge (mm)",
		"Geschwindigkeit (mm/s)",
		"Extrudieren",
		"ZurÃ¼ckziehen",
		"Babystepping",
		"Aktueller Z-Versatz: ",
		"Nachricht",
		"Nachrichten",
		"Panel Due Firmwareversion ",	// note space at end

		// File popup
		"Dateien auf Karte ",			// note the space on the end
		"Makros",
		"Fehler ",						// note the space at the end
		" beim Zugriff auf SD-Karte",	// note the space at the start
		"Dateiname: ",
		"GrÃ¶ÃŸe: ",
		"SchichthÃ¶he: ",
		"ObjekthÃ¶he: ",
		"BenÃ¶tigtes Filament: ",
		"Erzeugt mit: ",
		"Simulieren",

		// Printer status strings
		{
			"Verbinde",
			"Leerlauf",
			"Drucken",
			"Angehalten",
			"Starte",
			"Pausiert",
			"BeschÃ¤ftigt",
			"Pausiere",
			"Fortsetzen",
			"Firmware-Upload",
			"Wechsle Tool",
			"Simuliert"
		},

		// Theme names
		{
			"Hell",
			"Dunkel"
		},

		// display dimming types
		{
			"Immer Dim",
			"Nie dämmern",
			"Idle Dim"
		}


	},

	// French
	{
		// Language name
		"FranÃ§ais",

		// Main page strings
		"ContrÃ´le",
		"Imprimer",
		"Console",
		"Installation",
		"Actuel" THIN_SPACE DEGREE_SYMBOL "C",
		"Actif" THIN_SPACE DEGREE_SYMBOL "C",
		"Standby" THIN_SPACE DEGREE_SYMBOL "C",
		"Mouvement",
		"Extrusion",
		"Macro",
		"ARRÃŠT",

		// Print page
		"Extrudeuse" THIN_SPACE "%",
		"Vitesse ",								// note space at end
		"Ventilateur ",							// note space at end
		"Temps Restant: ",
		"Fichier ",								// note space at end
		", filament ",							// note space at end
		", couche ",							// note space at end
		"n/a",
		"Pause",
		"Baby step",
		"Reprise",
		"Annuler",
		"Set",

		// Setup page
		"Volume ",								// note space at end
		"Calibrer touch",
		"Affichage en nÃ©gatif",
		"Inverser affichage",
		"ThÃ©me",
		"LuminositÃ© -",
		"LuminositÃ© +",
		"Sauver paramÃªtres",
		"Effacer paramÃªtres",
		"Sauvegarde & RedÃ©marrage",

		// Misc
		"Confirmer le rÃ©initialisation de l'imprimante",
		"Confirm RedÃ©marrage",
		"Confirm suppression fichier",
		"Vous Ãªtes sÃ»re?",
		"Appuyer sur le point",
		"Certains rÃ©glages ne sont pas sauvegardÃ©s!",
		"Appuyer sur Sauvegarde et RedÃ©marrage pour utiliser les nouveaux rÃ©glages",
		"Restart required",
		"Restart now?",
		"Mouvement de la  tÃªte",
		"QuantitÃ© de MatiÃ©re extrudÃ©e (mm)",
		"Vitesse (mm/s)",
		"Extruder",
		"Retracter",
		"Baby stepping",
		"dÃ©calage Z courant : ",
		"Message",
		"Messages",
		"Version du firmware du Panel Due ",	// note space at end

		// File popup
		"Fichier sur carte ",					// note the space on the end
		"Macros",
		"Erreur ",								// note the space at the end
		" accÃ©s SD card en cours",				// note the space at the start
		"Nom du fichier : ",
		"Taille : ",
		"Hauteur de couche: ",
		"Hauteur de l'objet: ",
		"Filament requis: ",
		"Sliced par: ",
		"Simuler",

		// Printer status strings
		{
			"Connection en cours",
			"Au repos",
			"Impression",
			"ArrÃªt",
			"DÃ©marrage",
			"Pause",
			"OccupÃ©"
			"Pause",
			"Reprise",
			"TÃ©leverser le firmware",
			"Changement d'outil",
			"Simuler"
		},

		// Theme names
		{
			"Fond Blanc",
			"Fond Noir"
		},

		// display dimming types
		{
			"Toujours Dim",
			"Jamais Dim",
			"Idle Dim"
		}

	},
	// Czech
		{
			// Language name
			"ÄŒeÅ¡tina",

			// Main page strings
			"OvlÃ¡dÃ¡nÃ­",
			"Tisk",
			"Konzole",
			"NastavenÃ­",
			"AktuÃ¡lnÃ­" THIN_SPACE DEGREE_SYMBOL "C",
			"AktivnÃ­" THIN_SPACE DEGREE_SYMBOL "C",
			"NeÄ�innÃ¡" THIN_SPACE DEGREE_SYMBOL "C",
			"Pohyb",
			"Extruder",
			"Makra",
			"STOP",

			// Print page
			"Extruder" THIN_SPACE "%",
			"Rychl. ",							// note space at end
			"Vent. ",							// note space at end
			"ÄŒas do konce: ",
			"soubor ",							// note space at end
			", materiÃ¡l ",						// note space at end
			", vrstva ",							// note space at end
			"n/a",
			"Pozastavit",
			"Baby step",
			"PokraÄ�ovat",
			"ZruÅ¡it",
			"OK",

			// Setup page
			"Hlasitost ",							// note space at end
			"Kalibrace dotyku",
			"Zrcadlit displej",
			"ObrÃ¡tit displej",
			"Motiv",
			"PodsvÃ­cenÃ­ -",
			"PodsvÃ­cenÃ­ +",
			"UloÅ¾it nastavenÃ­",
			"Smazat nastavenÃ­",
			"UloÅ¾it a Restart",

			// Misc
			"SkuteÄ�nÄ› obnovit tovÃ¡rnÃ­ nastavenÃ­?",
			"Restartovat?",
			"SkuteÄ�nÄ› smazat?",
			"UrÄ�itÄ›?",
			"DotknÄ›te se bodu",
			"NÄ›kterÃ¡ nastavenÃ­ nejsou uloÅ¾ena!",
			"Zvolte UloÅ¾it a Restart pro dokonÄ�enÃ­",
			"VyÅ¾adovÃ¡n restart",
			"Restartovat nynÃ­?",
			"Posun hlavy",
			"MnoÅ¾stvÃ­ (mm)",
			"Rychlost (mm/s)",
			"VytlaÄ�it (extr.)",
			"ZatlaÄ�it (retr.)",
			"Baby stepping",
			"AktuÃ¡lnÃ­ Z offset: ",
			"ZprÃ¡va",
			"ZprÃ¡vy",
			"Verze firmware Panel Due ",	// note space at end
			// File popup
			"Soubory na kartÄ› ",				// note the space on the end
			"Makra",
			"Chyba ",						// note the space at the end
			" pÅ™Ã­stupu ke kartÄ›",			// note the space at the start
			"NÃ¡zev: ",
			"Velikost: ",
			"VÃ½Å¡ka vrstvy: ",
			"VÃ½Å¡ka objektu: ",
			"SpotÅ™eba (mat.): ",
			"Slicer: ",
			"Simulace",

			// Printer status strings
			{
				"PÅ™ipojovÃ¡nÃ­",
				"NeÄ�innÃ½",
				"Tiskne",
				"Zastaven",
				"Startuje",
				"Pozastaven",
				"ZaneprÃ¡zdnÄ›nÃ½",
				"Pozastavuje se",
				"PokraÄ�uje",
				"NahrÃ¡vÃ¡ firmware",
				"VÃ½mÄ›na nÃ¡stroje",
				"Simulace"
			},

			// Theme names
			{
				"SvÄ›tlÃ½",
				"TmavÃ½"
			},
			// display dimming types
			{
				"Vždy dim",
				"Nikdy nezměníme",
				"Idle Dim"
			}

		},

#if 0	// Spanish not supported yet
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
		"Simulate",

		// Printer status strings
		{
			"conexiÃ³n",
			"ocioso",
			"imprimiendo",
			"detuvo",
			"empezando",
			"pausado",
			"ocupado",
			"pausando",
			"reanudando",
			"carga del firmware",
			"herramienta de cambio",
			"simulating"
		},

		// Theme names
		{
			"Light",
			"Dark"
		}
	},
#endif
};

#endif /* SRC_STRINGS_HPP_ */
