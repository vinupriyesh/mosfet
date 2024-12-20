#!/bin/bash

# Get the current directory
DIR_TO_COMPRESS=$(pwd)

# Get the current directory name
DIR_NAME=$(basename "$DIR_TO_COMPRESS")

# Get the current date and time
TIMESTAMP=$(date +"%y-%d-%m-%H-%M")

# Set the archive name
ARCHIVE_NAME="${DIR_NAME}_${TIMESTAMP}"

# Create a temporary file to list files to be included in the archive
INCLUDE_LIST=$(mktemp)

# Find all files that are NOT ignored by .gitignore
git ls-files --cached --others --exclude-standard > "$INCLUDE_LIST"

# Add additional files or folders to the include list
# Replace "/path/to/your/file_or_directory" with the paths you want to include
# echo "/path/to/your/file1" >> "$INCLUDE_LIST"
# echo "/path/to/your/folder2" >> "$INCLUDE_LIST"

# Read the list of files into an array
readarray -t FILES < "$INCLUDE_LIST"

# Compress the files into a .tar.gz archive
tar -czvf "$ARCHIVE_NAME".tar.gz "${FILES[@]}"

# Clean up the temporary file
rm "$INCLUDE_LIST"

echo "Compression complete: $ARCHIVE_NAME.tar.gz"
