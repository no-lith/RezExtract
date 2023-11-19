#ifndef REZ_HPP
#define REZ_HPP

#pragma once

namespace rez
{

inline std::int32_t g_convert_dtx = -1;

void extract( const std::vector<std::filesystem::path>& file_path, const std::filesystem::path& save_path );

}

#endif
