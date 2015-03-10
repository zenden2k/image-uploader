#ifndef IMAGEEDITOR_LOGGER_H
#define IMAGEEDITOR_LOGGER_H

namespace ImageEditor {
	class Logger {
		public:
			static void Write(const TCHAR* szFormat, ...);
	};

};
#endif