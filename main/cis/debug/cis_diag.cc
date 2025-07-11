#include "cis_diag.hh"

#define MAX_DIAGNOSTICS

namespace DIAGNOSTICS
{
	namespace Detail
	{
		std::vector<DIAGNOSTIC> SDiag;
	}

	void print_all()
	{
		for(const DIAGNOSTIC& d : Detail::SDiag)
		{
			std::string message = std::format("{}: {}:{}: {}: {}",
				d.file,
				d.line,
				d.column,
				to_string(d.level),
				d.message);

			if(!d.line_text.empty())
			{
				message += '\n';
				message += d.line_text;
			}

			Console::PrintErrorLine("{}", message);
		}

		Detail::SDiag.clear();
	}
}