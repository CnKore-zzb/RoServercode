#pragma once
#include "xDefine.h"

#define SET_FLAG_MACRO(flags, sit, val)\
	do{\
		if (val)\
			*flags |= ((QWORD)0x01<<sit);\
		else\
			*flags &= ~((QWORD)0x01<<sit);\
	}while (false)

template<typename T, typename M>
inline void setFlag(T &flags, const M &sit, bool val = true)
{
	SET_FLAG_MACRO(&flags, sit, val);
}
template<typename T, typename M>
inline void setFlag(T *flags, const M &sit, bool val = true)
{
	if (flags) 
		SET_FLAG_MACRO(flags, sit, val);
}


template<typename T, typename M>
inline void resetFlag(T &flags, const M &sit)
{
	flags &= ~((QWORD)0x01<<sit);
}
template<typename T, typename M>
inline void resetFlag(T *flags, const M &sit)
{
	if (flags)
		*flags &= ~((QWORD)0x01<<sit);
}


template<typename T, typename M>
inline void flipFlag(T &flags, const M &sit)
{
	flags ^= ((QWORD)0x01<<sit);
}
template<typename T, typename M>
inline void flipFlag(T *flags, const M &sit)
{
	if (flags)
		*flags ^= ((QWORD)0x01<<sit);
}


template<typename T, typename M>
inline bool isSetFlag(const T &flags, const M &sit)
{
	return (flags&((QWORD)0x01<<sit))!=0;
}

template<typename T, typename M>
inline T maskFlag(const T &flags, const M &sit)
{
	return flags & ~((QWORD)0x01<<sit);
}

