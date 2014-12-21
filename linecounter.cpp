#include <cstdlib>
#include <cstdio>
#include <dirent.h>
#include <iostream>
#include <string.h>
#include <fstream>
#include <algorithm>
#include <functional> 
#include <locale>

/*
 * Thanks to Evan Teran for the string trimming functions.
 * http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
 * I combined his three functions into one for simplicity's sake.
 */
static inline std::string& trim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

/**
 * Count the lines of a C++ source file that aren't part of a block comment,
 * part of a line comment, or blank.
 * @param file_path
 * @return Line count
 */
std::size_t count_lines(const char* file_path){
    // Open an input stream at the file_path
    std::ifstream file_stream (file_path);
    // Holds current line
    std::string line;
    // Currently inside a block comment?
    bool in_block_comment (false);
    // Current line count
    std::size_t count = 0;
    // Ending char sequence for a block comment
    const std::string end = "*/";
    // Starting char sequence for a block comment
    const std::string start = "/*";
    // Holds the first occurrence of end
    std::size_t end_pos;
    // Holds the first occurrence of start
    std::size_t start_pos;
    // Start looping through each line
    while(std::getline(file_stream, line)){
        // Trim the whitespace from both sides of the line
        trim(line);
        // Get start_pos and end_pos
        end_pos = line.find(end);
        start_pos = line.find(start);
        if(!in_block_comment){
            // If we aren't in a block comment...
            if(start_pos != std::string::npos){
                // If block comment starts on this line...
                if(end_pos == std::string::npos || end_pos < start_pos){
                    // And if the block comment isn't ended on the same line,
                    // flip the flag
                    in_block_comment = true;
                }
                // Move on
                continue;
            }else{
                if(line != "" && line.find("//") != 0){
                    // If the line isn't blank and it doesn't start with a line
                    // comment starting character sequence, increment conter
                    count += 1;
                }
            }
        }else{
            // If we are in a block comment...
            if(end_pos != std::string::npos && (start_pos == std::string::npos || start_pos < end_pos)){
                // If it's being ended on this line and not restarted, flip flag
                in_block_comment = false;
                // Move on
                continue;
            }
        }
    }
    return count;
}

int main(int argc, char** argv) {
    // Holds the target dir as a string
    std::string current_dir_str;
    // Holds the target dir as a DIR pointer
    DIR* current_dir;
    if(argc != 2){
        // If there isn't a single argument, use the current
        current_dir_str = ".";
    }else{
        // If a argument was provided, use it as the target
        current_dir_str = argv[1];
    }
    // Open the directory getting the DIR pointer
    current_dir = opendir(current_dir_str.c_str());
    if(current_dir == NULL){
        // If the directory couldn't be opened show an error
        std::cerr << "Cannot open directory: " << argv[1] << std::endl;
        return 300;
    }
    
    // Will hold the directory child entry as we loop
    struct dirent* directory_entry;
    // Holds the file name for each directory_entry
    std::string file_name;
    // Holds the total line count
    std::size_t line_count = 0;
    
    // Start looping through the directory_entrys
    while((directory_entry = readdir(current_dir)) != NULL){
        // Get the name of the entry
        file_name = directory_entry->d_name;
        if(
                file_name.length() >= 3
                &&
                (
                file_name.find(".cpp") == (file_name.length() - 4)
                ||
                file_name.find(".h") == (file_name.length() - 2)
                )
        ){
            // If it's a file with a .h or .cpp extension, then process the line
            // count by first getting the full path
            std::string file_path = current_dir_str + "/" + file_name;
            // Get the line count for the file
            std::size_t file_line_count (count_lines(file_path.c_str()));
            // Add to the current total line count
            line_count += file_line_count;
            // Output the file name and the its individual line count
            std::cout << file_path.c_str() << "\t=" << file_line_count
                    << std::endl;
        }
    }
    // Flush and exit
    std::cout << line_count << std::endl;
    return 0;
}