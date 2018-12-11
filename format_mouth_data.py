import argparse
import math
import os
import csv
import sys

oral_comm_lx = 0
oral_comm_ly = 1
oral_comm_rx = 2
oral_comm_ry = 3
dental_sh_tx = 4
dental_sh_ty = 5
dental_sh_bx = 6
dental_sh_by = 7

def format_data( path_to_data ):
	"""
		Read in file contents, change to new output, overwrite file contents w/
		new output format. Ouput format:
		- width of mouth
		- extent of dental show
		- angle of mouth smile
	"""
	output_data = []
	with open( path_to_data ) as csv_file:
		csv_reader = csv.reader( csv_file, delimiter=' ' )
		data_idx = 0
		for row in csv_reader:
			data = [float(x) for x in row]

			output_data.append( [0, 0, 0] )

			output_data[data_idx][0] = abs(
				data[oral_comm_lx] - data[oral_comm_rx] )
			output_data[data_idx][1] = abs(
				data[dental_sh_ty] - data[dental_sh_by] )
			output_data[data_idx][2] = math.atan2(
				data[oral_comm_ly] - data[dental_sh_by],
				data[oral_comm_lx] - data[dental_sh_bx] )

			# print( "Conversion {}, {}, {}".format(
			# 	output_data[data_idx][0],
			# 	output_data[data_idx][1],
			# 	math.degrees( output_data[data_idx][2] ) ) )

			data_idx += 1
	print( "Converted data length: {}".format( len( output_data ) ) )

	with open( path_to_data, 'w' ) as overwrite_file:
		for row in output_data:
			overwrite_file.write( "{:.6f} {:.6f} {:.6f}\n".format(
				row[0], row[1], row[2] ) )

# create parser
s_parser = argparse.ArgumentParser(
	description="Translate oral commisure & dental show data to neural net input format")
s_parser.add_argument(
	"-f",
	default="",
	action="store",
	dest="input_data",
	help="Specify space-delimited input data csv (full path)")

if __name__=="__main__":
	# parse arguments
	args = s_parser.parse_args()
	input_data = args.input_data
	if input_data == "":
		print("Must provide input data to format. Exiting\n")
		sys.exit()

	# make sure file is csv
	root, ext = os.path.splitext( input_data )
	if ext != ".csv":
		print("Must provide csv file (found: {}). Exiting\n".format( ext ) )
		sys.exit()

	print( "\nConverting {}\n".format( input_data ) )

	# format
	format_data( input_data )