#ifndef CONSOLE_H
#define CONSOLE_H

#include <string>

// Small cross-platform console helpers.
//
// Every read goes through std::getline, so a stray word or a blank line can
// never leave characters in the buffer for the next prompt to swallow, and a
// non-numeric answer can never spin the menu loop.
namespace Console {

void clear();
void pause(const std::string& message = "Press Enter to continue...");

// Returns false only on end-of-input (Ctrl+Z / Ctrl+D), which callers should
// treat as "quit". Anything invalid is re-prompted instead.
bool readInt(const std::string& prompt, int min, int max, int& out);
bool readLine(const std::string& prompt, std::string& out);

void heading(const std::string& title);
void sleepMs(unsigned int milliseconds);

}  // namespace Console

#endif  // CONSOLE_H
