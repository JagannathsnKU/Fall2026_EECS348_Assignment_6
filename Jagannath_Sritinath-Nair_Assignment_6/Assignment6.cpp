/**
 * Program Name: EECS 348 Assignment 6: Batch Sudoku Solver with GenAI Integration
 * Brief Description: Object-oriented batch Sudoku solver writing formatted solutions to solutions.txt
 * Inputs: puzzle1.txt through puzzle5.txt
 * Outputs: Console logs and solutions.txt output file
 * Author: Jagannath Sritinath Nair
 * Creation Date: October 15, 2026
 * Revision Date: November 13, 2026
 * Revised by: Jacob Spry (originally), Jagannath Sritinath Nair (now)
 * Revisions: Replaced the per-puzzle solutionN.txt files with one combined solutions.txt.
 *            The file is opened once in main() and every puzzle block is appended in order.
 *            Each block has a "Puzzle: x" header, then either "No solution found." or every
 *            solution in input-file format followed by "Solutions found: n".
 *            Prologue and in-line comments were rewritten for this assignment.
 * Collaborators: Jacob Spry (Provided original Assignment 5 baseline code)
 * Other Sources: Claude 3.5 Sonnet (https://claude.ai)
 **/

#include <array>    // std::array for the fixed-size 9x9 board
#include <fstream>  // std::ifstream / std::ofstream for reading puzzles and writing solutions.txt
#include <iostream> // std::cout for console logging
#include <string>   // std::string for file names and token parsing
#include <vector>   // std::vector for candidates and the list of solutions
#include <utility>  // std::move for cheap string transfer in constructors

// ---------------------------------------------------------------------------
// SudokuGrid: stores one 9x9 board, loads it from a file, checks Sudoku rules,
// and prints itself in the input-file format.
// ---------------------------------------------------------------------------
class SudokuGrid { // Begin class definition for SudokuGrid
public: // Public access specifier for member functions and constants
    static constexpr int SIZE = 9;  // The board has 9 rows and 9 columns
    static constexpr int BLOCK = 3; // Each sub-box is 3 cells wide and 3 cells tall

    /**
     * Source: Jacob Spry (Original Assignment 5 Code)
     * Other Collaborators: None
     **/
    SudokuGrid() { // Constructor body opening brace
        for (auto& row : cells_) { // Range-based loop iterating over each row
            row.fill(0);           // Mark every cell in that row as blank (0)
        }                          // Closing brace for the row iteration loop
    }                              // Closing brace for the default constructor

    /**
     * Source: Claude 3.5 Sonnet & Jacob Spry (Modified by Jagannath Sritinath Nair for robust token extraction)
     * Other Collaborators: None
     **/
    bool loadFromFile(const std::string& filename) { // Function definition for loading puzzle from file
        std::ifstream in(filename); // Open the puzzle file for reading
        if (!in) return false;      // Return false immediately if file cannot be opened

        int count = 0;       // Track how many valid cells have been stored so far
        std::string token;   // Hold one whitespace-separated chunk of the file
        while (in >> token) {          // Loop to read tokens one at a time, skipping whitespace
            for (char ch : token) {    // Loop through each character within the token
                if (count >= SIZE * SIZE) return false; // Return false if cell count exceeds 81
                int value;                              // Declare variable to hold parsed integer value
                if (ch >= '1' && ch <= '9') {           // Check if character is a valid clue digit
                    value = ch - '0';                   // Convert character digit to its numeric integer value
                } else if (ch == '_' || ch == '.' || ch == '0') { // Check if character represents a blank space
                    value = 0;                          // Store blank symbols as integer 0
                } else {                                // Catch-all block for unrecognized characters
                    return false;                       // Return false indicating an invalid file format
                }                                       // Closing brace for character checking conditional block
                cells_[count / SIZE][count % SIZE] = value; // Map linear index to 2D row/column coordinates
                ++count;                                    // Increment the total parsed cell counter
            }                                           // Closing brace for character iteration loop
        }                                               // Closing brace for token input loop
        return count == SIZE * SIZE;                    // Return true only if exactly 81 cells were loaded
    }                                                   // Closing brace for loadFromFile function

    /**
     * Source: Jacob Spry (Original Assignment 5 Code)
     * Other Collaborators: None
     **/
    int get(int r, int c) const { return cells_[r][c]; } // Inline getter method returning cell value

    /**
     * Source: Jacob Spry (Original Assignment 5 Code)
     * Other Collaborators: None
     **/
    void set(int r, int c, int v) { cells_[r][c] = v; } // Inline setter method assigning value to cell

    /**
     * Source: Jacob Spry (Original Assignment 5 Code)
     * Other Collaborators: None
     **/
    bool findEmptyCell(int& row, int& col) const { // Function definition to locate empty cells
        for (int r = 0; r < SIZE; ++r) {         // Outer loop scanning rows top to bottom
            for (int c = 0; c < SIZE; ++c) {     // Inner loop scanning columns left to right
                if (cells_[r][c] == 0) {         // Check if current cell contains 0 (blank)
                    row = r;                     // Assign current row index to output parameter
                    col = c;                     // Assign current column index to output parameter
                    return true;                 // Return true indicating an empty cell was found
                }                                // Closing brace for empty cell conditional check
            }                                    // Closing brace for inner column loop
        }                                        // Closing brace for outer row loop
        return false; // Return false indicating no blank cells remain
    }                                            // Closing brace for findEmptyCell function

    /**
     * Source: Jacob Spry (Original Assignment 5 Code)
     * Other Collaborators: None
     **/
    bool isValidPlacement(int r, int c, int v) const { // Function checking placement legality
        for (int i = 0; i < SIZE; ++i) {                  // Loop iterating through row and column indices
            if (i != c && cells_[r][i] == v) return false; // Check if value appears elsewhere in the same row
            if (i != r && cells_[i][c] == v) return false; // Check if value appears elsewhere in the same column
        }                                                 // Closing brace for row/column check loop
        const int br = (r / BLOCK) * BLOCK; // Calculate top row of the 3x3 box containing (r, c)
        const int bc = (c / BLOCK) * BLOCK; // Calculate left column of the 3x3 box containing (r, c)
        for (int i = br; i < br + BLOCK; ++i) {           // Outer loop iterating through rows of 3x3 box
            for (int j = bc; j < bc + BLOCK; ++j) {       // Inner loop iterating through columns of 3x3 box
                if ((i != r || j != c) && cells_[i][j] == v) return false; // Check for duplicate value inside 3x3 box
            }                                             // Closing brace for inner box loop
        }                                                 // Closing brace for outer box loop
        return true; // Return true if placement violates no Sudoku rules
    }                                                     // Closing brace for isValidPlacement function

    /**
     * Source: Jacob Spry (Original Assignment 5 Code)
     * Other Collaborators: None
     **/
    std::vector<int> candidates(int r, int c) const { // Function returning vector of valid candidates
        std::vector<int> result;                 // Initialize vector to hold surviving valid digits
        for (int v = 1; v <= 9; ++v) {           // Loop iterating through digits 1 through 9
            if (isValidPlacement(r, c, v)) {     // Check if digit v is legal at (r, c)
                result.push_back(v);             // Append valid digit to candidate vector
            }                                    // Closing brace for validity conditional check
        }                                        // Closing brace for digit iteration loop
        return result; // Return populated vector of candidate numbers
    }                                            // Closing brace for candidates function

    /**
     * Source: Jacob Spry (Original Assignment 5 Code)
     * Other Collaborators: None
     **/
    bool givensAreConsistent() const { // Function validating starting clue consistency
        for (int r = 0; r < SIZE; ++r) {         // Loop iterating through all rows
            for (int c = 0; c < SIZE; ++c) {     // Loop iterating through all columns
                if (cells_[r][c] != 0 && !isValidPlacement(r, c, cells_[r][c])) { // Check if non-zero clue conflicts
                    return false;                // Return false if initial clue contradiction found
                }                                // Closing brace for conflict check
            }                                    // Closing brace for column loop
        }                                        // Closing brace for row loop
        return true; // Return true if all initial clues are consistent
    }                                            // Closing brace for givensAreConsistent function

    /**
     * Source: Jacob Spry (Original Assignment 5 Code)
     * Other Collaborators: None
     **/
    void print(std::ostream& os) const { // Function printing formatted grid to output stream
        for (int r = 0; r < SIZE; ++r) {                       // Loop iterating through each row
            if (r > 0 && r % BLOCK == 0) {                     // Check if row index is a multiple of 3
                os << "------+-------+------\n";               // Print horizontal divider line
            }                                                  // Closing brace for horizontal divider check
            for (int c = 0; c < SIZE; ++c) {                   // Loop iterating through each column
                if (c > 0 && c % BLOCK == 0) os << "| ";       // Print vertical pipe divider between blocks
                os << (cells_[r][c] == 0 ? '_' : static_cast<char>('0' + cells_[r][c])); // Print underscore or character digit
                if (c < SIZE - 1) os << ' ';                   // Print space between cells unless at end of row
            }                                                  // Closing brace for column printing loop
            os << '\n';                                        // Print newline character at end of row
        }                                                      // Closing brace for row printing loop
    }                                                          // Closing brace for print function

    /**
     * Source: Claude 3.5 Sonnet & Jagannath Sritinath Nair (Added for plain input-format output)
     * Other Collaborators: None
     **/
    void printPlain(std::ostream& os) const { // Function writing plain grid format to stream
        for (int r = 0; r < SIZE; ++r) {                       // Loop iterating through each row
            for (int c = 0; c < SIZE; ++c) {                   // Loop iterating through each column
                if (c > 0) os << ' ';                          // Print single space separator between cells
                os << (cells_[r][c] == 0 ? '_' : static_cast<char>('0' + cells_[r][c])); // Print underscore for 0 or digit character
            }                                                  // Closing brace for inner column loop
            os << '\n';                                        // Print newline character at end of row
        }                                                      // Closing brace for outer row loop
    }                                                          // Closing brace for printPlain function

private: // Private access specifier for internal grid data representation
    std::array<std::array<int, SIZE>, SIZE> cells_; // 2D fixed-size array storing grid cell values
}; // Closing semicolon and comment for SudokuGrid class definition

// ---------------------------------------------------------------------------
// SudokuSolver: recursive depth-first search with backtracking that collects
// every valid solution of a puzzle.
// ---------------------------------------------------------------------------
class SudokuSolver { // Begin class definition for SudokuSolver
public: // Public access specifier for solver methods
    /**
     * Source: Jacob Spry (Original Assignment 5 Code)
     * Other Collaborators: None
     **/
    explicit SudokuSolver(const SudokuGrid& puzzle) : working_(puzzle) {} // Constructor initializing working grid

    /**
     * Source: Jacob Spry (Original Assignment 5 Code)
     * Other Collaborators: Jacob Spry
     **/
    const std::vector<SudokuGrid>& solveAll() { // Method initiating solver search workflow
        solutions_.clear();                    // Clear any previous solutions from vector
        if (working_.givensAreConsistent()) {  // Check if initial puzzle givens are valid
            search();                          // Execute recursive search algorithm
        }                                      // Closing brace for consistency check conditional
        return solutions_;                     // Return reference to discovered solutions vector
    }                                          // Closing brace for solveAll function

private: // Private access specifier for helper methods and working state
    /**
     * Source: Jacob Spry (Original Assignment 5 Code)
     * Other Collaborators: Jacob Spry
     **/
    void search() { // Private recursive backtracking method
        int r, c; // Declare row and column coordinate variables
        if (!working_.findEmptyCell(r, c)) {  // Base case: check if no empty cells remain
            solutions_.push_back(working_);   // Store completed valid board into solutions vector
            return;                           // Return from current recursive branch
        }                                     // Closing brace for empty cell check conditional
        for (int v : working_.candidates(r, c)) { // Loop iterating through valid candidate digits
            working_.set(r, c, v);                // Place candidate digit into cell
            search();                             // Recursively search for subsequent cell solutions
            working_.set(r, c, 0);                // Backtrack: reset cell back to 0
        }                                         // Closing brace for candidate loop
    }                                             // Closing brace for search function

    SudokuGrid working_;                  // Working board mutated during recursive traversal
    std::vector<SudokuGrid> solutions_;   // Container storing all discovered valid solutions
}; // Closing semicolon and comment for SudokuSolver class definition

// ---------------------------------------------------------------------------
// PuzzleRunner: processes one puzzle file, logs to the console, and appends
// that puzzle's block to the shared solutions.txt stream.
// ---------------------------------------------------------------------------
class PuzzleRunner { // Begin class definition for PuzzleRunner
public: // Public access specifier for runner constructor and methods
    /**
     * Source: Jacob Spry (Original Assignment 5 Code)
     * Other Collaborators: None
     **/
    explicit PuzzleRunner(std::string filename) : filename_(std::move(filename)) {} // Constructor initializing filename member

    /**
     * Source: Claude 3.5 Sonnet & Jagannath Sritinath Nair (Modified for unified solutions.txt formatting)
     * Other Collaborators: None
     **/
    void run(std::ostream& console, std::ostream& fileOut) const { // Method executing pipeline for a single puzzle
        console << "==============================\n";      // Print visual separator line to console stream
        console << "Puzzle file: " << filename_ << "\n\n";   // Print active filename to console stream

        fileOut << "Puzzle: " << filename_ << '\n';          // Write required puzzle header line to solutions.txt

        SudokuGrid puzzle;                                   // Declare SudokuGrid instance for loading file
        if (!puzzle.loadFromFile(filename_)) {               // Attempt to load puzzle; check for file error
            console << "Error: could not read a valid 9x9 puzzle from this file.\n\n"; // Print error to console
            fileOut << "No solution found.\n\n";             // Write required failure message to output file
            return;                                          // Return early from execution pipeline
        }                                                    // Closing brace for load failure conditional

        console << "Puzzle:\n";                              // Print puzzle heading label to console
        puzzle.print(console);                               // Print initial grid layout to console
        console << '\n';                                     // Print trailing newline for spacing

        SudokuSolver solver(puzzle);                         // Instantiate SudokuSolver with loaded puzzle
        const auto& solutions = solver.solveAll();           // Retrieve constant reference to discovered solutions

        if (solutions.empty()) {                             // Check if solutions vector is empty
            console << "No solution found\n\n";              // Print failure message to console
            fileOut << "No solution found.\n\n";             // Write required failure message to solutions.txt
            return;                                          // Return early from execution pipeline
        }                                                    // Closing brace for empty solutions conditional

        console << solutions.size()                          // Output solution count with correct pluralization to console
                << (solutions.size() == 1 ? " solution" : " solutions") << " found:\n\n"; // Print conclusion header
        for (std::size_t i = 0; i < solutions.size(); ++i) { // Loop iterating through each solution for console
            if (solutions.size() > 1) console << "Solution " << (i + 1) << ":\n"; // Print solution index label if multiple exist
            solutions[i].print(console);                     // Print solved grid layout to console
            console << '\n';                                 // Print trailing newline between solutions
        }                                                    // Closing brace for console solution output loop

        for (std::size_t i = 0; i < solutions.size(); ++i) { // Loop iterating through solutions for file output
            solutions[i].printPlain(fileOut);                // Write grid in plain input-format format to solutions.txt
            fileOut << '\n';                                 // Print blank line separator between solution grids
        }                                                    // Closing brace for file output solution loop
        fileOut << "Solutions found: " << solutions.size() << "\n\n"; // Write required summary line and spacer to solutions.txt
    }                                                        // Closing brace for run function

private: // Private member data access specifier
    std::string filename_; // String storing target puzzle filename
}; // Closing semicolon and comment for PuzzleRunner class definition

// ---------------------------------------------------------------------------
// main: opens solutions.txt once, then processes puzzle1.txt..puzzle5.txt in order
// (or any files named on the command line).
// ---------------------------------------------------------------------------
/**
 * Source: Claude 3.5 Sonnet & Jagannath Sritinath Nair (Modified for single solutions.txt batch control)
 * Other Collaborators: None
 **/
int main(int argc, char* argv[]) { // Main function entry point
    std::vector<std::string> files; // Initialize string vector for puzzle filenames
    if (argc > 1) {                          // Check if user supplied custom command-line arguments
        for (int i = 1; i < argc; ++i) {     // Loop iterating through provided arguments
            files.emplace_back(argv[i]);     // Add each argument filename to vector
        }                                    // Closing brace for argument loop
    } else {                                 // Fallback branch for default puzzle files
        for (int i = 1; i <= 5; ++i) {       // Loop iterating from 1 to 5
            files.push_back("puzzle" + std::to_string(i) + ".txt"); // Construct default filename and push to vector
        }                                    // Closing brace for default filename loop
    }                                        // Closing brace for conditional block

    std::ofstream out("solutions.txt");      // Open (and truncate) the single shared output file
    if (!out) {                              // Check if output stream failed to open
        std::cerr << "Error: could not create solutions.txt\n"; // Print error to standard error stream
        return 1;                            // Return non-zero integer indicating failure
    }                                        // Closing brace for output stream check

    for (const auto& f : files) {            // Range-based loop iterating over each puzzle file
        PuzzleRunner(f).run(std::cout, out); // Instantiate PuzzleRunner for file f and execute pipeline
    }                                        // Closing brace for file processing loop

    std::cout << "Results written to solutions.txt\n"; // Print final console confirmation message
    return 0;                                // Return integer 0 indicating successful termination
}                                            // Closing brace for main function
