#ifndef REZ_HPP
#define REZ_HPP

#pragma once

namespace rez
{

inline bool g_dtx_to_lithtech = false;

void extract( const std::vector<std::filesystem::path>& file_path, const std::filesystem::path& save_path );

}

#endif
