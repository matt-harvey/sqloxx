#ifndef GUARD_info_hpp_1545210001778421
#define GUARD_info_hpp_1545210001778421

#include <jewel/version_fwd.hpp>

namespace sqloxx
{

/**
 * Provides information about the Sqloxx library.
 */
class Info
{
public:
	
	/**
	 * @returns a jewel::Version object representing the library version.
	 *
	 * Exception safetly: </em>nothrow guarantee</em>
	 */
	static jewel::Version version();

	Info() = delete;
	Info(Info const& rhs) = delete;
	Info(Info&& rhs) = delete;
	Info& operator=(Info const& rhs) = delete;
	Info& operator=(Info&& rhs) = delete;
	~Info() = delete;

};  // class Info

}  // namespace sqloxx

#endif  // GUARD_info_hpp_1545210001778421
