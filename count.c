/*
 * COMP 321 Project 2: Word Count
 *
 * This program counts the characters, words, and lines in one or more files,
 * depending on the command-line arguments.
 * 
 * Zikang Chen zc45
 */

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "csapp.h"

struct counts {
	int   char_count;
	int   word_count;
	int   line_count;
};

struct node {
	struct counts *count;
	char *filename;
	struct node *next;
};

static void	app_error_fmt(const char *fmt, ...);
static int	do_count(char *input_files[], const int nfiles,
		    const bool char_flag, const bool word_flag,
		    const bool line_flag, const bool test_flag);
static void	print_counts(FILE *fp, struct counts *cnts, const char *name,
		    const bool char_flag, const bool word_flag,
		    const bool line_flag);
static void print_helper(struct node *head, bool char_flag, 
			bool word_flag, bool line_flag, int tc, int tw, int tl);

/*
 * Requires:
 *   The first argument, "fmt", is a printf-style format string, and all
 *   subsequent arguments must match the types of arguments indicated in the
 *   format string.
 *
 * Effects:
 *   Prints a formatted error message to stderr using the supplied
 *   format string and arguments.  The message is prepended with
 *   the string "ERROR: " and a newline is added at the end.
 */
static void
app_error_fmt(const char *fmt, ...)
{
	va_list ap;

	fprintf(stderr, "ERROR: ");
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
}

/*
 * Requires:
 *   The "fp" argument must be a valid FILE pointer.
 *   The "cnts" argument must not be NULL and must point to an allocated
 *   structure of type "struct counts".
 *   The "name" argument must not be NULL and must point to an allocated
 *   string.
 *
 * Effects:
 *   Prints the character, word, and line counts for a particular file
 *   as directed by the flags.
 */
static void
print_counts(FILE *fp, struct counts *cnts, const char *name,
    const bool char_flag, const bool word_flag, const bool line_flag)
{

	if (line_flag)
		fprintf(fp, "%8d", cnts->line_count);
	if (word_flag)
		fprintf(fp, "%8d", cnts->word_count);
	if (char_flag)
		fprintf(fp, "%8d", cnts->char_count);
	fprintf(fp, " %s\n", name);
}

/*
 * Requires:
 *   The "input_files" argument is an array of strings with "nfiles" valid
 *   strings.
 *
 * Effects: 
 *   Prints to stdout the counts for each file in input_files in
 *   alphabetical order followed by the total counts.  The
 *   "char/word/line_flag" arguments indicate which counts should be
 *   printed on a file-by-file basis.  The total count will include all
 *   three counts regardless of the flags.  An error message is printed
 *   to stderr for each file that cannot be opened.
 *
 *   Returns 0 if every file in input_files was successfully opened and
 *   processed.  If, however, an error occurred opening or processing any
 *   of the files, a small positive integer is returned.
 *
 *   The behavior is undefined when "test_flag" is true.
 */
static int
do_count(char *input_files[], const int nfiles, const bool char_flag,
    const bool word_flag, const bool line_flag, const bool test_flag)
{
	// initiate a head node
	struct node *head = NULL;

	// processing each file
	FILE *input_file;
	int i;
	int error = 0, c = 0;
	int tc = 0, tw = 0, tl = 0;

	if (test_flag){
		return (0);
	}

	for (i = 0; i < nfiles; i++){
		int cc = 0, wc = 0, lc = 0;
		// file name
		char *input = input_files[i];
		input_file = fopen(input, "r");
        if (input_file == NULL) {
            app_error_fmt("cannot open \'%s\'", input);
            return (1);  /* non-zero for error */
        }
		bool indicator = false;

		// using temp to record the last character
		int temp = -1;
        while ((c = fgetc(input_file)) != EOF){
			// character count
			if (char_flag){
				if (c >= 0 && c <= 255){
					cc++;
					tc++;
				}
			}
	
			// word count
			if (word_flag){
				if (isspace(c) || c == '\n'){
					if (indicator){
						wc++;
						tw++;
					}
					indicator = false; 
				} else {
					indicator = true;
				}
			}

			// line count
			if (line_flag){			
				if (c == '\n'){
					lc++;
					tl++;
				}
			}

			temp = c;
		}

        if (!feof(input_file)) {
            /*
             * If feof() returns FALSE, then the above while loop
             * didn't reach the end of file.  The EOF returned by
             * fgetc() instead meant that an error occurred while
             * reading from the input file. 
             */
            app_error_fmt("cannot read \'%s\'", input);
            error = 1;  /* non-zero for error */
        } else {
			// add the word count if necessary
			if (temp != '\n' && !isspace(temp) && temp >= 0){
				wc++;
				tw++;
			}
		}

		// create a node for each file
		struct counts *new_count = malloc(sizeof(struct counts));
		struct node *new_node = malloc(sizeof(struct node));
		new_count->char_count = cc;
		new_count->word_count = wc;
		new_count->line_count = lc;
		new_node->next = NULL;
		new_node->filename = input;
		new_node->count = new_count;

		// assign head
		if (head == NULL){
			head = new_node;
		} else if (strcmp(head->filename, new_node->filename) >= 0){
			new_node->next = head;
			head = new_node;
		} else {		
			// iterate over the node list
			struct node *cur = head;
			while (cur->next != NULL){
				if (strcmp(new_node->filename, cur->next->filename) <= 0){
					new_node->next = cur->next;
					cur->next = new_node;
					break;
				} else {
					cur = cur->next;
				}
			}

			// append the node at the end of the list
			if (cur->next == NULL){
				cur->next = new_node;
				new_node->next = NULL;
			}
		}
		fclose(input_file);
	}
	// print
	print_helper(head, char_flag, word_flag, line_flag, tc, tw, tl);
	return error;
}

/*
 * Requires:
 *   The head node is the head for the linked list.
 *   The"char/word/line_flag" arguments indicate which counts should be
 *   printed on a file-by-file basis.
 *
 * Effects:
 *   Prints the character, word, and line counts for a particular file
 *   as directed by the flags.
 */
static void print_helper(struct node *head, bool char_flag, 
	bool word_flag, bool line_flag, int tc, int tw, int tl){
		struct node *cur = head;
		struct node *prev = NULL;

		// iterate over the node list
		while (cur != NULL){
			print_counts(stdout, cur->count, cur->filename, char_flag,
			word_flag, line_flag);

			prev = cur;
			cur = cur->next;

			// free the memory
			Free(prev->count);
			Free(prev);
		}
		struct counts *total_count = malloc(sizeof(struct counts));
		total_count->char_count = tc;
		total_count->word_count = tw;
		total_count->line_count = tl;
		print_counts(stdout, total_count, "total", true, true, true);
		Free(total_count);
}

/*
 * Requires:
 *   Nothing.
 *
 * Effects:
 *   Parses command line arguments and invokes do_count with the appropriate
 *   arguments based on the command line.
 */
int 
main(int argc, char **argv)
{
	int c;			// Option character
	extern int optind;	// Option index

	// Abort flag: Was there an error on the command line? 
	bool abort_flag = false;

	// Option flags: Were these options on the command line?
	bool char_flag = false;
	bool line_flag = false;
	bool test_flag = false;
	bool word_flag = false;

	// Process the command line arguments.
	while ((c = getopt(argc, argv, "cltw")) != -1) {
		switch (c) {
		case 'c':
			// Count characters.
			if (char_flag) {
				// A flag can only appear once.
				abort_flag = true;
			} else {
				char_flag = true;
			}
			break;
		case 'l':
			// Count lines.
			if (line_flag) {
				// A flag can only appear once.
				abort_flag = true;
			} else {
				line_flag = true;
			}
			break;
		case 't':
			// Enable test flag.
			if (test_flag) {
				// A flag can only appear once.
				abort_flag = true;
			} else {
				test_flag = true;
			}
			break;
		case 'w':
			// Count words.
			if (word_flag) {
				// A flag can only appear once.
				abort_flag = true;
			} else {
				word_flag = true;
			}
			break;
		case '?':
			// An error character was returned by getopt().
			abort_flag = true;
			break;
		}
	}
	if (abort_flag || optind == argc) {
		/*
		 * In this case, use fprintf instead of app_error_fmt because
		 * error messages describing how to run a program typically
		 * begin with "usage: " not "ERROR: ".
		 */
		fprintf(stderr,
		    "usage: %s [-c] [-l] [-t] [-w] <input filenames>\n",
		    argv[0]);
		return (1);	// Indicate an error.
	}

	// Process the input files, and return the result.
	return (do_count(&argv[optind], argc - optind, char_flag, word_flag,
	    line_flag, test_flag));
}
