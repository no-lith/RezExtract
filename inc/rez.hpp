#ifndef REZ_HPP
#define REZ_HPP

#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace rez {

/**
 * @brief 
 * @param file_path 
 * @param save_path 
 */
void extract(
	const std::vector<std::filesystem::path>& file_path,
	const std::wstring& save_path
);

}

#endif