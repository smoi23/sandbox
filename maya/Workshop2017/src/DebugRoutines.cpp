/*	Version: 1.0.2
	Change Log:
	last edit: 08.07.2012
*/
#ifndef DEBUG
#define DEBUG 1
#endif
#ifndef DEBUGINMAYA
#define DEBUGINMAYA 0
#endif

#include <maya/MGlobal.h>

// --------------------------------------------------- Macro for info output
#define PRINT_DEBUG(msg)					\
	if (DEBUG) {						\
		cout << msg << endl;			\
	}									\
	if (DEBUGINMAYA) {						\
		MGlobal::displayInfo(msg);			\
	}
// --------------------------------------------------- Macro for info output
#define OutInfo2(msg)					\
	MGlobal::displayInfo(msg);	
// --------------------------------------------------- Macro for error checking
#define CHECK_ERR(stat, msg)					\
    if ( MS::kSuccess != stat ) {			\
    	MGlobal::displayError(msg+stat.errorString()+stat.statusCode());				\
		return stat;						\
	} 
// --------------------------------------------------- Macro for warning output
#define OUT_WARNING(msg)					\
	MGlobal::displayWarning(msg);	

