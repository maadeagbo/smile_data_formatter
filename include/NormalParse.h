#pragma once

#include "ddFileIO.h"
#include <vector>

struct Args {
	bool create_canonical = false;
	bool help = false;
	std::string stripped_filename = "";
	std::string output_dir = "";
	std::string input_file = "";
	std::string input_dir = "";
	std::string canon_in = "";
	std::string canon_gt = "";
	std::vector<std::string> queries;
};

typedef std::vector<std::vector<std::string>> output_data;

/** \brief Parse CSV and extract data row-by-row*/
output_data parse_csv(const Args& args);

/** \brief Output csv file*/
void write_data(const Args& args, const output_data& data);

/** \brief Extract queries from file */
std::vector<std::string> extract_queries(const char* file);

/** \brief format new input and groundtruth csv's */
void create_formatted_csvs(Args args);

/** \brief */
dd_array<std::string> load_files(const char *directory);