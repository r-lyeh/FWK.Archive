// - rlyeh, public domain

#pragma once

#ifndef _MSC_VER
#define SECTION(name) __attribute__((section("." #name "#")))
#else
#define SECTION(name) __declspec(allocate("." #name))
#endif
