#ifndef FILE_DIALOG_HPP
#define FILE_DIALOG_HPP

#pragma once

namespace rez::fdg
{

auto open_file(
	const std::string_view title,
	const std::string_view filter,
	const HWND hwnd
) -> std::vector<std::filesystem::path>;

auto open_folder(
	const std::wstring_view title,
	const HWND hwnd
) -> std::filesystem::path;

}

#endif
