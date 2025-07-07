#include "Diagnostics.hh"

#define MAX_DIAGNOSTICS

namespace DIAGNOSTICS
{
	namespace Detail
	{
		std::vector<DIAGNOSTIC> SDiag;
	}

	void PrintAll()
	{
		for(const DIAGNOSTIC& d : Detail::SDiag)
		{
			std::string message = std::format("{}: {}:{}: {}: {}",
				d.file,
				d.line,
				d.column,
				ToString(d.level),
				d.message);

			if(!d.lineText.empty())
			{
				message += '\n';
				message += d.lineText;
			}

			Console::PrintErrorLine("{}", message);
		}

		Detail::SDiag.clear();
	}
}