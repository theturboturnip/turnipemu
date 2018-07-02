#pragma once

namespace TurnipEmu {
	void LogLine(const char* tag, const char* format, ...);
	void Indent();
	void Unindent();
}
