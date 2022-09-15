#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#pragma once

#include <string_view>

namespace rez::con {

/**
 * @brief attach or alloc to a console
 * @param new console title
 */
void attach( const std::string_view& title );

}

#endif