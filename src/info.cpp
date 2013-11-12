#include "info.hpp"
#include <jewel/version.hpp>

using jewel::Version;

namespace sqloxx
{

Version
Info::version()
{
	return Version
	(	SQLOXX_VERSION_MAJOR,
		SQLOXX_VERSION_MINOR,
		SQLOXX_VERSION_PATCH
	);
}

}  // namespace sqloxx
