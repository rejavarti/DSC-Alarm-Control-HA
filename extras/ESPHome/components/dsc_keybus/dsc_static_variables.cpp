// Global DSC interface instances and static variable definitions
// These are defined here to avoid multiple definition errors

#if defined(dscClassicSeries)

#include "dsc_arduino_compatibility.h"
#include "dscClassic.h"
// Include the default pin definitions
#include "dsc_keybus.h"

// Global DSC interface instance definition
dscClassicInterface dsc(DSC_DEFAULT_CLOCK_PIN, DSC_DEFAULT_READ_PIN, 
                        #ifdef DSC_CLASSIC_PC16_PIN
                        DSC_CLASSIC_PC16_PIN,
                        #else
                        DSC_DEFAULT_PC16_PIN,
                        #endif
                        DSC_DEFAULT_WRITE_PIN);

#elif defined(dscPowerSeries)

#include "dsc_arduino_compatibility.h"
#include "dscKeybus.h"
#include "dsc_keybus.h"

// Global DSC interface instance definition for PowerSeries
dscKeybusInterface dsc(DSC_DEFAULT_CLOCK_PIN, DSC_DEFAULT_READ_PIN, DSC_DEFAULT_WRITE_PIN);

#endif // defined(dscClassicSeries) / defined(dscPowerSeries)