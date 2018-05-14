#include "NormalParse.h"
#include "StringLib.h"

output_data parse_csv(const Args& args) {
	output_data out_d;
	std::vector<unsigned> idxs;

	//ddIO io_handle;
	ddFileIO<> io_handle;
	bool opened = io_handle.open(args.input_file.c_str(), ddIOflag::READ);
	bool capture_idx = true;

	if (opened) {
		const char* line = io_handle.readNextLine();
		
		while(line) {
			dd_array<cbuff<64>> vals = StrSpace::tokenize1024<64>(line, ",");
			if (capture_idx) {
				// record indices of queried columns, use to extract data
				DD_FOREACH(cbuff<64>, buff, vals) {
					for(auto& query : args.queries) {
						if (buff.ptr->contains(query.c_str())) {
							idxs.push_back(buff.i);
							//printf("%s::%llu\n", buff.ptr->str(), (long long unsigned)buff.i);
						}
					}
				}
				// add header to output and record for mapping
				out_d.push_back(std::vector<std::string>(idxs.size()));
				for (size_t i = 0; i < idxs.size(); ++i) {
					out_d[0][i] = vals[idxs[i]].str();
				}

				capture_idx = false;
			} else {
				// add new row to output
				const size_t curr_spot = out_d.size();
				out_d.push_back(std::vector<std::string>(idxs.size()));
				for (size_t i = 0; i < idxs.size(); ++i) {
					out_d[curr_spot][i] = vals[idxs[i]].str();
				}
			}
			line = io_handle.readNextLine();
		}
		
	}

	return out_d;
}

void write_data(const Args& args, const output_data& data) {
	//ddIO io_handle;
	ddFileIO<> io_handle;

#ifdef WIN32
	//const char* slash = "\\";
#else
	//const char* slash = "/";
#endif

	std::string outfile = args.output_dir + args.stripped_filename + "_out.csv";
	printf("Writing %s\n", outfile.c_str());
	bool opened = io_handle.open(outfile.c_str(), ddIOflag::WRITE);

	if (opened) {
		const char* delim = ",";
		for (auto& row : data) {
			size_t i = 0;
			for (auto& column : row) {
				io_handle.writeLine(column.c_str());
				if (++i < row.size()) {
					io_handle.writeLine(delim);
				}
			}
			delim = " "; // 1st row is comma, other rows are space
			io_handle.writeLine("\n");
		}
	}
}

std::vector<std::string> extract_queries(const char* file) {
	std::vector<std::string> out_q;

	//ddIO io_handle;
	ddFileIO<> io_handle;
	bool opened = io_handle.open(file, ddIOflag::READ);

	if (opened) {
		const char* line = io_handle.readNextLine();

		while(line) {
			printf("  Query: %s\n", line);
			out_q.push_back(std::string(line));
			line = io_handle.readNextLine();
		}
	} else {
		printf("extract_queries::Failed to open: %s\n", file);
	}

	return out_q;
}

void create_formatted_csvs(Args args) {
	// check arguments
	if (args.queries.size() < 1) {
		printf("No queries provided to parse files. Skipping.\n\n");
		return;
	}
	const bool single_file = args.input_dir == "";
	
	if (single_file && args.input_file == "") {
		printf("No input file or directory provided to parse. Skipping\n");
		return;
	}
	
	
	if (args.input_dir != "") {
		printf("Input dir:  %s\n\n", args.input_dir.c_str());
		ddFileIO<1024> io_handle;
		bool opened = io_handle.open(args.input_dir.c_str(), ddIOflag::DIRECTORY);

		if (opened) {
			dd_array<std::string> files = io_handle.get_directory_files();
			DD_FOREACH(std::string, file, files) {
				// get input file and stripped file name
				args.input_file = *file.ptr;
				const size_t idx = args.input_file.find_last_of("/\\");
				args.stripped_filename = args.input_file.substr(idx + 1);
				args.stripped_filename = args.stripped_filename.substr(
					0, args.stripped_filename.size() - 4);

				printf("Input csv:  %s\n", args.input_file.c_str());
				printf("Output csv: %s_out.csv\n", args.stripped_filename.c_str());
				printf("Output dir: %s\n\n", args.output_dir.c_str());

				output_data data = parse_csv(args);
				// write to output directory
				write_data(args, data);
			}
		}
	} else {
		printf("Input csv:  %s\n", args.input_file.c_str());
		printf("Output csv: %s_out.csv\n", args.stripped_filename.c_str());
		printf("Output dir: %s\n\n", args.output_dir.c_str());
		output_data data = parse_csv(args);
		// write to output directory
		write_data(args, data);
	}
}

dd_array<std::string> load_files(const char *directory) {
	// open folder & extract files
	ddFileIO<> f_handle;
	f_handle.open(directory, ddIOflag::DIRECTORY);
	dd_array<std::string> unfiltered = f_handle.get_directory_files();

	// check if file contains _s_out.csv or _v_out.csv
	dd_array<unsigned> valid_files(unfiltered.size());
	unsigned files_found = 0;
	DD_FOREACH(std::string, file, unfiltered) {
		if (file.ptr->find("_s_out.csv") != std::string::npos || 
				file.ptr->find("_v_out.csv") != std::string::npos) {
			// capture index of matching files
			valid_files[files_found] = file.i;
			files_found++;
		}
	}

	// extract file names
	dd_array<std::string> out(files_found);
	for (unsigned i = 0; i < files_found; i++) {
		std::string _s = unfiltered[valid_files[i]];
		out[i] = _s.substr(_s.find_last_of("\\/") + 1);
	}

	return out;
}