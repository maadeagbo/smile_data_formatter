#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "NormalParse.h"
#include "CanonicalParse.h"

/** \brief Parse input arguments to program */
Args parse_args(std::vector<std::string>& args_vec);

int main(int argc, char const *argv[]) {
	// parse input arguments
	std::vector<std::string> v;
	for (int i = 1; i < argc; ++i) {
		v.push_back(std::string(argv[i]));
	}
	Args args = parse_args(v);

	// leave if help screen
	if(args.help) return 0;
	
	create_formatted_csvs(args);

	// convert to canonical space	
	if (args.create_canonical) create_canonical_verts(args);

	return 0;
}

Args parse_args(std::vector<std::string>& args_vec) {
	// check if argument flag
	auto check_if_arg = [](std::string in_str) {
		return *(in_str.c_str()) == '-';
	};

	// check if next argument is a valid(non-arg) value
	auto check_value = [&](const size_t val_idx) {
		if (val_idx + 1 < args_vec.size() && !check_if_arg(args_vec[val_idx + 1])) {
			return args_vec[val_idx + 1];
		}
		return std::string("");
	};

	Args output;

	for (int i = 0; i < (int)args_vec.size(); ++i) {
		std::string str = args_vec[i];

		if (check_if_arg(str)) {
			// help message
			if (str == "-h" || str == "--help") {
				output.help = true;

				printf("\t-h \t--help \t\tHelp message\n"
							 "\t-f \t--file \t\tfile to convert\n"
							 "\t-d \t--dir  \t\tDirectory of files to convert\n"
							 "\t-q \t--query \tQueried columns to extract\n"
							 "\t-o \t--out_dir \tOutput directory\n"
							 "\t-c \t--canonical \tFlag for creating canonical data\n"
							 "\t-ci \t--canon_in \tLocation of canonical input files\n"
							 "\t-cg \t--canon_gt \tLocation of canonical groundtruth files\n");
			}
			// extract file name to create output file name and directory
			if (str == "-f" || str == "--file") {
				std::string in_f = check_value(i);
				if(in_f != "") {
					const size_t idx = in_f.find_last_of("/\\");

					output.input_file = in_f;
					output.output_dir = in_f.substr(0, idx); // just incase none provided

					in_f = in_f.substr(idx + 1);
					const size_t ext_idx = in_f.find_last_of(".");
					output.stripped_filename = in_f.substr(0, ext_idx);
				}
			}
			// specify output directory
			if (str == "-o" || str == "--out_dir") {
				std::string out = check_value(i);
				if(out != "") {
					output.output_dir = out;
				}
			}
			// specify input directory
			if (str == "-d" || str == "--dir") {
				std::string in = check_value(i);
				if(in != "") {
					output.input_dir = in;
					output.output_dir = in; // just incase none provided
				}
			}
			// specify queried columns
			if (str == "-q" || str == "--query") {
				std::string in = check_value(i);
				if(in != "") {
					output.queries = extract_queries(in.c_str());
				}
			}
			// activate canonical flag
			if (str == "-c" || str == "--canonical") {
				output.create_canonical = true;
			}
			// record location of canonical input
			if (str == "-ci" || str == "--canon_in") {
				std::string in = check_value(i);
				if (in != "") output.canon_in = in;
			}
			// record location of canonical ground truth
			if (str == "-cg" || str == "--canon_gt") {
				std::string in = check_value(i);
				if (in != "") output.canon_gt = in;
			}
		}
	}

	return output;
}