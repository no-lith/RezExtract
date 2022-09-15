#ifndef FILE_DIALOG_HPP
#define FILE_DIALOG_HPP

#pragma once

#include <filesystem>
#include <string>
#include <vector>

/**
 * @brief File Dialog
 */
namespace rez::fdg {

/**
 * @brief get the path of multiple files
 * @return a vector with the path of each selected file
 */
auto open( ) -> std::vector<std::filesystem::path>;

/**
 * @brief get the path of a folder
 * @return a wstring with the selected path
 */
auto save( ) -> std::wstring;

}

#endif