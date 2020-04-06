#pragma once

namespace SR
{
	template<typename T>
	class ISealed
	{
		friend T;
	private:
		ISealed() {}
		ISealed(const ISealed& o) {}
		ISealed(ISealed&& o) {}
	};
}