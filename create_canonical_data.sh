#!/usr/bin/env bash

# parse 1 argument (if script located/called from different directory than
# source) for parent dir of bin/fk_data

PARSER_EXE=./bin/fk_data
SOURCE_DATA=smile_data/
Q_IN=queries_in.txt
Q_GT=queries_groundtruth.txt
if [[ $# == 1 ]]; then
	PARSER_EXE=./$1/bin/fk_data
	SOURCE_DATA=$1/smile_data/
	Q_IN=$1/queries_in.txt
	Q_GT=$1/queries_groundtruth.txt
	echo "Running smile data parser from" $PARSER_EXE "on" $SOURCE_DATA
fi

# create directories for storing data
mkdir -p smile_input smile_ground

# clean out directories
rm -rf smile_input/*
rm -rf smile_ground/*

# split the data to input and ground truth data for keras
$PARSER_EXE -d $SOURCE_DATA -q $Q_IN -o smile_input/
$PARSER_EXE -d $SOURCE_DATA -q $Q_GT -o smile_ground/

# convert data to canonical space
$PARSER_EXE -c -ci smile_input/ -cg smile_ground/

# combine data into csv file for python script
[ ! -e all_canonical_input.csv ] || rm all_canonical_input.csv
cat smile_input/*_canon.csv >> all_canonical_input.csv
[ ! -e all_canonical_ground.csv ] || rm all_canonical_ground.csv
cat smile_ground/*_canon.csv >> all_canonical_ground.csv

# add pre-conversion step to format input data
python3 format_mouth_data.py -f all_canonical_input.csv

# remove out directories
rm -rf smile_input
rm -rf smile_ground