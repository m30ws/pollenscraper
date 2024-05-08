#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define NAME_CLASS_NAME "peludKategorija"
#define VALUE_CLASS_NAME "peludStupacOcitanje"
#define NAME_MAX_SIZE 64+1

#define EXPORT_JSON 1
#if defined(EXPORT_JSON) && EXPORT_JSON == 1
	#define JSON_FILE "./data.json"
	#define JSON_DATA_VER "1.0"
#endif

#define DBG 1
#define DEBUG(x) if (DBG) { x; }

#define IS_BLANK(c) ((c)==' ' || (c)=='\n' || (c)=='\r' || (c)=='\t')


/*
	Process each value that gets extracted here
*/
void process_value(char** cat, float* val)
{
#if 0
	// Can modify originals through ptrs (category is dealloc'd in outside loop)
	int len = strlen(*cat);
	*cat = realloc(*cat, len + 9);
	strcat(*cat, "_appended");
	*val = 6.6;
#endif

	char* category = *cat;
	float value = *val;

	DEBUG( printf("%s: %2.1f\n", category, value); )
}

/*
	Find start function
	has two pointers corresponding to current pos in source string
	and current pos in keyword string that was matched up current char
	
	@param[in] ptr Source string
	@param[in] kwd Which keyword to search for
	@return Pointer to the beginning of the keyword in the text or NULL
*/
char* find_start(char* ptr, char* kwd)
{
	// dont worry its quite stupid
	int kwd_len = strlen(kwd);
	char* kwd_ptr = kwd;
	char* ret_ptr = NULL;
	
	while (*ptr) {
		if (*kwd_ptr == '\0') {
		// only time it should be is when kwd is found
			ret_ptr = ptr - kwd_len;
			break;
		}

		if (*kwd_ptr == *ptr) {
			kwd_ptr++;
		} else {
			kwd_ptr = kwd;
		}

		ptr++;
	}

	return ret_ptr;
}


/*
	Reads file in blocks
	Automatically allocates memory and
	optionally returns amount read (may be set to null)

	@param[in] filename Either file name or ""/"stdin" to read from stdin
	@param[out] read_len Optional pointer to where to store file size
	@return Pointer to loaded file or NULL if error
*/
char* read_file_blocks(const char* filename, size_t* read_len)
{
	FILE* fp;
	if (!filename || strlen(filename) == 0 || strcmp("stdin", filename) == 0) { // stdin
		fp = stdin;
	} else if ((fp = fopen(filename, "r")) == NULL) { // file
		DEBUG( printf("cannot open '%s'\n", filename); )
		return NULL;
	}

	DEBUG( printf("reading from: %s\n", (fp==stdin) ? "STDIN" : filename) )

	const int bloksize = 4096;
	size_t filebufsize = bloksize;
	char blockbuf[bloksize];
	char* filebuf = malloc(filebufsize);
	filebuf[0] = '\0';

	size_t rlen = 0;
	while (1) {
		size_t cnt = fread(blockbuf, bloksize, 1, fp);
		if (cnt <= 0)
			break;

		rlen += cnt;

		filebufsize += bloksize;
		char* newbuf = realloc(filebuf, filebufsize);
		if (newbuf) {
			filebuf = newbuf;
		} else {
			DEBUG( printf("error allocating %d bytes\n", filebufsize); )
			if (fp != stdin) fclose(fp);
			return NULL;
		}
		strcat(filebuf, blockbuf);
	}

	if (fp != stdin)
		fclose(fp);

	if (read_len)
		*read_len = rlen;
	return filebuf;
}


int main(int argc, char* argv[])
{
	//== Read file
	char* contents;
	contents = (argc > 1) ? argv[1] : ""; // use 'contents' as filename
	if ((contents = read_file_blocks(contents, NULL)) == NULL)
		return 1;

#if defined(EXPORT_JSON) && EXPORT_JSON == 1
	FILE* json_fp;
	if (!JSON_FILE || strlen(JSON_FILE) == 0 || strcmp("stdout", JSON_FILE) == 0) {
		json_fp = stdout;
	} else {
		json_fp = fopen(JSON_FILE, "w");
	}
	if (json_fp)
		fprintf(json_fp, "{\n\t\"ver\": " JSON_DATA_VER ",\n\t\"values\": {\n");
#endif

	//== Find values
	int n_found = 0;
	char* found;
	char* contents_ptr = contents;
	while ((found = find_start(contents_ptr, NAME_CLASS_NAME)) != NULL) {

	// Parse category (name of the plant/pollen)
		// Category name may be misspelled so just skip all blanks until tag end
		contents_ptr = found + strlen(NAME_CLASS_NAME);
		while (IS_BLANK(*contents_ptr) || *contents_ptr=='"' || *contents_ptr=='>')
			contents_ptr++;

	// Skip from '<' of the next tag until no more "blank" chars
		// Find start of the next tag
		char* next_tag_start = contents_ptr;
		while (*next_tag_start != '<')
			next_tag_start++;

		// Trim blanks from behind
		char* name_end = next_tag_start-1;
		while (IS_BLANK(*name_end))
			name_end--;
		*(name_end + 1) = '\0';

		// Store category name
		char* kat_name = malloc(NAME_MAX_SIZE);
		strcpy(kat_name, contents_ptr);
		contents_ptr = next_tag_start + 1;

	// Find and parse pollen value
		if ((found = find_start(contents_ptr, VALUE_CLASS_NAME)) != NULL) {
			contents_ptr = found + strlen(VALUE_CLASS_NAME);
			while (IS_BLANK(*contents_ptr) || *contents_ptr=='"' || *contents_ptr=='>')
				contents_ptr++;

			// now at: 6.7</span>...
			float val = 0.f;
			sscanf(contents_ptr, " %f ", &val);

	// Process value
			char* kat_ptr = kat_name;
			process_value(&kat_ptr, &val);

#if defined(EXPORT_JSON) && EXPORT_JSON == 1
			if (json_fp)
				fprintf(json_fp, "%s\t\t\"%s\": %2.1f", (n_found>0) ? ",\n" : "", kat_ptr, val);
#endif

			free(kat_name);
			n_found++;
		}

	// Inc to next find range
		while (*contents_ptr != '<')
			contents_ptr++;
	}

#if defined(EXPORT_JSON) && EXPORT_JSON == 1
		if (json_fp)
			fprintf(json_fp, "\n\t}%s\t\"count\": %d", (n_found>0) ? ",\n" : "", n_found);
#endif

	DEBUG(printf("-----\nExtracted: %d\n", n_found))

#if defined(EXPORT_JSON) && EXPORT_JSON == 1
	fprintf(json_fp, "\n}\n");
	fclose(json_fp); //
#endif

// Dealloc
	free(contents);
	return 0;
}
