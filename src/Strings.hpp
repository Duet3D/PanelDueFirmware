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
	CSTRING statusValues[11];

	// Colour theme names
	CSTRING colourSchemeNames[NumColourSchemes];
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
			"Changing tool"
		},

		// Theme names
		{
			"Light",
			"Dark"
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
		"Geschwindigkeit ",					// note space at end
		"Lüfter ",							// note space at end
		"Zeit übrig: ",
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
		"Lautstärke ",						// note space at end
		"Touch kalibrieren",
		"Anzeige spiegeln",
		"Farben invertieren",
		"Thema",
		"Helligkeit -",
		"Helligkeit +",
		"Einstellungen speichern",
		"Einstellungen löschen",
		"Speichern & Neustarten",

		// Misc
		"Rücksetzen bestätigen",
		"Neustart bestätigen",
		"Löschen bestätigen",
		"Sind sie sicher?",
		"Berühren sie den Punkt",
		"Einige Einstellungen sind nicht gespeichert!",
		"Berühren sie Speichern & Neustarten um die neuen Einstellungen anzuwenden",
		"Neustarten erforderlich",
		"Neustarten jetzt?",
		"Kopf bewegen",
		"Extrusionsmenge (mm)",
		"Geschwindigkeit (mm/s)",
		"Extrudieren",
		"Zurückziehen",
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
		"Größe: ",
		"Schichthöhe: ",
		"Objekthöhe: ",
		"Benötigtes Filament: ",
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
			"Beschäftigt",
			"Pausiere",
			"Fortsetzen",
			"Firmware-Upload",
			"Wechsle Tool"
		},

		// Theme names
		{
			"Hell",
			"Dunkel"
		}
	},

	// French
	{
		// Language name
		"Français",

		// Main page strings
		"Contrôle",
		"Imprimer",
		"Console",
		"Installation",
		"Actuel" THIN_SPACE DEGREE_SYMBOL "C",
		"Actif" THIN_SPACE DEGREE_SYMBOL "C",
		"Standby" THIN_SPACE DEGREE_SYMBOL "C",
		"Mouvement",
		"Extrusion",
		"Macro",
		"ARRÊT",

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
		"Affichage en négatif",
		"Inverser affichage",
		"Théme",
		"Luminosité -",
		"Luminosité +",
		"Sauver paramêtres",
		"Effacer paramêtres",
		"Sauvegarde & Redémarrage",

		// Misc
		"Confirmer le réinitialisation de l'imprimante",
		"Confirm Redémarrage",
		"Confirm suppression fichier",
		"Vous êtes sûre?",
		"Appuyer sur le point",
		"Certains réglages ne sont pas sauvegardés!",
		"Appuyer sur Sauvegarde et Redémarrage pour utiliser les nouveaux réglages",
		"Restart required",
		"Restart now?",
		"Mouvement de la  tête",
		"Quantité de Matiére extrudée (mm)",
		"Vitesse (mm/s)",
		"Extruder",
		"Retracter",
		"Baby stepping",
		"décalage Z courant : ",
		"Message",
		"Messages",
		"Version du firmware du Panel Due ",	// note space at end

		// File popup
		"Fichier sur carte ",					// note the space on the end
		"Macros",
		"Erreur ",								// note the space at the end
		" accés SD card en cours",				// note the space at the start
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
			"Arrêt",
			"Démarrage",
			"Pause",
			"Occupé"
			"Pause",
			"Reprise",
			"Téleverser le firmware",
			"Changement d'outil"
		},

		// Theme names
		{
			"Fond Blanc",
			"Fond Noir"
		}
	},
	// Czech
		{
			// Language name
			"Čeština",

			// Main page strings
			"Ovládání",
			"Tisk",
			"Konzole",
			"Nastavení",
			"Aktuální" THIN_SPACE DEGREE_SYMBOL "C",
			"Aktivní" THIN_SPACE DEGREE_SYMBOL "C",
			"Nečinná" THIN_SPACE DEGREE_SYMBOL "C",
			"Pohyb",
			"Extruder",
			"Makra",
			"STOP",

			// Print page
			"Extruder" THIN_SPACE "%",
			"Rychl. ",							// note space at end
			"Vent. ",							// note space at end
			"Čas do konce: ",
			"soubor ",							// note space at end
			", materiál ",						// note space at end
			", vrstva ",							// note space at end
			"n/a",
			"Pozastavit",
			"Baby step",
			"Pokračovat",
			"Zrušit",
			"OK",

			// Setup page
			"Hlasitost ",							// note space at end
			"Kalibrace dotyku",
			"Zrcadlit displej",
			"Obrátit displej",
			"Motiv",
			"Podsvícení -",
			"Podsvícení +",
			"Uložit nastavení",
			"Smazat nastavení",
			"Uložit a Restart",

			// Misc
			"Skutečně obnovit tovární nastavení?",
			"Restartovat?",
			"Skutečně smazat?",
			"Určitě?",
			"Dotkněte se bodu",
			"Některá nastavení nejsou uložena!",
			"Zvolte Uložit a Restart pro dokončení",
			"Vyžadován restart",
			"Restartovat nyní?",
			"Posun hlavy",
			"Množství (mm)",
			"Rychlost (mm/s)",
			"Vytlačit (extr.)",
			"Zatlačit (retr.)",
			"Baby stepping",
			"Aktuální Z offset: ",
			"Zpráva",
			"Zprávy",
			"Verze firmware Panel Due ",	// note space at end
			// File popup
			"Soubory na kartě ",				// note the space on the end
			"Makra",
			"Chyba ",						// note the space at the end
			" přístupu ke kartě",			// note the space at the start
			"Název: ",
			"Velikost: ",
			"Výška vrstvy: ",
			"Výška objektu: ",
			"Spotřeba (mat.): ",
			"Slicer: ",
			"Simulace",

			// Printer status strings
			{
				"Připojování",
				"Nečinný",
				"Tiskne",
				"Zastaven",
				"Startuje",
				"Pozastaven",
				"Zaneprázdněný",
				"Pozastavuje se",
				"Pokračuje",
				"Nahrává firmware",
				"Výměna nástroje"
			},

			// Theme names
			{
				"Světlý",
				"Tmavý"
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
