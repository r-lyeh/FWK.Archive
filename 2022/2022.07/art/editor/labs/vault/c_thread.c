// - rlyeh, public domain

#pragma once

#if defined _MSC_VER || defined __TINYC__
#define THREAD __declspec(thread)
#else
#define THREAD __thread
#endif
