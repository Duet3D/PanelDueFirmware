/*
 * Strings.hpp
 *
 *  Created on: 27 Feb 2017
 *      Author: David
 *
 * The encoding used for this file must be UTF-8 to ensure that accented characters are displayed correctly.
 */

#ifndef SRC_UI_STRINGS_HPP_
#define SRC_UI_STRINGS_HPP_

#include "ecv.h"
#undef array
#undef result
#undef value
#include "UI/ColourSchemes.hpp"
#include "ObjectModel/PrinterStatus.hpp"
#include "Configuration.hpp"

#define CSTRING const char * const _ecv_array
#define Newline			"\n"
#define DegreeSymbol	"\u00B0"

// Unicode strings for special characters in our font
#define DECIMAL_POINT	"\xC2\xB7"		// Unicode middle-dot, code point B7
#define DEGREE_SYMBOL	"\xC2\xB0"		// Unicode degree-symbol, code point B0
#define THIN_SPACE		"\xC2\x80"		// Unicode control character, code point 0x80, we use it as thin space
#define LEFT_ARROW		"\xC2\x81"		// Unicode control character, code point 0x81, we use it as up arrow
#define UP_ARROW		"\xC2\x82"		// Unicode control character, code point 0x82, we use it as up arrow
#define RIGHT_ARROW		"\xC2\x83"		// Unicode control character, code point 0x83, we use it as down arrow
#define DOWN_ARROW		"\xC2\x84"		// Unicode control character, code point 0x84, we use it as down arrow
#define MORE_ARROW		"\xC2\x85"
#define LESS_ARROW		"\xC2\x86"

struct StringTable
{
	// Language name
	CSTRING languageName;

	// Main page strings
	CSTRING control;
	CSTRING print;
	CSTRING status;
	CSTRING console;
	CSTRING setup;
	CSTRING current;
	CSTRING active;
	CSTRING standby;
	CSTRING move;
	CSTRING extrusion;
	CSTRING macro;
	CSTRING stop;

	// Status page
	CSTRING extruderPercent;
	CSTRING speed;
	CSTRING fan;
	CSTRING timeRemaining;
	CSTRING simulated;
	CSTRING file;
	CSTRING filament;
	CSTRING slicer;
	CSTRING notAvailable;
	CSTRING pause;
	CSTRING babystep;
	CSTRING resume;
	CSTRING cancel;
	CSTRING reprint;
	CSTRING resimulate;
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
	CSTRING infoTimeout;
	CSTRING screensaverAfter;
	CSTRING babystepAmount;
	CSTRING feedrate;

	// Misc
	CSTRING confirmFactoryReset;
	CSTRING confirmFileDelete;
	CSTRING areYouSure;
	CSTRING touchTheSpot;
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
	CSTRING response;

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
	CSTRING lastModified;
	CSTRING estimatedPrintTime;
	CSTRING simulatedPrintTime;
	CSTRING simulate;

	// Printer status strings
	CSTRING statusValues[(unsigned int)OM::PrinterStatus::NumTypes];

	// Colour theme names
	CSTRING colourSchemeNames[NumColourSchemes];

	// Display dimmer types
	CSTRING displayDimmingNames[(unsigned int)DisplayDimmerType::NumTypes];

	CSTRING heaterCombineTypeNames[(unsigned int)HeaterCombineType::NumTypes];

};

const StringTable LanguageTables[] =
{
	// English
	{
		// ISO-639.1 language code
		"en",

		// Main page strings
		"Control",
		"Print",
		"Status",
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
		"sim'd ",							// note space at end
		"file ",							// note space at end
		"filament ",						// note space at end
		"slicer ",							// note space at end
		"n/a",
		"Pause",
		"Baby step",
		"Resume",
		"Cancel",
		"Print again",
		"Simulate again",
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
		"Info timeout ",					// note space at end
		"Screensaver ",						// note space at end
		"Babystep ",						// note space at end
		"Feedrate ",						// note space at end

		// Misc
		"Confirm factory reset",
		"Confirm file delete",
		"Are you sure?",
		"Touch the spot",
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
		"Response",

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
		"Last modified: ",
		"Estimated print time: ",
		"Simulated print time: ",
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
			"Simulating",
			"Off",
			"Cancelling",
		},

		// Theme names
		{
			"Light theme",
			"Dark theme 1",
			"Dark theme 2"
		},

		// Display dimming types
		{
			"Never dim",
			"Dim if idle",
			"Always dim"
		},

		// Heater combine types
		{
			"Heat's not comb.",
			"Heaters comb.",
		}
 	},

	// German
	{
		// ISO-639.1 language code
		"de",

		// Main page strings
		"Steuerung",
		"Druck",
		"Status",
		"Konsole",
		"Einstellung",
		"Istwert" THIN_SPACE DEGREE_SYMBOL "C",
		"Aktiv" THIN_SPACE DEGREE_SYMBOL "C",
		"Standby" THIN_SPACE DEGREE_SYMBOL "C",
		"Bewegung",
		"Extrusion",
		"Makro",
		"STOP",

		// Print page
		"Extruder" THIN_SPACE "%",
		"Speed ",							// note space at end. Was "Geschwindigkeit " but that is too long to fit in the space available.
		"Lüfter ",							// note space at end
		"Restzeit: ",
		"Simul. ",							// note space at end
		"Datei ",							// note space at end
		"Filament ",						// note space at end
		"Slicer ",							// note space at end
		"n/v",
		"Pause",
		"Einzelschritt",
		"Fortsetzen",
		"Abbrechen",
		"Erneut drucken",
		"Erneut simulieren",
		"Set",

		// Setup page
		"Lautstärke ",						// note space at end
		"Touch kalibrieren",
		"Anzeige spiegeln",
		"Anzeige umkehren",
		"Darstellung",
		"Helligkeit  -",
		"Helligkeit  +",
		"Einstellungen sichern",
		"Werksreset",
		"Sichern & Neustarten",
		"Info Timeout ",					// note space at end
		"Bildschirmschoner ",					// note space at end
		"Babystep ",						// note space at end
		"Feedrate ",						// note space at end

		// Misc
		"Alle Einstellungen zurücksetzen",
		"Die Datei wird gelöscht",
		"Sind sie sicher?",
		"Bitte auf den Punkt tippen",
		"Kopf bewegen",
		"Extrusionsmenge (mm)",
		"Geschwindigkeit (mm/s)",
		"Extrudieren",
		"Zurückziehen",
		"Einzelschritte",
		"Aktueller Z-Versatz: ",
		"Nachricht",
		"Nachrichten",
		"Panel Due Firmwareversion ",	// note space at end
		"Antwort",

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
		"Letzte Änderung: ",
		"Geschätzte Druckdauer: ",
		"Simulierte Druckdauer: ",
		"Simulieren",

		// Printer status strings
		{
			"Verbinde",
			"Leerlauf",
			"Druckt",
			"Angehalten",
			"Starte",
			"Pausiert",
			"Beschäftigt",
			"Pausiere",
			"Fortsetzen",
			"Firmware-Upload",
			"Wechsle Tool",
			"Simuliert",
			"Aus",
			"Abbrechen",
		},

		// Theme names
		{
			"Anzeige hell",
			"Anzeige dunkel 1",
			"Anzeige dunkel 2"
		},

		// Display dimming types
		{
			"Dimmen aus",
			"Dim bei idle",				// shortened due to space limitations, ideally "Nur im Standby dimmen"
			"Dimmen ein"
		},

		// Heater combine types
		{
			"Heat's nicht komb.",
			"Heat's kombiniert"
		}
	},

	// French
	{
		// ISO-639.1 language code
		"fr",

		// Main page strings
		"Contrôle",
		"Imprimer",
		"Statut",
		"Console",
		"Configuration",
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
		"Ventilo ",								// note space at end. "Ventilateur 0%" was too long to fit.
		"Temps Restant: ",
		"Simul. ",								// note space at end
		"Fichier ",								// note space at end
		"filament ",							// note space at end
		"slicer ",								// note space at end
		"n/a",
		"Pause",
		"Baby step",
		"Reprise",
		"Annuler",
		"Réimprimer",
		"Resimuler",
		"Set",

		// Setup page
		"Volume ",								// note space at end
		"Calibrer l'écran",
		"Affichage en négatif",
		"Inverser affichage",
		"Théme",
		"Luminosité -",
		"Luminosité +",
		"Sauver paramêtres",
		"Réinitialisation",
		"Sauvegarde & Redémarrage",
		"Info timeout ",						// note space at end
		"Veille ecran ",							// note space at end
		"Babystep ",							// note space at end
		"Feedrate ",							// note space at end

		// Misc
		"Confirmer la réinitialisation",
		"Confirmer la suppression du fichier",
		"Etes-vous sûre?",
		"Appuyer sur le point",
		"Mouvement de la  tête",
		"Quantité de matière extrudée (mm)",
		"Vitesse (mm/s)",
		"Extruder",
		"Rétracter",
		"Baby stepping",
		"Z offset actuel:  ",
		"Message",
		"Messages",
		"Version du firmware du Panel Due ",	// note space at end
		"Réponse",

		// File popup
		"Fichier sur la carte ",					// note the space on the end
		"Macros",
		"Erreur ",								// note the space at the end
		" accés à la carte SD en cours",				// note the space at the start
		"Nom du fichier : ",
		"Taille : ",
		"Hauteur de couche: ",
		"Hauteur de l'objet: ",
		"Filament requis: ",
		"Slicé par: ",
		"Dernière modification: ",
		"Temps d'impression estimé: ",
		"Temps d'impression simulé: ",
		"Simuler",

		// Printer status strings
		{
			"Conn. en cours",					// "Connexion en cours" was too long
			"Au repos",
			"Impression",
			"Arrêté",
			"Démarrage",
			"Pause",
			"Occupé",
			"Pause",
			"Reprendre",
			"Updating",
			"Chgt d'outil",
			"Simul. en cours",
			"Off",
			"Annulation..."
		},

		// Theme names
		{
			"Fond Blanc",
			"Fond Noir 1",
			"Fond Noir 2"
		},

		// Display dimming types
		{
			"Jamais tamiser",
			"Tamiser si inactive",
			"Toujours tamiser"
		},

		// Heater combine types
		{
			"Heat's not comb.",
			"Heaters comb.",
		}
	},

	// Spanish
	{
		// ISO-639.1 language code
		"es",

		// Main page strings
		"Control",
		"Imprimir",
		"Estado",
		"Consola",
		"Configuración",
		"Actual" THIN_SPACE DEGREE_SYMBOL "C",
		"Activo" THIN_SPACE DEGREE_SYMBOL "C",
		"Inactivo" THIN_SPACE DEGREE_SYMBOL "C",
		"Mover",
		"Extrusión",
		"Macro",
		"PARADA",							// It could also be STOP, both are OK

		// Print page
		"Extrusor" THIN_SPACE "%",
		"Veloc. ",							// note space at end. "Velocidad" is too long.
		"Ventil. ",							// note space at end. "Ventilador" is too lonh.
		"Tiempo restante: ",
		"simul. ",							// note space at end
		"archivo ",							// note space at end
		"filamento ",						// note space at end
		"laminador ",							// note space at end
		"n/d",								// Not available / no disponible
		"Pausa",
		"Micro paso",						// Literal translation of baby step it's very odd in spanish...
		"Reanudar",
		"Cancelar",
		"Imprimir de nuevo",
		"Simular de nuevo",
		"Fijar",							// "Establecer" would be more correct, but it's longer.

		// Setup page
		"Volumen ",							// note space at end
		"Calibrar toque",					// this one is tricky because "touch" is very known in regard to screens...
		"Espejar pantalla",
		"Invertir pantalla",
		"Tema",
		"Brillo -",
		"Brillo +",
		"Guardar parámetros",
		"Borrar parámetros",
		"Guardar y Reiniciar",
		"Info timeout ",					// note space at end
		"Salvapantallas ",					// note space at end
		"Micropaso ",						// note space at end
		"Vel. avance",						// note space at end

		// Misc
		"Confirma restablecimiento de fábrica",
		"Confirma borrar archivo",
		"Está seguro?",
		"Toca el punto",
		"Mover cabezal",
		"Cantidad de extrusión (mm)",
		"Velocidad (mm/s)",
		"Extruir",
		"Retraer",
		"Micro pasos",
		"Separación actual de Z: ",
		"Mensaje",
		"Mensajes",
		"Versión del Firmware del Panel Due ",	// note space at end
		"Respuesta",

		// File popup
		"Archivos en la tarjeta ",			// note the space on the end
		"Macros",
		"Error ",							// note the space at the end
		" accediendo a la tarjeta SD",		// note the space at the start
		"Nombre del archivo: ",
		"Tamaño: ",
		"Altura de capa: ",
		"Altura de objeto: ",
		"Filamento necesario: ",
		"Laminado por: ",					// there is no translation in spanish for this meaning, so I proposed to use "processed by" which is understandable
		"Última modificación: ",
		"Tiempo estimado de impresión: ",
		"Tiempo de impresión simulado: ",
		"Simular",

		// Printer status strings
		{
			"conectando",
			"en espera",					// it's more frequently use "en espera" than "ocioso", it makes more sense for a machine
			"imprimiendo",
			"parado",
			"empezando",
			"pausado",
			"ocupado",
			"pausando",
			"reanudando",
			"carga del firmware",
			"cambio de herramienta",
			"simulando",
			"apagado",
			"cancelando"
		},

		// Theme names
		{
			"Claro",
			"Oscuro 1",
			"Oscuro 2"
		},

		// Display dimming types
		{
			"Nunca Atenuar",
			"Atenuar en espera",
			"Siempre Atenuar",
		},

		// Heater combine types
		{
			"Heat's not comb.",
			"Heaters comb.",
		}
	},

	// Czech
	{
		// ISO-639.1 language code
		"cs",

		// Main page strings
		"Ovládání",
		"Tisk",
		"Status",
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
		"simul. ",							// note space at end
		"slicer ",							// note space at end
		"soubor ",							// note space at end
		"materiál ",						// note space at end
		"n/a",
		"Pozastavit",
		"Baby step",
		"Pokračovat",
		"Zrušit",
		"Print again",
		"Simulate again",
		"OK",

		// Setup page
		"Hlasitost ",						// note space at end
		"Kalibrace dotyku",
		"Zrcadlit displej",
		"Obrátit displej",
		"Motiv",
		"Podsvícení -",
		"Podsvícení +",
		"Uložit nastavení",
		"Smazat nastavení",
		"Uložit a Restart",
		"Info timeout ",					// note space at end
		"Screensaver ",						// note space at end
		"Babystep ",						// note space at end
		"Feedrate ",						// note space at end

		// Misc
		"Skutečně obnovit tovární nastavení?",
		"Skutečně smazat?",
		"Určitě?",
		"Dotkněte se bodu",
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
		"Odpověď",

		// File popup
		"Soubory na kartě ",			// note the space on the end
		"Makra",
		"Chyba ",						// note the space at the end
		" přístupu ke kartě",			// note the space at the start
		"Název: ",
		"Velikost: ",
		"Výška vrstvy: ",
		"Výška objektu: ",
		"Spotřeba (mat.): ",
		"Slicer: ",
		"Poslední úprava: ",
		"Zbývající čas tisku: ",
		"Simulovaný čas tisku: ",
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
			"Výměna nástroje",
			"Simulace",
			"Off",
			"Cancelling"
		},

		// Theme names
		{
			"Světlý",
			"Tmavý 1",
			"Tmavý 2"
		},

		// Display dimming types
		{
			"Nikdy neztlumit jas",
			"Pohasnout při nečinnosti",
			"Pohasnout vždy"
		},

		// Heater combine types
		{
			"Heat's not comb.",
			"Heaters comb.",
		}
	},

	// Italian
	{
		// ISO-639.1 language code
		"it",

		// Main page strings
		"Controlla",
		"Stampa",
		"Status",
		"Console",
		"Configura",
		"Corrente" THIN_SPACE DEGREE_SYMBOL "C",
		"Attiva" THIN_SPACE DEGREE_SYMBOL "C",
		"Standby" THIN_SPACE DEGREE_SYMBOL "C",
		"Muovi",
		"Estrusione",
		"Macro",
		"FERMA",

		// Print page
		"Estrusore" THIN_SPACE "%",
		"Velocità ",							// note space at end
		"Ventola ",								// note space at end
		"Tempo rimanente: ",
		"simul. ",							// note space at end
		"file ",							// note space at end
		"filamento ",						// note space at end
		"slicer ",							// note space at end
		"n/a",
		"Pausa",
		"Baby step",
		"Riprendi",
		"Cancella",
		"Stampa di nuovo",
		"Simula di nuovo",
		"Imposta",

		// Setup page
		"Volume ",							// note space at end
		"Calibra touch",
		"Specchia schermo",
		"Inverti schermo",
		"Tema",
		"Luminosità -",
		"Luminosità +",
		"Salva impostazioni",
		"Resetta impostazioni",
		"Salva & Riavvia",
		"Info timeout ",					// note space at end
		"Salvaschermo ",					// note space at end
		"Babystep ",						// note space at end
		"Feedrate ",						// note space at end

		// Misc
		"Conferma reset impostazioni",
		"Confirma eliminazione file",
		"Sei sicuro?",
		"Tocca il punto",
		"Muovi testa",
		"Quantità estrusione (mm)",
		"Velocità (mm/s)",
		"Estrudi",
		"Retrai",
		"Baby stepping",
		"Z offset corrente: ",
		"Messaggio",
		"Messaggi",
		"Versione firmware Panel Due ",	// note space at end
		"Risposta",

		// File popup
		"File su card ",				// note the space on the end
		"Macro",
		"Errore ",						// note the space at the end
		" accedendo alla SD card",			// note the space at the start
		"Nome file: ",
		"Dimensione: ",
		"Altezza layer: ",
		"Altezza oggetto: ",
		"Filamento necessario: ",
		"Slice effettuato con: ",
		"Ultima modifica: ",
		"Tempo di stampa stimato: ",
		"Tempo di stampa simulato: ",
		"Simula",

		// Printer status strings
		{
			"Connettendo",
			"Idle",
			"Stampando",
			"Fermato",
			"Avviando",
			"Pausa",
			"Occupato",
			"Pausa",
			"Riprendendo",
			"Caricamento firmware",
			"Cambiando tool",
			"Simulando",
			"Off",
			"Cancelling"
		},

		// Theme names
		{
			"Tema chiaro",
			"Tema scuro 1",
			"Tema scuro 2"
		},

		// Display dimming types
		{
			"Non attenuare",
			"Attenua se idle",
			"Attenua sempre"
		},

		// Heater combine types
		{
			"Heat's not comb.",
			"Heaters comb.",
		}
 	},

#if USE_CYRILLIC_CHARACTERS
	// Ukrainian
	{
		"uk",

		// Main page strings
		"Контроль",
		"Друк",
		"Друк",
		"Консоль",
		"Налаштування",
		"Поточна" THIN_SPACE DEGREE_SYMBOL "C",
		"Активна" THIN_SPACE DEGREE_SYMBOL "C",
		"Очікувана" THIN_SPACE DEGREE_SYMBOL "C",
		"Рух",
		"Екструзія",
		"Макро",
		"СТОП",

		// Print page
		"Екструдер" THIN_SPACE "%",
		"Швидкість ",             // note space at end
		"Вентилятор ",               // note space at end
		"Залишилось часу: ",
		"simul. ",						// note space at end
		"файл ",              			// note space at end
		"філамент ",           			// note space at end
		"slicer ",						// note space at end
		"-",
		"Пауза",
		"Мікрокрок",
		"Продовжиити",
		"Відмінити",
		"Друкувати знову",
		"Симулювати знову",
		"Встановити",

		// Setup page
		"Гучність ",              // note space at end
		"Калібрув. дотик",
		"Відзеркалити дисп.",
		"Перевернути диспл.",
		"Тема",
		"Яскравість -",
		"Яскравість +",
		"Зберегти налашт.",
		"Скинути налашт.",
		"Зберегти & Перезавантажити",
		"Вичерпаний час очікування на інформацію ",          // note space at end
		"Заставка ",           // note space at end
		"Мікрокрок ",            // note space at end
		"Подача ",            // note space at end

		// Misc
		"Підтвердіть скинення до заводських налаштуванб",
		"Підтвердіть видалення файлу",
		"Ви впевнені?",
		"Доторкніться до точки",
		"Рухати головку",
		"Кількість екструзії (мм/с)",
		"Швидкість (мм/с)",
		"Екструдувати",
		"Ретракт",
		"Мікрокрок",
		"Поточне зміщення Z: ",
		"Повідомлення",
		"Повідомлення",
		"Версія прошивки Panel Due ",  // note space at end
		"Відповідь",

		// File popup
		"Файли на картці ",       // note the space on the end
		"Макроси",
		"Помилка ",           // note the space at the end
		" доступу до SD-картки",     // note the space at the start
		"Ім'я файлу: ",
		"Розмір: ",
		"Висота шару: ",
		"Висота об'єктуt: ",
		"Потрібен філамент: ",
		"Автор слайсу: ",
		"Востаннє змінено: ",
		"Приблизний час друку: ",
		"Симульований час друку: ",
		"Симулювати",

		// Printer status strings
		{
			"Підключення",
			"Бездіяльність",
			"Друкую",
			"Зупинено",
			"Запуск",
			"Зупинений",
			"Зайнятий",
			"Зупиняю",
			"Продовжую",
			"Завантаження прошивки",
			"Змінюю інструмент",
			"Симулюю",
			"Зачекайте",
			"Ініціалізовую",
		},

		// Theme names
		{
			"Світла тема",
			"Темна тема 1",
			"Темна тема 2"
		},

		// Display dimming types
		{
			"Ніколи не тьмяніти",
			"Тьмяніти за бездіяльності",
			"Завжди тьмяніти"
		},

		// Heater combine types
		{
			"Heat's not comb.",
			"Heaters comb.",
		}
	},
	// Russian
	{
		"ru",

		// Main page strings
		"Контроль",
		"Печать",
		"Статус",
		"Консоль",
		"Настройки",
		"Текущая" THIN_SPACE DEGREE_SYMBOL "C",
		"Заданная" THIN_SPACE DEGREE_SYMBOL "C",
		"Преднагрев" THIN_SPACE DEGREE_SYMBOL "C",
		"Движения",
		"Экструзия",
		"Макрос",
		"СТОП",

		// Print page
		"Экструзия" THIN_SPACE "%",
		"Скорость ",             // note space at end
		"обдув ",               // note space at end
		"Оставшееся время: ",
		"Файл ",              // note space at end
		", Файл ",            // note space at end
		", Филамент ",             // note space at end
		"Слайсер ",
		"-",
		"Пауза",
		"Высота слоя",
		"Продолжить",
		"Oтмена",
		"Повтор печати",
		"Повтор моделирования",
		"Установить",

		// Setup page
		"Громкость ",              // note space at end
		"Калибровка экрана",
		"Отзеркаленый дисплей",
		"Перевёрнутый дисплей",
		"Тема",
		"Яркость -",
		"Яркость +",
		"Сохранить настройки",
		"Сбросить настройки",
		"Сохранить и перезапустить",
		"Ожидание информации ",          // note space at end
		"Заставка ",           // note space at end
		"Мелкий шаг ",            // note space at end
		"Скорость подачи ",            // note space at end

		// Misc
		"Подтвердить сброс настроек",
		"Подтвердить удаление файла",
		"Вы уверены?",
		"Дотронуться до точки",
		"Двигать экструдером",
		"Количество экструзии (мм)",
		"Скорость (мм / с)",
		"Экструдировать",
		"Ретракт",
		"Высота слоя",
		"Текущее смещение по оси Z: ",
		"Сообщение",
		"Сообщения",
		"Версия прошивки Panel Due ",  // note space at end
		"Ответ",

		// File popup
		"Файлы на карте ",       // note the space on the end
		"Макросы",
		"Ошибка ",           // note the space at the end
		"доступ к SD-карте",     // note the space at the start
		"Имя файла: ",
		"Размер: ",
		"Высота слоя: ",
		"Высота объекта: ",
		"Нет филамента: ",
		"Нарезано: ",
		"Последнее изменение: ",
		"Расчетное время печати: ",
		"Время имитации печати: ",
		"Симуляция ",

		// Printer status strings
		{
			"Подключение",
			"Ожидание",
			"Печать",
			"Остановлен",
			"Запуск",
			"Пауза",
			"Занятый",
			"Пауза",
			"Продолжить",
			"Загрузка прошивки",
			"Смена инструмента",
			"Симуляция",
			"Отмена",
			"Инициализация",
		},

		// Theme names
		{
			"Светлая тема",
			"Тёмная тема 1",
			"Тёмная тема 2"
		},

		// Display dimming types
		{
			"Никогда не тускнеет",
			"Уменьшить при ожидании",
			"Всегда тусклый"
		},

		// Heater combine types
		{
			"Нагреватели не комб.",
			"Нагреватели комб.",
		}
	}
#endif
};

constexpr unsigned int NumLanguages = sizeof(LanguageTables) / sizeof(LanguageTables[0]);
#endif /* SRC_UI_STRINGS_HPP_ */
