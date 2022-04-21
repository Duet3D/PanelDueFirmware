#include <cstddef>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <string>

#include "ObjectModel/PrinterStatus.hpp"

enum class DisplayDimmerType : uint8_t
{
	never = 0,				// never dim the display
	onIdle, 				// only display when printer status is idle
	always,					// default - always dim
	NumTypes
};

enum class HeaterCombineType : uint8_t
{
	notCombined = 0,
	combined,
	NumTypes
};


#include <UI/Strings.hpp>

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))

static int print_index(size_t index);

static int print_all(void)
{
	for (size_t i = 1; i < ARRAY_SIZE(LanguageTables); i++) {
		print_index(i);
	}

	return 0;
}

static int print_index(size_t index)
{
	const StringTable *orig = &LanguageTables[0];

	if (index >= ARRAY_SIZE(LanguageTables))
		return -1;

	const StringTable *translation = &LanguageTables[index];

#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)
#define PRINT_ENTRY(name) do { std::cout << STRINGIFY(orig->name) << ": '" << orig->name << "'" << ", "; \
			translation->name ? std::cout << "'" << translation->name << "'" : std::cout << "MISSING"; \
			translation->name ? std::cout << " l: " << std::strlen(orig->name) << "/" << std::strlen(translation->name) << std::endl : std::cout << std::endl; } while(0)

	std::cout << "*****: " << translation->languageName << std::endl;
	PRINT_ENTRY(languageName);
	PRINT_ENTRY(control);
	PRINT_ENTRY(print);
	PRINT_ENTRY(status);
	PRINT_ENTRY(console);
	PRINT_ENTRY(setup);
	PRINT_ENTRY(pendant);
	PRINT_ENTRY(current);
	PRINT_ENTRY(active);
	PRINT_ENTRY(standby);
	PRINT_ENTRY(move);
	PRINT_ENTRY(extrusion);
	PRINT_ENTRY(macro);
	PRINT_ENTRY(stop);

	// Print page
	PRINT_ENTRY(extruderPercent);
	PRINT_ENTRY(speed);
	PRINT_ENTRY(fan);
	PRINT_ENTRY(timeRemaining);
	PRINT_ENTRY(simulated);
	PRINT_ENTRY(file);
	PRINT_ENTRY(filament);
	PRINT_ENTRY(slicer);
	PRINT_ENTRY(notAvailable);
	PRINT_ENTRY(pause);
	PRINT_ENTRY(babystep);
	PRINT_ENTRY(resume);
	PRINT_ENTRY(cancel);
	PRINT_ENTRY(reprint);
	PRINT_ENTRY(resimulate);
	PRINT_ENTRY(set);

	// Setup page
	PRINT_ENTRY(volume);
	PRINT_ENTRY(calibrateTouch);
	PRINT_ENTRY(mirrorDisplay);
	PRINT_ENTRY(invertDisplay);
	PRINT_ENTRY(theme);
	PRINT_ENTRY(brightnessDown);
	PRINT_ENTRY(brightnessUp);
	PRINT_ENTRY(saveSettings);
	PRINT_ENTRY(clearSettings);
	PRINT_ENTRY(saveAndRestart);
	PRINT_ENTRY(infoTimeout);
	PRINT_ENTRY(screensaverAfter);
	PRINT_ENTRY(babystepAmount);
	PRINT_ENTRY(feedrate);

	// Misc
	PRINT_ENTRY(confirmFactoryReset);
	PRINT_ENTRY(confirmFileDelete);
	PRINT_ENTRY(areYouSure);
	PRINT_ENTRY(touchTheSpot);
	PRINT_ENTRY(moveHead);
	PRINT_ENTRY(extrusionAmount);
	PRINT_ENTRY(extrusionSpeed);
	PRINT_ENTRY(extrude);
	PRINT_ENTRY(retract);
	PRINT_ENTRY(babyStepping);
	PRINT_ENTRY(currentZoffset);
	PRINT_ENTRY(message);
	PRINT_ENTRY(messages);
	PRINT_ENTRY(firmwareVersion);
	PRINT_ENTRY(response);

	// File popup
	PRINT_ENTRY(filesOnCard);
	PRINT_ENTRY(macros);
	PRINT_ENTRY(error);
	PRINT_ENTRY(accessingSdCard);
	PRINT_ENTRY(fileName);
	PRINT_ENTRY(fileSize);
	PRINT_ENTRY(layerHeight);
	PRINT_ENTRY(objectHeight);
	PRINT_ENTRY(filamentNeeded);
	PRINT_ENTRY(generatedBy);
	PRINT_ENTRY(lastModified);
	PRINT_ENTRY(estimatedPrintTime);
	PRINT_ENTRY(simulatedPrintTime);
	PRINT_ENTRY(simulate);


	std::cout << "Status Values:" << std::endl;
	PRINT_ENTRY(statusValues[(unsigned int)OM::PrinterStatus::connecting]);
	PRINT_ENTRY(statusValues[(unsigned int)OM::PrinterStatus::idle]);
	PRINT_ENTRY(statusValues[(unsigned int)OM::PrinterStatus::printing]);
	PRINT_ENTRY(statusValues[(unsigned int)OM::PrinterStatus::stopped]);
	PRINT_ENTRY(statusValues[(unsigned int)OM::PrinterStatus::configuring]);
	PRINT_ENTRY(statusValues[(unsigned int)OM::PrinterStatus::paused]);
	PRINT_ENTRY(statusValues[(unsigned int)OM::PrinterStatus::resuming]);
	PRINT_ENTRY(statusValues[(unsigned int)OM::PrinterStatus::flashing]);
	PRINT_ENTRY(statusValues[(unsigned int)OM::PrinterStatus::toolChange]);
	PRINT_ENTRY(statusValues[(unsigned int)OM::PrinterStatus::simulating]);
	PRINT_ENTRY(statusValues[(unsigned int)OM::PrinterStatus::off]);
	PRINT_ENTRY(statusValues[(unsigned int)OM::PrinterStatus::cancelling]);

	std::cout << "Colour Scheme Names:" << std::endl;
	for (size_t j = 0; j < ARRAY_SIZE(orig->colourSchemeNames); j++) {
		PRINT_ENTRY(colourSchemeNames[j]);
	}

	std::cout << "Display Dimming Names:" << std::endl;
	PRINT_ENTRY(displayDimmingNames[(unsigned int)DisplayDimmerType::never]);
	PRINT_ENTRY(displayDimmingNames[(unsigned int)DisplayDimmerType::always]);
	PRINT_ENTRY(displayDimmingNames[(unsigned int)DisplayDimmerType::onIdle]);

	std::cout << "Heater Combine Type Names:" << std::endl;
	PRINT_ENTRY(heaterCombineTypeNames[(unsigned int)HeaterCombineType::notCombined]);
	PRINT_ENTRY(heaterCombineTypeNames[(unsigned int)HeaterCombineType::combined]);
	
	return 0;

}

static int print_lang(const char *name)
{
	std::string n(name);

	for (size_t i = 0; i < ARRAY_SIZE(LanguageTables); i++) {
		const StringTable *translation = &LanguageTables[i];

		if (n.compare(translation->languageName) == 0) {
			print_index(i);
			return 0;
		}
	}

	return 0;
}

static int list_languages(void)
{
	for (size_t i = 0; i < ARRAY_SIZE(LanguageTables); i++) {
		const StringTable *lang = &LanguageTables[i];

		std::cout << lang->languageName << std::endl;
	}

	return 0;
}

int main(int argc, char *argv[])
{

	if (argc <= 1 || std::string("all").compare(argv[1]) == 0) {
		return print_all();
	} else if (std::string("list").compare(argv[1]) == 0) {
		return list_languages();
	} else if (std::string("help").compare(argv[1]) == 0) {
		std::cout << argv[0] << " all|list|help|LANGUAGE_NAMES..." << std::endl << std::endl;
		std::cout << "all - print all translations" << std::endl
			  << "list - list all language names" << std::endl
			  << "help - print this help" << std::endl
			  << "LANGUAGES_NAMES - pass a list of languages names to print languages' translations" << std::endl;
	} else {
		for (ssize_t i = 1; i < argc; i++) {
			print_lang(argv[i]);
		}

		return 0;
	}

	return 0;
}
